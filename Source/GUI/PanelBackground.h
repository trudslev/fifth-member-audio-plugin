#pragma once

#include "FifthMemberTheme.h"

/**
    Everything on the panel that never changes, rasterised once into a juce::Image at construction
    and blitted thereafter.

    That is the "baked static layers" half of the rendering decision. The prototype is 100 % CSS
    with no assets at all, so the knobs and lamps are code-drawn per TapeRot and Reflect-84 - but
    the fascia alone is a base gradient plus a brushed texture plus two ambient radials plus four
    asymmetric corner wears plus two scuffs, over 1240 x 848. Recompositing that on every repaint
    would be pure waste for pixels that cannot change. design/README.md asks for the tick rings to
    be baked for the same reason.

    The asymmetry is the point and is transcribed exactly rather than approximated with a tiling
    noise texture: four corner wears at different sizes and opacities, two scuffs at different
    angles, four screw slots at four distinct angles. Identical screws read as CG.

    It bakes at `bakeScale`, not 1:1. The panel's two defining textures - the fascia's 3px brush and
    the rack ears' 2px one - are at the resolution limit, so a 1:1 bake blitted to a Retina display
    resolves them to a flat wash and loses the metal entirely. Baking at 2x costs ~17 MB and holds
    up wherever the editor is scaled to.
*/
class PanelBackground final : public juce::Component
{
public:
    PanelBackground();

    void paint (juce::Graphics& g) override;

private:
    static constexpr int bakeScale = 2;

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
