#include "PanelBackground.h"

PanelBackground::PanelBackground()
{
    // Pure background: never intercepts a click, so every control layered over it still receives
    // its own.
    setInterceptsMouseClicks (false, false);
}

void PanelBackground::paint (juce::Graphics& g)
{
    using namespace FifthMemberTheme;

    static const juce::Image plate = juce::ImageFileFormat::loadFrom (
        BinaryData::fifthmemberplate2x_png, (size_t) BinaryData::fifthmemberplate2x_pngSize);

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (plate, juce::Rectangle<float> (0.0f, 0.0f, Layout::canvasWidth, Layout::canvasHeight),
                  juce::RectanglePlacement::stretchToFit);
}
