#pragma once

#include "FifthMemberTheme.h"

/**
    Everything on the panel that never changes, rasterised once into a juce::Image at construction
    and blitted thereafter.

    That is the "baked static layers" half of the rendering decision. The prototype is 100 % CSS
    with no assets at all, so the knobs and lamps are code-drawn per TapeRot and Reflect-84 - but
    the fascia alone is a base gradient plus a brushed texture plus two ambient radials plus four
    asymmetric corner wears plus two scuffs, over 1240 x 855. Recompositing that on every repaint
    would be pure waste for pixels that cannot change. design/README.md asks for the tick rings to
    be baked for the same reason.

    The asymmetry is the point and is transcribed exactly rather than approximated with a tiling
    noise texture: four corner wears at different sizes and opacities, two scuffs at different
    angles, four screw slots at four distinct angles. Identical screws read as CG.
*/
class PanelBackground final : public juce::Component
{
public:
    PanelBackground();

    void paint (juce::Graphics& g) override;

private:
    void buildImage();

    void paintFascia (juce::Graphics& g);
    void paintRackEars (juce::Graphics& g);
    void paintNameplate (juce::Graphics& g);
    void paintHeaderChrome (juce::Graphics& g);
    void paintPanelChrome (juce::Graphics& g);
    void paintStaticLabels (juce::Graphics& g);
    void paintFoot (juce::Graphics& g);

    juce::Image baked;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelBackground)
};
