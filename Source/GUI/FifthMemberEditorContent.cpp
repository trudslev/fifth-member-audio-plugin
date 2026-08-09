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
    /** The printed scale each fixed knob carries. Kept beside the knob table rather than inside
    KnobVariant because a variant is a SIZE - two knobs can share a body and print different rings,
    as TIME and DAMPING do at 66px. */
const FifthMemberTheme::Layout::KnobScale* scaleForParameter (const juce::String& id)
{
    using namespace FifthMemberTheme::Layout;
    static const KnobScale time    = scaleOf (timeMarks);
    static const KnobScale damping = scaleOf (dampingMarks);
    static const KnobScale sparse  = scaleOf (percentSparseMarks);
    static const KnobScale full    = scaleOf (percentFullMarks);
    static const KnobScale fb      = scaleOf (feedbackMarks);
    static const KnobScale trim    = scaleOf (trimMarks);

    if (id == ParamIDs::timeMs)     return &time;
    if (id == ParamIDs::damping)    return &damping;
    if (id == ParamIDs::feedback)   return &fb;
    if (id == ParamIDs::outputTrim) return &trim;
    if (id == ParamIDs::mix)        return &full;
    // SATURATION and CROSS-FEED sit on smaller bodies, so 25 and 75 stay unlabelled to keep the
    // numerals off each other (section 4.4).
    if (id == ParamIDs::saturation || id == ParamIDs::crossFeed) return &sparse;
    return nullptr;
}

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
        knob->setScale (scaleForParameter (specs[i].paramID));

        if (auto* param = processorRef.apvts.getParameter (specs[i].paramID))
            knob->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

        addAndMakeVisible (*knob);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processorRef.apvts, specs[i].paramID, *knob);

        attachReadout (*knob, specs[i].paramID);

        knobs[i] = std::move (knob);
        displayProportion[i] = knobs[i]->getDrawnProportion();
    }

    // The three character dials. Created ONCE here; only their attachments change with the mode.
    for (size_t i = 0; i < dials.size(); ++i)
    {
        auto dial = std::make_unique<FifthMemberKnob> (Layout::KnobSize::dial76);
        dial->setCentrePosition ({ Layout::dialCentreX[i], Layout::dialCentreY });

        // Dial 1 alone carries a second ring. Its inner arc legends a percentage (Wow in Tape,
        // Repeat Degrade in Digital) and its outer one a frequency (Mod Rate in BBD); a percentage
        // and a frequency cannot share numerals - 1 Hz sits at 0 degrees where the percent ring
        // prints 50 - so both are permanently printed at different radii and each lights or dims
        // with the mode, exactly as the stacked labels do. Section 4.5.
        // Both of dial 1's rings carry their own range: exactly one of them matches the bound
        // parameter at a time, so neither can rely on the Slider's mapping. Dials 2 and 3 have a
        // single ring that always matches, so they keep the plain form.
        static const Layout::KnobScale dialPercent = Layout::scaleOf (Layout::percentFullMarks,
                                                                       0.0f, 100.0f, 1.0f);
        static const Layout::KnobScale dialHz      = Layout::scaleOf (Layout::modRateMarks,
                                                                       0.1f, 5.0f, 0.4090339496f);
        static const Layout::KnobScale dialPlain   = Layout::scaleOf (Layout::percentFullMarks);
        dial->setScale (i == 0 ? &dialPercent : &dialPlain);
        if (i == 0)
            dial->setOuterScaleAndResize (&dialHz, { Layout::dialCentreX[i], Layout::dialCentreY });
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
void FifthMemberEditorContent::attachReadout (juce::Slider& knob, const juce::String& paramID)
{
    auto* param = processorRef.apvts.getParameter (paramID);
    if (param == nullptr)
        return;

    // Section 6.3: the LCD shows this control's value while it is being moved. GUARDED ON THE
    // KNOB'S OWN DRAG STATE - a SliderAttachment also fires when a Program is applied and on every
    // host automation step, and without the guard the readout latches onto whichever parameter was
    // written last and flickers for the length of a song.
    auto* raw = &knob;
    knob.onValueChange = [this, raw, param]
    {
        if (raw->isMouseButtonDown())
            programHeader.showParameter (*param);
    };
    knob.onDragEnd = [this] { programHeader.releaseParameter(); };
}

void FifthMemberEditorContent::bindDials (int character)
{
    boundCharacter = character;

    // Section 4.5: dial 1's outer Hz arc is the live ring in BBD and the dim one elsewhere; the
    // inner percent arc is the reverse. Exactly one of the two reads as current at any time.
    dials[0]->setOuterRingLit (character == 1);

    for (size_t i = 0; i < dials.size(); ++i)
    {
        const auto* id = dialParameterFor ((int) i, character);

        // Release the old attachment BEFORE making the new one: two live attachments on one
        // Slider would both write to their parameters on the next change.
        dialAttachments[i].reset();

        if (id == nullptr)
        {
            // Unassigned in this mode. No attachment, so the knob turns a value that goes nowhere -
            // and crucially writes nothing to the APVTS. Re-binding on the next mode change pulls it
            // back to that parameter's stored value, which the slew then animates.
            dials[i]->setDoubleClickReturnValue (false, 0.0);
            continue;
        }

        if (auto* param = processorRef.apvts.getParameter (id))
            dials[i]->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

        dialAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processorRef.apvts, id, *dials[i]);

        attachReadout (*dials[i], id);
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
