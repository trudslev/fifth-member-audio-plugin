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

    // Resizable, but WITHOUT JUCE's corner grip - that is what the second argument adds, and it
    // draws its diagonal hatch straight over the plate's bottom-right rack ear. It was the only
    // region of the composite diff that did not correspond to a designed runtime element: the
    // panel's own artwork, overprinted by a piece of framework furniture.
    //
    // The window stays resizable from its edges, and the fixed aspect ratio below still governs.
    // The trade is that a host offering no frame of its own leaves the editor at its opening size;
    // that is the better failure for a photoreal panel than a hatch across the ear.
    setResizable (true, false);

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
