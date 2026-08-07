#pragma once

#include "PluginProcessor.h"
#include "GUI/FifthMemberEditorContent.h"

/**
    A thin shell: one FifthMemberEditorContent drawn at the fixed reference canvas size, with a
    single uniform scale transform and the aspect ratio locked.

    design/README.md calls for a "single non-resizable window". Every sibling casting is
    resizable-with-locked-aspect and users expect it, so the house convention wins here - the panel
    still scales as one unit and its proportions never change.
*/
class FifthMemberAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit FifthMemberAudioProcessorEditor (FifthMemberAudioProcessor&);
    ~FifthMemberAudioProcessorEditor() override = default;

    void resized() override;

private:
    FifthMemberEditorContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FifthMemberAudioProcessorEditor)
};
