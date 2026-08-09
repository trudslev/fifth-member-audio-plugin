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
    // Sized to the WRAPPER, not to the body or the tick tips, because the numerals live outside
    // both and a component clips its own paint. Section 4.2 gives the wrapper as 2R + 68 for a
    // single ring; dial 1 carries the outer Hz arc whose numerals reach much further, so it takes
    // the 232 box that section 4.5 sizes against the widest label's OUTER extent.
    //
    // Getting this wrong is silent: the ring still draws, the numerals just vanish at the edge of
    // the component, which reads as a designer having omitted them.
    const auto& v = variant();
    const float wrapper = outerScale != nullptr ? Layout::dialWrapper1 : v.diameter + 68.0f;
    const float half = wrapper * 0.5f;

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

    // inset 0 -6px 12px rgba(0,0,0,.65) - a BAND along the bottom lip, not a wash over the whole
    // lower half. Spread across half the face it flattened the dome into a dark disc and swallowed
    // the top-left light the radial gradient exists to provide.
    {
        const float band = v.diameter * 0.26f;
        juce::ColourGradient inner { juce::Colours::transparentBlack, centre.x, centre.y + r - band,
                                     juce::Colours::black.withAlpha (0.62f), centre.x, centre.y + r, false };
        g.setGradientFill (inner);
        g.fillEllipse (body);
    }

    // No separate specular pass. `Paint::radial` already sizes to the farthest corner the way CSS
    // does, so the body gradient's own off-centre first stop IS the highlight; adding a bloom on
    // top of it double-counted the light and hazed the upper left.

    // inset 0 1px 1px rgba(255,255,255,.16-.18): the lit upper lip
    g.setColour (juce::Colours::white.withAlpha (0.18f));
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
    const float radius = variant().diameter * 0.5f;

    // --- printed scale, outside the body, drawn first -------------------------
    //
    // A tick at every printed numeral and nowhere else (section 4.2a). The angle of each mark comes
    // from the Slider's own NormalisableRange - the same call the pointer uses - so a skewed
    // parameter's marks come out unevenly spaced, which is correct, and no table of angles can
    // drift away from the taper it legends.
    if (scale != nullptr && ! scale->bakedInPlate)
        drawScale (g, centre, radius, *scale,
                    Layout::tickMajorInner, Layout::tickMajorOuter, Layout::numeralClearance,
                    outerScale == nullptr || ! outerLit);

    if (outerScale != nullptr && ! outerScale->bakedInPlate)
        drawScale (g, centre, radius, *outerScale,
                    Layout::outerTickInner, Layout::outerTickOuter, Layout::outerNumeralClearance,
                    outerLit);

    paintBody (g, centre, variant(), getDrawnProportion());
}

void FifthMemberKnob::setOuterScaleAndResize (const Layout::KnobScale* s, juce::Point<float> centre)
{
    outerScale = s;
    setCentrePosition (centre);
}

void FifthMemberKnob::setOuterRingLit (bool shouldBeLit) noexcept
{
    if (outerLit == shouldBeLit)
        return;
    outerLit = shouldBeLit;
    repaint();
}

void FifthMemberKnob::drawScale (juce::Graphics& g, juce::Point<float> centre, float radius,
                                  const Layout::KnobScale& s,
                                  float tickInner, float tickOuter, float numeralClear, bool lit)
{
    using namespace FifthMemberTheme;

    const auto tickCol  = lit ? Colour::scaleTick      : Colour::scaleTickDim;
    const auto minorCol = lit ? Colour::scaleTickMinor : Colour::scaleTickMinorDim;
    const auto inkCol   = lit ? Colour::scaleNumeral   : Colour::scaleNumeralDim;

    const auto font = Font::label (Layout::numeralSize);
    const float tracking = Font::trackingPx (Layout::numeralTracking, Layout::numeralSize);

    for (int i = 0; i < s.count; ++i)
    {
        const auto& m = s.marks[i];

        // The knob's own mapping, so this is exact for skewed parameters rather than assumed linear.
        const float p = (float) valueToProportionOfLength ((double) m.value);
        const float angle = Layout::knobArcStartDegrees
                          + p * (Layout::knobArcEndDegrees - Layout::knobArcStartDegrees);

        const float innerR = m.major ? tickInner : tickInner + (Layout::tickMinorInner - Layout::tickMajorInner);
        g.setColour (m.major ? tickCol : minorCol);
        g.drawLine ({ Geometry::pointOnCircle (centre, radius + innerR, angle),
                       Geometry::pointOnCircle (centre, radius + tickOuter, angle) },
                     m.major ? Layout::tickMajorWidth : Layout::tickMinorWidth);

        if (m.printed == nullptr)
            continue;

        // Section 4.2a: numerals are anchored by the box edge FACING THE DIAL, not by their centre,
        // so a one-character and a three-character label clear the tick tip by the same 4px. A fixed
        // radius instead leaves wide labels visibly tighter to the arc than narrow ones.
        const juce::String text (m.printed);
        const float halfWidth = 0.5f * Text::trackedWidth (text, font, tracking);
        const float rad = juce::degreesToRadians (angle);
        const float r = radius + numeralClear
                      + halfWidth * std::abs (std::sin (rad))
                      + Layout::numeralHalfCap * std::abs (std::cos (rad));

        const auto at = Geometry::pointOnCircle (centre, r, angle);
        Text::drawTracked (g, text, font, tracking,
                            { at.x - 40.0f, at.y - 8.0f, 80.0f, 16.0f },
                            juce::Justification::centred, inkCol);
    }
}
