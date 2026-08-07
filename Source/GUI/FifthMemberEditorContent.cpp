#include "FifthMemberEditorContent.h"

#include "../PluginProcessor.h"

#include <cmath>

using namespace FifthMemberTheme;

namespace
{
    /** Which parameter each dial position drives, per mode.

        Where a position has no label in the current mode it keeps editing its own parameter - the
        topmost (Tape) one - rather than falling back to whatever the current mode's first
        parameter happens to be. That would make Dial 3 in Digital a second Repeat Degrade control,
        duplicating Dial 1, and duplicate controls on a hardware panel read as a fault. Keeping its
        own parameter also matches the persistence model: turning it edits the value that will be
        in effect when that mode comes back. */
    const char* dialParameterFor (int dial, int character)
    {
        switch (dial)
        {
            case 0:
                return character == 1 ? ParamIDs::modRate
                     : character == 2 ? ParamIDs::degrade
                                      : ParamIDs::wow;
            case 1:
                return character == 1 ? ParamIDs::modDepth : ParamIDs::flutter;
            default:
                return ParamIDs::genLoss;
        }
    }
}

const std::array<FifthMemberEditorContent::KnobSpec, 7>& FifthMemberEditorContent::knobSpecs()
{
    static const std::array<KnobSpec, 7> specs { {
        { ParamIDs::timeMs,     Layout::timeKnobCentre,       Layout::KnobSize::standard66 },
        { ParamIDs::feedback,   Layout::feedbackKnobCentre,   Layout::KnobSize::primary84 },
        { ParamIDs::crossFeed,  Layout::crossFeedKnobCentre,  Layout::KnobSize::small62 },
        { ParamIDs::mix,        Layout::mixKnobCentre,        Layout::KnobSize::primary82 },
        { ParamIDs::outputTrim, Layout::trimKnobCentre,       Layout::KnobSize::small62 },
        { ParamIDs::damping,    Layout::dampingKnobCentre,    Layout::KnobSize::standard66 },
        { ParamIDs::saturation, Layout::saturationKnobCentre, Layout::KnobSize::standard66 },
    } };

    return specs;
}

//==============================================================================
FifthMemberEditorContent::FifthMemberEditorContent (FifthMemberAudioProcessor& processor)
    : processorRef (processor),
      scope (processor),
      controls (processor),
      programHeader (processor)
{
    setSize ((int) canvasWidth, (int) canvasHeight);

    panelBackground.setBounds (getLocalBounds());
    addAndMakeVisible (panelBackground);

    const auto& specs = knobSpecs();

    for (size_t i = 0; i < specs.size(); ++i)
    {
        auto knob = std::make_unique<FifthMemberKnob> (specs[i].size);
        knob->setCentrePosition (specs[i].centre);

        if (auto* param = processorRef.apvts.getParameter (specs[i].paramID))
            knob->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

        addAndMakeVisible (*knob);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processorRef.apvts, specs[i].paramID, *knob);

        knobs[i] = std::move (knob);
        displayProportion[i] = knobs[i]->getDrawnProportion();
    }

    // The three character dials. Created ONCE here; only their attachments change with the mode.
    for (size_t i = 0; i < dials.size(); ++i)
    {
        auto dial = std::make_unique<FifthMemberKnob> (Layout::KnobSize::dial76);
        dial->setCentrePosition ({ Layout::dialCentreX[i], Layout::dialCentreY });
        addAndMakeVisible (*dial);
        dials[i] = std::move (dial);
    }

    bindDials ((int) processorRef.apvts.getRawParameterValue (ParamIDs::character)->load());

    for (size_t i = 0; i < dials.size(); ++i)
        dialDisplayProportion[i] = dials[i]->getDrawnProportion();

    addAndMakeVisible (scope);
    addAndMakeVisible (controls);
    addAndMakeVisible (programHeader);

    controls.onCharacterChanged = [this] { beginRearmSweep(); };

    lastFrameMs = juce::Time::getMillisecondCounter();
    startTimerHz (Layout::animationHz);
}

FifthMemberEditorContent::~FifthMemberEditorContent()
{
    stopTimer();
}

//==============================================================================
void FifthMemberEditorContent::bindDials (int character)
{
    boundCharacter = character;

    for (size_t i = 0; i < dials.size(); ++i)
    {
        const auto* id = dialParameterFor ((int) i, character);

        // Release the old attachment BEFORE making the new one: two live attachments on one
        // Slider would both write to their parameters on the next change.
        dialAttachments[i].reset();

        if (auto* param = processorRef.apvts.getParameter (id))
            dials[i]->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

        dialAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processorRef.apvts, id, *dials[i]);
    }
}

void FifthMemberEditorContent::beginRearmSweep()
{
    // Hold every dial at minimum for a frame or two, then release. Combined with the slew below,
    // all three then visibly sweep up to their new values.
    rearmFramesRemaining = juce::jmax (1, juce::roundToInt (
        Layout::rearmHoldMs / (1000.0f / (float) Layout::animationHz)));
}

//==============================================================================
void FifthMemberEditorContent::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dt = juce::jlimit (1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    const int character = (int) processorRef.apvts.getRawParameterValue (ParamIDs::character)->load();

    if (character != boundCharacter)
    {
        bindDials (character);

        // A mode change from automation or a Program load never touched PanelControls, so arm the
        // sweep here too - the panel should re-set itself however the mode arrived.
        if (rearmFramesRemaining <= 0)
            beginRearmSweep();
    }

    const auto slewFor = [dt] (float settleMs)
    {
        // CHORUS-60's law: 1 - 0.002^(dt/settle). Time-based, so travel takes the same wall time
        // whatever the frame rate and a dropped frame lengthens the step rather than shortening
        // the motion.
        return 1.0f - std::pow (Layout::slewRemainderAtSettle, dt / settleMs);
    };

    const auto advance = [] (FifthMemberKnob& knob, float& display, float slew, bool forceMinimum)
    {
        const float target = (float) knob.valueToProportionOfLength (knob.getValue());

        if (forceMinimum)
            display = 0.0f;
        else if (knob.isMouseButtonDown())
            display = target;                    // track the pointer 1:1; slewing under the user's
                                                 // own hand reads as lag, not as motion
        else
            display += slew * (target - display);

        knob.setDisplayProportion (display);
    };

    const bool holding = rearmFramesRemaining > 0;

    if (holding)
        --rearmFramesRemaining;

    for (size_t i = 0; i < knobs.size(); ++i)
        advance (*knobs[i], displayProportion[i], slewFor (Layout::knob66.slewMs), false);

    for (size_t i = 0; i < dials.size(); ++i)
        advance (*dials[i], dialDisplayProportion[i], slewFor (Layout::knob76.slewMs), holding);
}
