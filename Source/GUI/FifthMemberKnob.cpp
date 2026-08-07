#include "FifthMemberKnob.h"

using namespace FifthMemberTheme;

FifthMemberKnob::FifthMemberKnob (Layout::KnobSize size)
    : juce::Slider (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      knobSize (size)
{
    setRotaryParameters (juce::degreesToRadians (Layout::knobArcStartDegrees),
                         juce::degreesToRadians (Layout::knobArcEndDegrees), true);

    // The design's figure: 190 px of vertical travel spans the full parameter range.
    setMouseDragSensitivity (Layout::knobDragPixels);
    setVelocityBasedMode (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
}

void FifthMemberKnob::setCentrePosition (juce::Point<float> centre)
{
    // The hit area reaches the tick ring's outer edge plus a small click margin, so the whole
    // visible control is grabbable and the ring sits inside this component's own clip region.
    const auto& v = variant();
    const float half = v.diameter * 0.5f - v.tickInset + 3.0f;

    setBounds (juce::Rectangle<float> (centre.x - half, centre.y - half, half * 2.0f, half * 2.0f)
                   .getSmallestIntegerContainer());
}

void FifthMemberKnob::setDisplayProportion (float proportion) noexcept
{
    const float clamped = juce::jlimit (0.0f, 1.0f, proportion);

    if (std::abs (clamped - displayOverride) < 1.0e-5f)
        return;

    displayOverride = clamped;
    repaint();
}

float FifthMemberKnob::getDrawnProportion()
{
    if (displayOverride >= 0.0f)
        return displayOverride;

    return (float) valueToProportionOfLength (getValue());
}

//==============================================================================
void FifthMemberKnob::paintBody (juce::Graphics& g, juce::Point<float> centre,
                                 const Layout::KnobVariant& v, float value01)
{
    const float r = v.diameter * 0.5f;
    const juce::Rectangle<float> body { centre.x - r, centre.y - r, v.diameter, v.diameter };

    // --- tick ring, outside the body, drawn first ----------------------------
    {
        const float outer = r - v.tickInset;
        const float inner = outer - outer * 0.16f;
        const float span = Layout::knobArcEndDegrees - Layout::knobArcStartDegrees;
        const int steps = juce::jmax (1, juce::roundToInt (span / v.tickStepDegrees));

        g.setColour (v.tickColour);

        for (int i = 0; i <= steps; ++i)
        {
            const float angle = Layout::knobArcStartDegrees + (float) i * (span / (float) steps);
            g.drawLine ({ Geometry::pointOnCircle (centre, inner, angle),
                          Geometry::pointOnCircle (centre, outer, angle) }, v.tickWidth);
        }
    }

    // --- cast shadow ---------------------------------------------------------
    {
        juce::Path path;
        path.addEllipse (body);
        juce::DropShadow shadow { juce::Colours::black.withAlpha (0.5f),
                                  juce::roundToInt (v.diameter * 0.18f),
                                  { 0, juce::roundToInt (v.diameter * 0.09f) } };
        shadow.drawForPath (g, path);
    }

    // --- body: radial-gradient(circle at 38% 28%, ...) -----------------------
    g.setGradientFill (Paint::radial (body, 0.38f, 0.28f, v.faceTop, v.faceMid, 0.55f, v.faceBottom));
    g.fillEllipse (body);

    // inset 0 -Npx rgba(0,0,0,.65-.70): the underside falls away
    {
        juce::ColourGradient inner { juce::Colours::transparentBlack, centre.x, centre.y - r,
                                     juce::Colours::black.withAlpha (0.55f), centre.x, centre.y + r, false };
        inner.addColour (0.55, juce::Colours::transparentBlack);
        g.setGradientFill (inner);
        g.fillEllipse (body);
    }

    // inset 0 1px 1px rgba(255,255,255,.16-.18): the lit upper lip
    g.setColour (juce::Colours::white.withAlpha (0.17f));
    g.drawEllipse (body.reduced (0.5f).translated (0.0f, 0.5f), 1.0f);

    g.setColour (juce::Colour (0xFF090908));
    g.drawEllipse (body, 1.0f);

    // --- pointer -------------------------------------------------------------
    {
        const float angle = Geometry::knobAngleForValue (value01);
        const float outer = r - v.pointerTopOffset;
        const auto from = Geometry::pointOnCircle (centre, outer, angle);
        const auto to   = Geometry::pointOnCircle (centre, outer - v.pointerLength, angle);

        juce::Path pointer;
        pointer.addLineSegment ({ from, to }, v.pointerWidth);

        g.setGradientFill ({ juce::Colour (0xFFF6F1E6), from.x, from.y,
                             juce::Colour (0xFFB8B1A2), to.x, to.y, false });
        g.fillPath (pointer);
    }
}

void FifthMemberKnob::paint (juce::Graphics& g)
{
    const auto centre = getLocalBounds().toFloat().getCentre();
    paintBody (g, centre, variant(), getDrawnProportion());
}
