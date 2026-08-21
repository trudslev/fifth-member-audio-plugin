#include "PanelBackground.h"

PanelBackground::PanelBackground()
{
    // Pure background: never intercepts a click, so every control layered over it still receives
    // its own.
    setInterceptsMouseClicks (false, false);
}

/*  §1's rack ears: 52 px each side, full height, OUTSIDE the frame.

    They are the reason this casting's window is 1444 where the shared frame is 1340, and they are
    not decoration — they carry three of the panel's five identity marks. §1 records the alternative
    that was rejected (16 px bezels) and why: it would have removed those marks to protect a window
    dimension, which inverts the round's scope.

    Drawn rather than baked because the plate is the frame. A plate covering the window would have
    had to be re-cut at 1444 and would then disagree with every other casting's 1340.  */
void PanelBackground::paintEar (juce::Graphics& g, juce::Rectangle<float> ear, bool mirrored) const
{
    using namespace FifthMemberTheme;

    /*  §1's five stops, run left-to-right on the left ear and right-to-left on the right, so both
        catch light from the same side. A gradient copied unmirrored makes the right ear look lit
        from the wrong direction — which reads as a rendering fault rather than as a style.  */
    const float x0 = mirrored ? ear.getRight() : ear.getX();
    const float x1 = mirrored ? ear.getX() : ear.getRight();

    juce::ColourGradient chassis (Colour::earEdge, x0, ear.getY(),
                                  Colour::earFarEdge, x1, ear.getY(), false);
    chassis.addColour (0.22, Colour::earHighlight);
    chassis.addColour (0.52, Colour::earFace);
    chassis.addColour (0.78, Colour::earShade);
    g.setGradientFill (chassis);
    g.fillRect (ear);

    // The 1 px brush repeat. Vertical strokes on a vertical ear read as machined metal; horizontal
    // ones read as a gradient with a texture laid over it.
    g.setColour (Colour::earBrush);
    for (float x = ear.getX() + 1.0f; x < ear.getRight(); x += 3.0f)
        g.fillRect (x, ear.getY(), 1.0f, ear.getHeight());

    // Two Ø22 screws per ear, on the ear rather than on the fascia — they are what fixes the unit
    // into a rack, so they belong to the chassis and not to the panel.
    for (const auto& c : Layout::screwCentres)
    {
        if (! ear.contains (c))
            continue;

        const float r = Layout::screwDiameter * 0.5f;
        juce::ColourGradient face (Colour::earScrewFace,
                                   c.x + r * (0.38f - 0.5f) * 2.0f, c.y + r * (0.30f - 0.5f) * 2.0f,
                                   Colour::earScrewFace.darker (0.6f), c.x + r, c.y + r, true);
        g.setGradientFill (face);
        g.fillEllipse (c.x - r, c.y - r, Layout::screwDiameter, Layout::screwDiameter);
    }
}

/*  §1's three identity marks, and they are the whole reason the ears exist.

    "The rack ears carry three of its five identity marks - the RACK 4 stencil, the DLY 4 cable tape
    and the HALDEN HALL tape." §1 records that the alternative to ears was 16 px bezels, which
    "would have removed those three marks to protect a window dimension". So a panel with ears and
    no marks is the cost paid and the thing bought thrown away.

    All three sit on the ears rather than the fascia, which is what makes them read as things done
    TO the unit rather than printed on it: a stencil sprayed down the chassis, and two pieces of
    tape a touring engineer stuck on.  */
void PanelBackground::paintEarMarks (juce::Graphics& g) const
{
    using namespace FifthMemberTheme;

    //== Left ear: the RACK 4 stencil, rotated -90 deg ========================
    {
        juce::Graphics::ScopedSaveState state (g);
        const juce::Point<float> pivot { 26.0f, 506.0f };
        g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi,
                                                          pivot.x, pivot.y));

        Text::drawTracked (g, "RACK 4 " + Text::middleDot() + " MON WORLD",
                           Font::label (11.0f), 3.74f,
                           { pivot.x - 90.0f, pivot.y - 7.0f, 180.0f, 14.0f },
                           juce::Justification::centred, Colour::earStencilInk);
    }

    //== Right ear: two pieces of tape ========================================
    const auto tape = [&] (juce::Rectangle<float> box, float degrees, const juce::String& text,
                           float textSize, float tracking, bool vertical)
    {
        juce::Graphics::ScopedSaveState state (g);
        g.addTransform (juce::AffineTransform::rotation (juce::degreesToRadians (degrees),
                                                          box.getCentreX(), box.getCentreY()));

        g.setGradientFill ({ Colour::tapeTop, box.getX(), box.getY(),
                             Colour::tapeBottom, box.getX(), box.getBottom(), false });
        g.fillRect (box);

        juce::Graphics::ScopedSaveState inner (g);

        if (vertical)
            g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi,
                                                              box.getCentreX(), box.getCentreY()));

        const auto row = vertical
            ? juce::Rectangle<float> (box.getCentreX() - box.getHeight() * 0.5f,
                                      box.getCentreY() - 9.0f, box.getHeight(), 18.0f)
            : box.withSizeKeepingCentre (box.getWidth(), 18.0f);

        Text::drawTracked (g, text, Font::marker (textSize), tracking, row,
                           juce::Justification::centred, Colour::tapeInk);
    };

    // The cable tape, near the top: a hand-written channel number.
    tape ({ 1396.0f, 150.0f, 44.0f, 27.0f }, 1.4f, "DLY 4", 12.0f, 0.0f, false);

    // The load-in tape, running down the ear.
    tape ({ 1396.5f, 711.5f, 43.0f, 252.0f }, 1.6f,
          "HALDEN HALL " + Text::middleDot() + " LOAD-IN 06", 14.0f, 0.28f, true);
}

void PanelBackground::paint (juce::Graphics& g)
{
    using namespace FifthMemberTheme;

    //== The ears, either side of the frame ==================================
    paintEar (g, { 0.0f, 0.0f, Layout::earWidth, Layout::canvasHeight }, false);
    paintEar (g, { Layout::canvasWidth - Layout::earWidth, 0.0f,
                   Layout::earWidth, Layout::canvasHeight }, true);

    //== The plate, over the frame ===========================================
    /*  §11: the plate carries the baked scales, the section headings, the control labels and the
        numerals; the build draws the live layers over it — knob bodies and pointers, every button,
        every LED, the sync switch and its caption, the three Delay Character label stacks, dial 1's
        ring, and the contents of the LCD, both meter wells and the scope's readout strip.

        **Redrawing a baked string double-prints it at a one-pixel offset.** That is the failure
        mode while a plate exists, and it is visible — unlike the one TapeRot and Gatecrasher now
        have, where the same element fails by being absent and the panel merely looks emptier.  */
    static const juce::Image plate = juce::ImageFileFormat::loadFrom (
        BinaryData::fifthmemberplate3x_png, (size_t) BinaryData::fifthmemberplate3x_pngSize);

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (plate,
                 juce::Rectangle<float> (Layout::frameX, 0.0f,
                                         Layout::frameWidth, Layout::canvasHeight),
                 juce::RectanglePlacement::stretchToFit);

    // After the plate, because they are on the ears and the plate does not reach them - but drawn
    // here rather than in paintEar so a reader looking for the three marks §1 names finds them in
    // one place with §1's argument beside them.
    paintEarMarks (g);
}
