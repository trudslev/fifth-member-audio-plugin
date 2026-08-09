#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FifthMemberTheme.h"
#include "FifthMemberMenuLookAndFeel.h"

class FifthMemberAudioProcessor;

/**
    The PROGRAM cluster: the LCD with its FACT/USER tag and caret, the SAVE and DELETE buttons, and
    the two IN/OUT meter windows.

    Gatecrasher's contract, reused: three states (idle+factory, idle+user, naming), naming happens
    inline in the LCD rather than in a dialog, Cancel never touches the APVTS, and Save always
    creates a new Program.

    DELETE is the one control on the whole panel allowed to look inapplicable - design/README.md
    calls it out as the single exception to the no-dimming rule, on the grounds that it is a
    destructive action on a nonexistent target rather than a parameter. Every knob still obeys
    Correction 2 absolutely.
*/
class ProgramHeader final : public juce::Component,
                            private juce::Timer
{
public:
    explicit ProgramHeader (FifthMemberAudioProcessor& processor);
    ~ProgramHeader() override;

    void paint (juce::Graphics& g) override;

    /** Section 6.3. While a control is being moved the name cell shows "NAME: value unit" -
        "FEEDBACK: 62 %", "TIME: 375 ms" - reverting to the Program name 900 ms after release.

        **The CALLER guards on the control's own drag state.** A SliderAttachment also fires when a
        Program is applied and on every host automation step; without that guard the display latches
        onto whichever parameter was written last and flickers for the length of a song, which is
        exactly what section 6.3 forbids. Naming mode wins over both - the glass belongs to the name
        field until it commits or cancels. */
    void showParameter (const juce::RangedAudioParameter& param);
    void releaseParameter();
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress& key) override;
    void focusLost (FocusChangeType) override;

private:
    enum class Region { none, display, save, deleteOrCancel };

    void timerCallback() override;
    bool refreshFromProcessor();

    Region regionAt (juce::Point<float> p) const;
    bool isEnabled (Region region) const;

    void showProgramMenu();
    void enterNamingMode();
    void commitNaming();
    void cancelNaming();

    static juce::Rectangle<float> displayArea();
    static juce::Rectangle<float> saveArea();
    static juce::Rectangle<float> deleteArea();

    void paintButton (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& text,
                      bool enabled, bool hovered, bool isDelete);
    void paintMeters (juce::Graphics& g);

    /** What the name cell currently reads: the live parameter readout if one is in flight,
        otherwise "NN NAME". */
    juce::String currentLcdString() const;

    /** Owned here rather than made per-menu: it has to outlive showMenuAsync's callback, and a
        local would leave the menu drawing through a dangling LookAndFeel. */
    FifthMemberMenuLookAndFeel menuLookAndFeel;

    juce::String liveReadout;
    juce::uint32 readoutRevertAtMs = 0;

    FifthMemberAudioProcessor& processorRef;

    int displayedIndex = -1;
    juce::String displayedName;
    bool displayedIsFactory = true;
    bool displayedIsModified = false;

    bool namingMode = false;
    juce::String typedName;
    bool caretVisible = false;

    Region hovered = Region::none;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramHeader)
};
