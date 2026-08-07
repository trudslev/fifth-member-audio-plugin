#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FifthMemberTheme.h"

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
