#include "PluginEditor.h"

namespace
{
    constexpr int referenceWidth  = (int) FifthMemberEditorContent::canvasWidth;
    constexpr int referenceHeight = (int) FifthMemberEditorContent::canvasHeight;
}

FifthMemberAudioProcessorEditor::FifthMemberAudioProcessorEditor (FifthMemberAudioProcessor& p)
    : AudioProcessorEditor (&p), content (p)
{
    addAndMakeVisible (content);

    setResizable (true, true);

    if (auto* constrainer = getConstrainer())
    {
        constrainer->setFixedAspectRatio ((double) referenceWidth / (double) referenceHeight);
        constrainer->setSizeLimits (referenceWidth / 2, referenceHeight / 2,
                                    referenceWidth * 2, referenceHeight * 2);
    }

    // The reference canvas is taller than most screens allow at 1:1, so open at 70 %.
    setSize (juce::roundToInt (referenceWidth * 0.7), juce::roundToInt (referenceHeight * 0.7));
}

void FifthMemberAudioProcessorEditor::resized()
{
    const float scale = (float) getWidth() / (float) referenceWidth;

    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, referenceWidth, referenceHeight);
}
