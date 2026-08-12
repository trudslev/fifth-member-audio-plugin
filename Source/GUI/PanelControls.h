#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FifthMemberTheme.h"

#include <array>
#include <vector>

class FifthMemberAudioProcessor;

/**
    Every stateful element on the panel that is not a knob: the SYNC switch, the five NOTE DIVISION
    buttons, the three STEREO MODE buttons, the three DELAY CHARACTER mode buttons, the three
    conditional LEDs, and the character dials' stacked label rows with their own small lamps.

    Held in one component rather than a class per control because they all need the same thing -
    a poll of the APVTS and a repaint - and splitting them would mean eight timers where one does.
    It covers the whole canvas so it can paint in absolute coordinates, with hitTest narrowed to
    the union of its interactive rects (the pattern CHORUS-60's EngineButtonComponent establishes),
    so it never swallows a click meant for a knob underneath.

    **The fixed three-dial mechanic.** Three dial positions, always physically present, never
    appearing, disappearing or moving. Each carries every label it could ever have, permanently
    printed and stacked, each with its own 6 px LED. Switching Delay Character does exactly two
    things: it relights which label's LED is on, and it rotates the dial to that mode's stored
    value. Per Correction 2 the knob body itself never changes appearance - no dimming, no
    disabling, ever. And per the resolved gap, a dial with no label in the current mode keeps
    editing its own parameter, so nothing on this panel is a dead control.

    **Conditional LEDs on exactly three controls**, and nowhere else: NOTE DIVISION (Sync on) and
    TIME (Sync off), mutually exclusive; and CROSS-FEED (Ping-Pong only). Feedback, Mix, Output
    Trim, Damping and Saturation are always live and deliberately carry no lamp.
*/
class PanelControls final : public juce::Component,
                            private juce::Timer
{
public:
    explicit PanelControls (FifthMemberAudioProcessor& processor);
    ~PanelControls() override;

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    /** Fired when the Delay Character changes, so the editor can run the re-arm sweep. */
    std::function<void()> onCharacterChanged;

    /** **Every control that changes a parameter reports it to the LCD, switches included.**
        BRAND.md's rule, and the reason it is a rule rather than a judgement: deciding which
        controls are "self-explanatory" is harder to apply consistently than deciding none are -
        and a switch is often the LEAST obvious thing on a panel, because turning a knob shows you
        its own printed scale while flipping a switch shows you nothing.

        All four of this component's controls raise it. The editor wires it to the header. */
    std::function<void (const juce::String&)> onParameterTouched;

    /** One row of a dial's permanently-printed label stack. */
    struct DialLabel
    {
        const char* text;       // nullptr = this mode has no label on this dial
    };

    /** Row-major [dial][mode]. Dial 1 has a label in all three modes; dial 2 in Tape and BBD;
        dial 3 in Tape alone. */
    static const std::array<std::array<DialLabel, 3>, 3>& dialLabels();

private:
    enum class Region { none, syncSwitch, division, stereo, character };

    struct Hit
    {
        juce::Rectangle<float> area;
        Region region;
        int index;
    };

    void timerCallback() override;
    void rebuildHitAreas();
    const Hit* hitAt (juce::Point<float> position) const;

    int intParam (const char* id) const;
    bool boolParam (const char* id) const;
    void setChoice (const char* id, int index);

    void paintSyncSwitch (juce::Graphics& g, bool sync);
    void paintDivisionRow (juce::Graphics& g, bool sync, int division);
    void paintStereoRow (juce::Graphics& g, int stereoMode);
    void paintCharacterColumn (juce::Graphics& g, int character);
    void paintDialLabels (juce::Graphics& g, int character);
    void paintConditionalLeds (juce::Graphics& g, bool sync, int stereoMode);

    FifthMemberAudioProcessor& processorRef;

    std::vector<Hit> hits;
    const Hit* hovered = nullptr;

    // Mirrors, refreshed by the timer; repaint only when one actually moves.
    bool displayedSync = true;
    int displayedDivision = 0;
    int displayedStereo = 0;
    int displayedCharacter = 0;

    /** The thumb slides rather than jumping - 180 ms, the design's figure. */
    float thumbPosition = 1.0f;

    juce::uint32 lastFrameMs = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelControls)
};
