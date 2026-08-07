#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FifthMemberKnob.h"
#include "PanelBackground.h"
#include "PanelControls.h"
#include "ProgramHeader.h"
#include "RepeatTimelineScope.h"

#include <array>
#include <memory>

class FifthMemberAudioProcessor;

/**
    The fixed reference canvas, 1240 x 855.

    Every child draws in untransformed canvas coordinates and never learns the window size -
    PluginEditor applies one uniform scale transform to this whole component, so there is
    deliberately no resized() below it.

    This class also owns the knob animation, which is the mechanic CHORUS-60 established and this
    panel leans on hardest. SliderAttachment keeps setting parameters instantly; the 60 Hz timer
    here slews each knob's *drawn* proportion toward its parameter's, so state changes rotate the
    knobs rather than snapping them.

    On a Delay Character change it does something more theatrical, which design/README.md asks for
    by name: every dial's drawn angle is forced to minimum for one frame and then released, so all
    three visibly sweep up to their new values - "the panel physically re-setting itself, the way
    real recall works".
*/
class FifthMemberEditorContent final : public juce::Component,
                                       private juce::Timer
{
public:
    explicit FifthMemberEditorContent (FifthMemberAudioProcessor& processor);
    ~FifthMemberEditorContent() override;

    static constexpr float canvasWidth = FifthMemberTheme::Layout::canvasWidth;
    static constexpr float canvasHeight = FifthMemberTheme::Layout::canvasHeight;

private:
    struct KnobSpec
    {
        const char* paramID;
        juce::Point<float> centre;
        FifthMemberTheme::Layout::KnobSize size;
    };

    static const std::array<KnobSpec, 7>& knobSpecs();

    void timerCallback() override;
    void beginRearmSweep();

    FifthMemberAudioProcessor& processorRef;

    PanelBackground panelBackground;

    std::array<std::unique_ptr<FifthMemberKnob>, 7> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 7> attachments;
    std::array<float, 7> displayProportion {};

    /** The three character dials are re-pointed at a different parameter set when the mode
        changes, so their attachments are rebuilt - but the COMPONENTS are created once and
        re-attached, never recreated. Recreating them would reset the very rotation the sweep
        exists to animate away from. */
    std::array<std::unique_ptr<FifthMemberKnob>, 3> dials;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 3> dialAttachments;
    std::array<float, 3> dialDisplayProportion {};

    RepeatTimelineScope scope;
    PanelControls controls;
    ProgramHeader programHeader;

    int boundCharacter = -1;
    int rearmFramesRemaining = 0;
    juce::uint32 lastFrameMs = 0;

    void bindDials (int character);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FifthMemberEditorContent)
};
