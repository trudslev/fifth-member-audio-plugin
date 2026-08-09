#include "PanelBackground.h"

using namespace FifthMemberTheme;

namespace
{
    /** A notched legend: the panel's border is punched through by a fascia-coloured patch so the
        title appears to sit in the rule rather than on top of it. */
    void drawNotchedLegend (juce::Graphics& g, const juce::String& text, float panelX, float panelY)
    {
        const auto font = Font::label (Layout::legendTextSize);
        const float tracking = Font::trackingPx (Layout::legendTracking, Layout::legendTextSize);
        const float width = Text::trackedWidth (text, font, tracking);

        const juce::Rectangle<float> patch { panelX + 12.0f, panelY - 7.0f, width + 14.0f, 13.0f };

        g.setColour (Colour::fasciaMid);
        g.fillRect (patch);

        Text::drawTracked (g, text, font, tracking, patch, juce::Justification::centred,
                           Colour::panelText);
    }

    void drawPanelBox (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& legend)
    {
        // `linear-gradient(180deg, rgba(255,255,255,.022), rgba(0,0,0,.18))`, drawn as two passes.
        //
        // One gradient white->black cannot express this: CSS interpolates translucent stops in
        // PREMULTIPLIED space, where fading white out and fading black in are independent, while a
        // single JUCE gradient carries a mid grey at partial alpha and lightens the middle of the
        // box by six levels - which is what made every panel read as a raised plate instead of a
        // barely-there recess.
        g.setGradientFill (Paint::vertical (r, juce::Colours::white.withAlpha (0.022f),
                                            juce::Colours::transparentWhite));
        g.fillRoundedRectangle (r, 3.0f);

        g.setGradientFill (Paint::vertical (r, juce::Colours::transparentBlack,
                                            juce::Colours::black.withAlpha (0.18f)));
        g.fillRoundedRectangle (r, 3.0f);

        g.setColour (Colour::panelBorder);
        g.drawRoundedRectangle (r, 3.0f, 1.0f);

        drawNotchedLegend (g, legend, r.getX(), r.getY());
    }

    void drawKnobCaption (juce::Graphics& g, const juce::String& text, float centreX, float y,
                          juce::Colour colour = Colour::label)
    {
        Text::drawTracked (g, text, Font::label (10.0f), Font::trackingPx (0.20f, 10.0f),
                           { centreX - 120.0f, y, 240.0f, 12.5f }, juce::Justification::centred, colour);
    }

    /** Rotated -90 degrees, for the rack-ear stencils. */
    void drawRotatedStencil (juce::Graphics& g, const juce::String& text, juce::Point<float> centre,
                             float size, float trackingEm, juce::Colour colour)
    {
        juce::Graphics::ScopedSaveState save { g };
        g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi,
                                                         centre.x, centre.y));

        Text::drawTracked (g, text, Font::label (size), Font::trackingPx (trackingEm, size),
                           { centre.x - 140.0f, centre.y - size, 280.0f, size * 2.0f },
                           juce::Justification::centred, colour);
    }
}

//==============================================================================
PanelBackground::PanelBackground()
{
    setInterceptsMouseClicks (false, false);
    buildImage();
}

void PanelBackground::paint (juce::Graphics& g)
{
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (baked, juce::Rectangle<float> (Layout::canvasWidth, Layout::canvasHeight));
}

void PanelBackground::buildImage()
{
    baked = juce::Image (juce::Image::ARGB, (int) Layout::canvasWidth * bakeScale,
                         (int) Layout::canvasHeight * bakeScale, true);
    juce::Graphics g { baked };

    // Everything below paints in design coordinates; the bake resolution is this one transform.
    g.addTransform (juce::AffineTransform::scale ((float) bakeScale));

    // The unit is `border-radius:5px; overflow:hidden`, and the rack ears are square rectangles
    // that would otherwise fill the corners back in. Clipping once here rounds the chassis rather
    // than each layer having to know about it.
    {
        juce::Path chassis;
        chassis.addRoundedRectangle (juce::Rectangle<float> (0.0f, 0.0f, Layout::canvasWidth,
                                                             Layout::canvasHeight),
                                     Layout::unitRadius);
        g.reduceClipRegion (chassis);
    }

    paintFascia (g);
    paintRackEars (g);
    paintNameplate (g);
    paintHeaderChrome (g);
    paintPanelChrome (g);
    paintStaticLabels (g);
    paintFoot (g);
}

//==============================================================================
void PanelBackground::paintFascia (juce::Graphics& g)
{
    const juce::Rectangle<float> unit { 0.0f, 0.0f, Layout::canvasWidth, Layout::canvasHeight };

    g.setColour (Colour::unitBg);
    g.fillRoundedRectangle (unit, Layout::unitRadius);

    const juce::Rectangle<float> fascia { Layout::earWidth, 0.0f,
                                          Layout::canvasWidth - Layout::earWidth * 2.0f,
                                          Layout::canvasHeight };

    auto base = Paint::vertical (fascia, Colour::fasciaTop, Colour::fasciaBottom);
    base.addColour (0.42, Colour::fasciaMid);
    g.setGradientFill (base);
    g.fillRect (fascia);

    juce::Graphics::ScopedSaveState save { g };
    g.reduceClipRegion (fascia.getSmallestIntegerContainer());

    // Brushed texture: repeating 1px verticals, light then dark.
    for (float x = fascia.getX(); x < fascia.getRight(); x += 3.0f)
    {
        g.setColour (juce::Colours::white.withAlpha (0.022f));
        g.fillRect (x, 0.0f, 1.0f, Layout::canvasHeight);
        g.setColour (juce::Colours::black.withAlpha (0.05f));
        g.fillRect (x + 1.0f, 0.0f, 2.0f, Layout::canvasHeight);
    }

    // Ambient light: one cool radial from the top left, one warm from the bottom right.
    {
        const float w = fascia.getWidth();
        juce::ColourGradient a { juce::Colours::white.withAlpha (0.07f),
                                 fascia.getX() + w * 0.12f, -Layout::canvasHeight * 0.10f,
                                 juce::Colours::transparentWhite,
                                 fascia.getX() + w * 0.12f + w * 0.55f, Layout::canvasHeight * 0.5f, true };
        g.setGradientFill (a);
        g.fillRect (fascia);

        juce::ColourGradient b { juce::Colour (0xFF786E5F).withAlpha (0.10f),
                                 fascia.getX() + w * 0.88f, Layout::canvasHeight * 1.10f,
                                 juce::Colours::transparentBlack,
                                 fascia.getX() + w * 0.88f - w * 0.45f, Layout::canvasHeight * 0.4f, true };
        g.setGradientFill (b);
        g.fillRect (fascia);
    }

    // Corner wear, rubbed through to bare metal. Four different sizes and peaks so it is
    // asymmetric - a generic tiled noise texture would lose the whole point.
    //
    // Each patch is a box overhanging its corner by 2px with the gradient anchored INSIDE it, and
    // the box clips the result. Both halves of that matter: unclipped, a 120x70 rub spread over
    // the full 848px column and hazed the whole fascia several levels lighter than the design.
    // The mid stop matters too - CSS drops to alpha .10 by 45%, so the patch is a tight bright
    // core with a long faint tail, not the even ramp a proportional mid stop produces.
    struct Wear { float x, y, w, h, cx, cy, peak, mid, midAt, endAt; };
    const std::array<Wear, 4> wear { {
        { fascia.getX() - 2.0f,        -2.0f,   120.0f, 70.0f, 0.0f, 0.0f, 0.55f, 0.10f, 0.45f, 0.72f },
        { fascia.getRight() - 138.0f,  -2.0f,   140.0f, 80.0f, 1.0f, 0.0f, 0.48f, 0.08f, 0.48f, 0.74f },
        { fascia.getRight() - 148.0f,  Layout::canvasHeight - 88.0f, 150.0f, 90.0f,
          1.0f, 1.0f, 0.42f, 0.07f, 0.46f, 0.72f },
        { fascia.getX() - 2.0f,        Layout::canvasHeight - 62.0f, 100.0f, 64.0f,
          0.0f, 1.0f, 0.35f, 0.0f, 0.0f, 0.70f } } };

    for (const auto& w : wear)
    {
        const juce::Rectangle<float> box { w.x, w.y, w.w, w.h };

        juce::Graphics::ScopedSaveState state { g };
        g.reduceClipRegion (box.getSmallestIntegerContainer());

        // JUCE's radial gradients are circular, so the ellipse the design asks for (90% x 90% of
        // a 120x70 box, and friends) comes from squashing about the anchor corner.
        const juce::Point<float> centre { box.getX() + box.getWidth() * w.cx,
                                          box.getY() + box.getHeight() * w.cy };
        const float radius = box.getWidth() * 0.9f;
        g.addTransform (juce::AffineTransform::scale (1.0f, (box.getHeight() * 0.9f) / radius,
                                                      centre.x, centre.y));

        juce::ColourGradient rub { Colour::wearMetal.withAlpha (w.peak), centre.x, centre.y,
                                   Colour::wearMetal.withAlpha (0.0f), centre.x + radius, centre.y, true };
        if (w.midAt > 0.0f)
            rub.addColour (w.midAt, Colour::wearMetal.withAlpha (w.mid));
        rub.addColour (w.endAt, Colour::wearMetal.withAlpha (0.0f));
        g.setGradientFill (rub);
        g.fillRect (box.expanded (0.0f, box.getHeight() * 3.0f));
    }

    // Two scuffs, at different lengths, positions and angles.
    const auto scuff = [&g] (float x, float y, float w, float h, float angle, juce::Colour colour)
    {
        juce::Graphics::ScopedSaveState state { g };
        g.addTransform (juce::AffineTransform::rotation (juce::degreesToRadians (angle), x, y));

        juce::ColourGradient streak { colour.withAlpha (0.0f), x, y, colour.withAlpha (0.0f), x + w, y, false };
        streak.addColour (0.5, colour);
        g.setGradientFill (streak);
        g.fillRect (x, y, w, h);
    };

    scuff (Layout::earWidth + 300.0f, 8.0f, 210.0f, 5.0f, -0.6f, juce::Colour (0xFFCDC7BA).withAlpha (0.16f));
    scuff (Layout::canvasWidth - Layout::earWidth - 180.0f - 260.0f, Layout::canvasHeight - 34.0f,
           260.0f, 3.0f, 0.5f, juce::Colour (0xFFBEB8AC).withAlpha (0.13f));
}

void PanelBackground::paintRackEars (juce::Graphics& g)
{
    /** One end-lamp: `radial-gradient(120% 70% at fx% fy%, rgba(255,255,255,a), transparent 70%)`
        confined to a band at the top or bottom of the ear. These are what make the metal read as
        brushed aluminium catching stage light at both ends with a dull waist between - without
        them the ear is a flat grey strip, which is what it was. */
    const auto lamp = [&g] (juce::Rectangle<float> band, float fx, float fy, float rx, float ry,
                            float alpha, juce::Colour tint)
    {
        juce::Graphics::ScopedSaveState state { g };
        g.reduceClipRegion (band.getSmallestIntegerContainer());

        const juce::Point<float> centre { band.getX() + band.getWidth() * fx,
                                          band.getY() + band.getHeight() * fy };
        // An ellipse rx% x ry% of the band. JUCE radial gradients are circular, so take the
        // horizontal radius and squash vertically about the centre.
        const float radius = band.getWidth() * rx;
        const float squash = (band.getHeight() * ry) / radius;
        g.addTransform (juce::AffineTransform::scale (1.0f, squash, centre.x, centre.y));

        juce::ColourGradient glow { tint.withAlpha (alpha), centre.x, centre.y,
                                    tint.withAlpha (0.0f), centre.x + radius, centre.y, true };
        glow.addColour (0.70, tint.withAlpha (0.0f));   // the CSS stop: gone by 70% of the radius
        g.setGradientFill (glow);
        g.fillRect (band.expanded (0.0f, band.getHeight() * 4.0f));
    };

    struct Ear
    {
        float x;
        std::array<juce::Colour, 6> stops;
        std::array<double, 6> positions;
        float topBandH, topFx, topAlpha;
        float botBandH, botFx, botAlpha;
        bool  hasWaistShadow;
    };

    const std::array<Ear, 2> ears { {
        { 0.0f,
          { juce::Colour (0xFF8E8A82), juce::Colour (0xFFC2BEB4), juce::Colour (0xFF9D998F),
            juce::Colour (0xFF75726B), juce::Colour (0xFFA9A49A), juce::Colour (0xFF807C75) },
          { 0.0, 0.18, 0.40, 0.62, 0.84, 1.0 },
          120.0f, 0.20f, 0.55f, 150.0f, 0.30f, 0.42f, true },
        { Layout::canvasWidth - Layout::earWidth,
          { juce::Colour (0xFF7D7A73), juce::Colour (0xFFACA79D), juce::Colour (0xFF8C8880),
            juce::Colour (0xFF6F6C66), juce::Colour (0xFFB0ABA0), juce::Colour (0xFF7A7770) },
          { 0.0, 0.20, 0.44, 0.66, 0.88, 1.0 },
          140.0f, 0.80f, 0.50f, 170.0f, 0.70f, 0.48f, false } } };

    for (const auto& ear : ears)
    {
        const juce::Rectangle<float> r { ear.x, 0.0f, Layout::earWidth, Layout::canvasHeight };

        // CSS `linear-gradient(96deg, ...)`: the gradient LINE points 6 degrees past due east, so
        // this ramp runs across the 52px width, not down the 848px height. Drawn corner-to-corner
        // - which is what it was - a 52x848 strip makes it almost perfectly vertical, losing every
        // band of the brushed figure. The line is also longer than the box: CSS sizes it so the
        // corners project onto its ends, so only the middle of the ramp is ever visible.
        const auto dir = Geometry::directionFor (96.0f);
        const float length = std::abs (r.getWidth() * dir.x) + std::abs (r.getHeight() * dir.y);
        const auto mid = r.getCentre();
        const auto from = mid - dir * (length * 0.5f);
        const auto to   = mid + dir * (length * 0.5f);

        juce::ColourGradient grad { ear.stops[0], from, ear.stops[5], to, false };

        for (size_t i = 1; i < 5; ++i)
            grad.addColour (ear.positions[i], ear.stops[i]);

        g.setGradientFill (grad);
        g.fillRect (r);

        {
            juce::Graphics::ScopedSaveState save { g };
            g.reduceClipRegion (r.getSmallestIntegerContainer());

            // Brush marks: `repeating-linear-gradient(92deg, ...)`, 2px period. The gradient line
            // is 2 degrees past due east, so the marks lean 2 degrees off vertical - over 848px
            // that walks them 30px sideways, and the near-miss against the pixel grid is what
            // gives the metal its hatched shimmer. Drawn dead vertical they resolve to a flat
            // pinstripe and the ear stops reading as brushed aluminium.
            g.addTransform (juce::AffineTransform::rotation (
                juce::degreesToRadians (-2.0f), r.getCentreX(), r.getCentreY()));

            const float overhang = Layout::canvasHeight * 0.5f;

            for (float x = r.getX() - overhang; x < r.getRight() + overhang; x += 2.0f)
            {
                g.setColour (juce::Colours::white.withAlpha (0.10f));
                g.fillRect (x, -overhang, 1.0f, Layout::canvasHeight + overhang * 2.0f);
                g.setColour (juce::Colours::black.withAlpha (0.09f));
                g.fillRect (x + 1.0f, -overhang, 1.0f, Layout::canvasHeight + overhang * 2.0f);
            }
        }

        lamp ({ r.getX(), 0.0f, r.getWidth(), ear.topBandH },
              ear.topFx, 0.0f, 1.2f, 0.7f, ear.topAlpha, juce::Colours::white);
        lamp ({ r.getX(), Layout::canvasHeight - ear.botBandH, r.getWidth(), ear.botBandH },
              ear.botFx, 1.0f, 1.2f, 0.7f, ear.botAlpha, juce::Colours::white);

        // Only the left ear carries the dulled waist - the asymmetry is deliberate.
        if (ear.hasWaistShadow)
            lamp (r, 0.70f, 0.46f, 0.60f, 0.40f, 0.55f, juce::Colour (0xFF141210));
    }

    // Screws: four distinct slot angles, because identical screws read as CG.
    for (size_t i = 0; i < Layout::screwCentres.size(); ++i)
    {
        const auto centre = Layout::screwCentres[i];
        const float d = Layout::screwDiameter;
        const juce::Rectangle<float> body { centre.x - d * 0.5f, centre.y - d * 0.5f, d, d };

        {
            juce::Path path;
            path.addEllipse (body);
            juce::DropShadow shadow { juce::Colours::black.withAlpha (0.6f), 4, { 0, 2 } };
            shadow.drawForPath (g, path);
        }

        g.setGradientFill (Paint::radial (body, 0.36f, 0.30f, juce::Colour (0xFF4A4741),
                                          juce::Colour (0xFF2A2823), 0.5f, juce::Colour (0xFF14130F)));
        g.fillEllipse (body);

        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.drawEllipse (body.reduced (1.5f), 3.0f);

        juce::Graphics::ScopedSaveState save { g };
        g.addTransform (juce::AffineTransform::rotation (
            juce::degreesToRadians (Layout::screwSlotAngles[i]), centre.x, centre.y));
        g.setColour (Colour::screwSlot);
        g.fillRect (centre.x - 6.0f, centre.y - 1.5f, 12.0f, 3.0f);
    }

    // Ear stencils and the right ear's cable tape.
    drawRotatedStencil (g, "RACK 4 " + Text::middleDot() + " MON WORLD",
                        { 26.0f, Layout::canvasHeight * 0.5f }, 11.0f, 0.34f,
                        juce::Colour (0xFF1E1C19).withAlpha (0.62f));

    {
        const juce::Rectangle<float> tape { Layout::canvasWidth - 48.0f, 150.0f, 44.0f, 28.0f };

        juce::Graphics::ScopedSaveState save { g };
        g.addTransform (juce::AffineTransform::rotation (juce::degreesToRadians (1.4f),
                                                         tape.getCentreX(), tape.getCentreY()));

        {
            juce::Path path;
            path.addRectangle (tape);
            juce::DropShadow shadow { juce::Colours::black.withAlpha (0.45f), 4, { 0, 2 } };
            shadow.drawForPath (g, path);
        }

        g.setGradientFill (Paint::vertical (tape, Colour::cableTapeLight, Colour::cableTapeDark));
        g.fillRect (tape);

        g.setFont (Font::marker (12.0f));
        g.setColour (Colour::cableTapeInk);
        g.drawText ("DLY 4", tape, juce::Justification::centred, false);
    }

    drawRotatedStencil (g, "HALDEN HALL " + Text::middleDot() + " LOAD-IN 06",
                        { 1215.0f, Layout::canvasHeight - 170.0f }, 11.0f, 0.22f,
                        juce::Colour (0xFF26231F).withAlpha (0.55f));
}

//==============================================================================
void PanelBackground::paintNameplate (juce::Graphics& g)
{
    const juce::Rectangle<float> plate { Layout::nameplateX, Layout::nameplateY,
                                         Layout::nameplateW, Layout::nameplateH };

    juce::Graphics::ScopedSaveState save { g };
    g.addTransform (juce::AffineTransform::rotation (
        juce::degreesToRadians (Layout::nameplateRotationDegrees),
        plate.getCentreX(), plate.getCentreY()));

    // Torn edges: the design's clip-path polygon, as fractions of the plate.
    juce::Path torn;
    const std::array<juce::Point<float>, 6> corners { {
        { 0.01f, 0.06f }, { 0.99f, 0.00f }, { 1.00f, 0.92f },
        { 0.60f, 1.00f }, { 0.12f, 0.96f }, { 0.00f, 0.88f } } };

    for (size_t i = 0; i < corners.size(); ++i)
    {
        const juce::Point<float> p { plate.getX() + plate.getWidth() * corners[i].x,
                                     plate.getY() + plate.getHeight() * corners[i].y };
        if (i == 0) torn.startNewSubPath (p);
        else        torn.lineTo (p);
    }

    torn.closeSubPath();

    {
        juce::DropShadow shadow { juce::Colours::black.withAlpha (0.55f), 7, { 0, 3 } };
        shadow.drawForPath (g, torn);
    }

    juce::Graphics::ScopedSaveState clipped { g };
    g.reduceClipRegion (torn);

    auto tape = Paint::vertical (plate, Colour::gafferLight, Colour::gafferDark);
    tape.addColour (0.60, Colour::gafferMid);
    g.setGradientFill (tape);
    g.fillPath (torn);

    // Tape weave.
    for (float x = plate.getX(); x < plate.getRight(); x += 4.0f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.055f));
        g.fillRect (x, plate.getY(), 1.0f, plate.getHeight());
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRect (x + 1.0f, plate.getY(), 3.0f, plate.getHeight());
    }

    // Grime toward the top right.
    {
        juce::ColourGradient grime { juce::Colour (0xFF5A5042).withAlpha (0.30f),
                                     plate.getX() + plate.getWidth() * 0.8f,
                                     plate.getY() + plate.getHeight() * 0.2f,
                                     juce::Colours::transparentBlack,
                                     plate.getRight(), plate.getBottom(), true };
        g.setGradientFill (grime);
        g.fillPath (torn);
    }

    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.drawLine (plate.getX() + 4.0f, plate.getY() + 1.0f, plate.getRight() - 4.0f, plate.getY() + 1.0f, 1.0f);

    // The single most important brand element: marker on tape, written in a hurry. Deliberately
    // freestyle - it must not read as set type.
    const auto font = Font::marker (Layout::nameplateTextSize);
    const juce::Rectangle<float> textArea { plate.getX() + 20.0f, plate.getY() + 9.0f,
                                            plate.getWidth() - 40.0f, 32.34f };

    g.setFont (font);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawText ("FIFTH MEMBER", textArea.translated (0.0f, 1.0f), juce::Justification::centredLeft, false);

    g.setColour (Colour::markerInk);
    g.drawText ("FIFTH MEMBER", textArea, juce::Justification::centredLeft, false);
}

//==============================================================================
void PanelBackground::paintHeaderChrome (juce::Graphics& g)
{
    const auto tagFont = Font::label (Layout::taglineSize);
    const auto tagFontMedium = Font::labelMedium (Layout::taglineSize);
    const float tagTracking = Font::trackingPx (0.30f, Layout::taglineSize);

    Text::drawTracked (g, "TEMPO-SYNCED DELAY", tagFont, tagTracking,
                       { Layout::taglineX, Layout::taglineY1, 400.0f, Layout::taglineLineHeight },
                       juce::Justification::left, Colour::panelText);

    Text::drawTracked (g, "MODEL DL-88 " + Text::middleDot() + " STEREO", tagFontMedium, tagTracking,
                       { Layout::taglineX, Layout::taglineY2, 400.0f, Layout::taglineLineHeight },
                       juce::Justification::left, Colour::panelText);

    Text::drawTracked (g, "PROGRAM", Font::label (10.0f), Font::trackingPx (0.28f, 10.0f),
                       { Layout::programLabelX, Layout::programLabelY, 200.0f, 12.5f },
                       juce::Justification::left, Colour::panelText);

    const auto captionFont = Font::label (10.0f);
    const float captionTracking = Font::trackingPx (0.26f, 10.0f);

    Text::drawTracked (g, "IN", captionFont, captionTracking,
                       { Layout::meterInX, Layout::meterCaptionY, Layout::meterBoxW, 12.5f },
                       juce::Justification::centred, Colour::panelText);
    Text::drawTracked (g, "OUT", captionFont, captionTracking,
                       { Layout::meterOutX, Layout::meterCaptionY, Layout::meterBoxW, 12.5f },
                       juce::Justification::centred, Colour::panelText);

    g.setColour (Colour::headerRule);
    g.fillRect (Layout::fasciaContentX, Layout::headerRuleY, Layout::fasciaContentW, 1.0f);

    // Scope caption row - the lamp itself is live and drawn by RepeatTimelineScope.
    Text::drawTracked (g, "REPEATS LIVE", Font::label (12.0f), Font::trackingPx (0.26f, 12.0f),
                       { Layout::lampLabelX, Layout::scopeCaptionY, 300.0f, Layout::scopeCaptionH },
                       juce::Justification::left, Colour::label);

    Text::drawTracked (g, "REPEAT TIMELINE", Font::label (11.0f), Font::trackingPx (0.24f, 11.0f),
                       { Layout::headerContentX, Layout::scopeCaptionY,
                         Layout::headerContentW, Layout::scopeCaptionH },
                       juce::Justification::right, Colour::panelText);
}

void PanelBackground::paintPanelChrome (juce::Graphics& g)
{
    drawPanelBox (g, { Layout::timingX,  Layout::rowAY, Layout::timingW,  Layout::rowAH }, "TIMING");
    drawPanelBox (g, { Layout::repeatsX, Layout::rowAY, Layout::repeatsW, Layout::rowAH }, "REPEATS");
    drawPanelBox (g, { Layout::outputX,  Layout::rowAY, Layout::outputW,  Layout::rowAH }, "OUTPUT");
    drawPanelBox (g, { Layout::scopeX,   Layout::rowBY, Layout::scopeW,   Layout::rowBH }, "DELAY CHARACTER");

    // The two vertical rules inside DELAY CHARACTER.
    g.setColour (Colour::divider);
    g.fillRect (Layout::dialsDividerX, Layout::rowBDividerY, 1.0f, Layout::rowBDividerH);
    g.fillRect (Layout::sharedDividerX, Layout::rowBDividerY, 1.0f, Layout::rowBDividerH);
}

void PanelBackground::paintStaticLabels (juce::Graphics& g)
{
    // Knob captions. No LED belongs to any of these except TIME and CROSS-FEED, whose lamps are
    // drawn live by PanelControls; the text itself never changes.
    drawKnobCaption (g, "FEEDBACK", Layout::feedbackKnobCentre.x, Layout::repeatsLabelY, Colour::label);
    drawKnobCaption (g, "MIX", Layout::mixKnobCentre.x, Layout::outputLabelY, Colour::label);
    drawKnobCaption (g, "OUTPUT TRIM", Layout::trimKnobCentre.x, Layout::outputLabelY);
    drawKnobCaption (g, "DAMPING", Layout::dampingKnobCentre.x, Layout::sharedLabelY);
    drawKnobCaption (g, "SATURATION", Layout::saturationKnobCentre.x, Layout::sharedLabelY);

    Text::drawTracked (g, "NOTE DIVISION", Font::label (10.0f), Font::trackingPx (0.24f, 10.0f),
                       { Layout::divisionLabelX, Layout::divisionLabelY, 200.0f, 12.5f },
                       juce::Justification::left, Colour::panelText);

    Text::drawTracked (g, "STEREO MODE", Font::label (10.0f), Font::trackingPx (0.24f, 10.0f),
                       { Layout::stereoLabelX, Layout::stereoLabelY, 200.0f, 12.5f },
                       juce::Justification::left, Colour::panelText);

    Text::drawTracked (g, "CHARACTER " + Text::middleDot() + " ALL MODES",
                       Font::label (10.0f), Font::trackingPx (0.24f, 10.0f),
                       { Layout::sharedHeadingX, Layout::sharedHeadingY, 250.0f, 12.5f },
                       juce::Justification::left, Colour::panelText);
}

void PanelBackground::paintFoot (juce::Graphics& g)
{
    g.setColour (Colour::footRule);
    g.fillRect (Layout::fasciaContentX, Layout::footRuleY, Layout::fasciaContentW, 1.0f);

    // Recessed label window - a strip of tape someone wrote the channel on.
    const auto stencilFont = Font::stencil (11.0f);
    const float stencilTracking = Font::trackingPx (0.10f, 11.0f);
    const juce::String channelText { "CH 4 " + juce::String::charToString ((juce::juce_wchar) 0x2014)
                                     + " GTR / STAGE LEFT" };
    const float textWidth = Text::trackedWidth (channelText, stencilFont, stencilTracking);

    const juce::Rectangle<float> window { Layout::footWindowX, Layout::footWindowY,
                                          textWidth + 26.0f, Layout::footWindowH };

    g.setGradientFill (Paint::vertical (window, juce::Colour (0xFF0A0A09), juce::Colour (0xFF141312)));
    g.fillRect (window);

    g.setColour (juce::Colour (0xFF33312B));
    g.drawRect (window, 1.0f);

    Text::drawTracked (g, channelText, stencilFont, stencilTracking,
                       { Layout::footWindowTextX, Layout::footWindowTextY, textWidth + 4.0f, 15.2f },
                       juce::Justification::left, Colour::panelText);

    const auto footFont = Font::label (10.0f);
    const float footTracking = Font::trackingPx (0.26f, 10.0f);
    const auto dot = Text::middleDot();

    Text::drawTracked (g, "DL-88 " + dot + " TOURING SPEC " + dot + " 5U", footFont, footTracking,
                       { Layout::footSpecX, Layout::footTextY, 400.0f, 12.5f },
                       juce::Justification::left, Colour::panelText);

    // Section 9: a static serial, not a state. NF_VERSION_SHORT rather than the spec's literal
    // "v1.0" - the project is at 0.1.0, so hard-coding 1.0 would print a version that does not
    // exist, and the macro reaches 1.0 on its own when the project does.
    Text::drawTracked (g, juce::String (Layout::footSerial) + " " + dot + " v" NF_VERSION_SHORT,
                       footFont, footTracking,
                       { Layout::headerContentX, Layout::footTextY, Layout::headerContentW, 12.5f },
                       juce::Justification::right, Colour::panelText);
}
