#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <BinaryData.h>

#include <array>
#include <cmath>

/**
    Fifth Member's design tokens: every colour, coordinate, size and typographic constant.

    The prototype expresses layout as CSS flexbox, which cannot be transcribed directly - the house
    model is absolute-coordinate painting against a fixed reference canvas with no resized()
    anywhere. Everything below was resolved from that CSS by hand.

    Two corrections to design/README.md found while doing it, both load-bearing:

    1. The doc says the fascia is 1084 px. It is **1100**. 1084 = 1240 - 52 - 52 - 52: one rack ear
       subtracted twice. The unit is 52 (left ear) + 1136 (fascia border box) + 52 (right ear), and
       the fascia's content box is 1136 - 36 px padding = 1100 at origin x = 18. Every horizontal
       number here derives from that.

    2. The panel height is never stated. It resolves to ~854.5 px from the CSS; 855 is used.

    Nothing on this panel is content-box in the CSS sense by accident - the prototype has no
    box-sizing reset, so every declared width/height excludes its border. That is already folded
    into the numbers below.
*/
namespace FifthMemberTheme
{

//==============================================================================
namespace Colour
{
    // --- chassis & fascia ----------------------------------------------------
    inline const juce::Colour unitBg          { 0xFF0C0C0B };
    inline const juce::Colour fasciaTop       { 0xFF1B1A18 };
    inline const juce::Colour fasciaMid       { 0xFF131211 };   // at 42%
    inline const juce::Colour fasciaBottom    { 0xFF0E0E0C };
    inline const juce::Colour panelBorder     { juce::Colour::fromRGBA (255, 255, 255, 19) };  // .075
    inline const juce::Colour divider         { juce::Colour::fromRGBA (255, 255, 255, 18) };  // .07
    inline const juce::Colour headerRule      { juce::Colour::fromRGBA (255, 255, 255, 18) };
    inline const juce::Colour footRule        { juce::Colour::fromRGBA (255, 255, 255, 15) };  // .06
    inline const juce::Colour wearMetal       { 0xFFC4BEB2 };

    // --- metal ---------------------------------------------------------------
    inline const juce::Colour earLight        { 0xFFC2BEB4 };
    inline const juce::Colour earMid          { 0xFF9D998F };
    inline const juce::Colour earDark         { 0xFF75726B };
    inline const juce::Colour screwSlot       { 0xFF0B0A08 };
    inline const juce::Colour gafferLight     { 0xFFD9D4C6 };
    inline const juce::Colour gafferMid       { 0xFFBDB7A7 };
    inline const juce::Colour gafferDark      { 0xFFA8A293 };
    inline const juce::Colour cableTapeLight  { 0xFFE8E3D3 };
    inline const juce::Colour cableTapeDark   { 0xFFCDC7B6 };
    inline const juce::Colour markerInk       { 0xFF151310 };
    inline const juce::Colour cableTapeInk    { 0xFF20201C };

    // --- LCD & phosphor ------------------------------------------------------
    inline const juce::Colour lcdTop          { 0xFF071009 };
    inline const juce::Colour lcdBottom       { 0xFF040806 };
    inline const juce::Colour lcdBorder       { 0xFF2A2823 };
    inline const juce::Colour lcdText         { 0xFFCFD8CB };
    inline const juce::Colour lcdTextDim      { 0xFF9FB2A2 };
    inline const juce::Colour listBg          { 0xFF060D09 };
    inline const juce::Colour meterBg         { 0xFF05080A };
    inline const juce::Colour meterText       { 0xFFB9C3C8 };

    // --- scope ---------------------------------------------------------------
    inline const juce::Colour scopeBg         { 0xFF04060A };
    inline const juce::Colour scopeBorder     { 0xFF23221E };
    inline const juce::Colour scopeGrid       { juce::Colour::fromRGBA (150, 175, 155, 26) };  // .10
    inline const juce::Colour scopeStripRule  { juce::Colour::fromRGBA (150, 175, 155, 33) };  // .13
    inline const juce::Colour scopeBaseline   { juce::Colour::fromRGBA (160, 180, 165, 56) };  // .22
    inline const juce::Colour scopeReadout    { 0xFF7D8D7E };
    inline const juce::Colour scopeReadoutHi  { 0xFFA9BDA9 };
    inline const juce::Colour scopeGhost      { juce::Colour::fromRGBA (190, 200, 190, 18) };  // .07
    inline const juce::Colour scopePingMarker { juce::Colour::fromRGBA (200, 210, 200, 41) };  // .16

    // --- type ----------------------------------------------------------------
    inline const juce::Colour labelBright     { 0xFFF0EADE };
    inline const juce::Colour label           { 0xFFC3BCAE };
    inline const juce::Colour labelMid        { 0xFFA49D92 };
    inline const juce::Colour labelMidAlt     { 0xFFA9A297 };
    inline const juce::Colour labelDim        { 0xFF857F75 };
    inline const juce::Colour labelDimAlt     { 0xFF8D877C };
    inline const juce::Colour labelDimSoft    { 0xFF7E786E };
    inline const juce::Colour labelFaint      { 0xFF6D685F };
    inline const juce::Colour labelFainter    { 0xFF615C54 };
    inline const juce::Colour labelFaintest   { 0xFF57534C };
    inline const juce::Colour labelDisabled   { 0xFF4D4941 };
    inline const juce::Colour footLabel       { 0xFFA09883 };
    inline const juce::Colour scopeCaption    { 0xFF79746B };

    // --- controls ------------------------------------------------------------
    /** The one accent, per BRAND.md. Reserved for the scope lamp and the pulse train - the only
        coloured light on the panel. Every LED is warm white, never coloured. */
    inline const juce::Colour accent          { 0xFFFF9D3C };

    inline const juce::Colour ledOnCore       { 0xFFFFFFFF };
    inline const juce::Colour ledOnMid        { 0xFFEFE9D6 };
    inline const juce::Colour ledOnEdge       { 0xFFB9B09A };
    inline const juce::Colour ledOffCore      { 0xFF3A3833 };
    inline const juce::Colour ledOffEdge      { 0xFF191816 };
    inline const juce::Colour ledOffCoreSmall { 0xFF34322D };
    inline const juce::Colour ledOffEdgeSmall { 0xFF171614 };

    inline const juce::Colour buttonTop       { 0xFF2B2924 };
    inline const juce::Colour buttonBottom    { 0xFF171613 };
    inline const juce::Colour buttonTopHover  { 0xFF332F29 };
    inline const juce::Colour buttonBotHover  { 0xFF1B1A16 };
    inline const juce::Colour buttonBorder    { 0xFF0A0A08 };

    inline const juce::Colour saveTop         { 0xFF2A2823 };
    inline const juce::Colour saveBottom      { 0xFF161512 };
    inline const juce::Colour saveTopHover    { 0xFF35322B };
    inline const juce::Colour saveBotHover    { 0xFF1C1A16 };
    inline const juce::Colour deleteTop       { 0xFF242219 };
    inline const juce::Colour deleteBottom    { 0xFF141310 };
    inline const juce::Colour buttonText      { 0xFFCEC7BA };

    inline const juce::Colour switchTrack     { 0xFF0B0A09 };
    inline const juce::Colour switchBorder    { 0xFF2B2924 };
    inline const juce::Colour switchThumbTop  { 0xFFCFC9BB };
    inline const juce::Colour switchThumbBot  { 0xFF8D887D };

    inline const juce::Colour tooltipBg       { 0xFF0B0A09 };
    inline const juce::Colour tooltipBorder   { 0xFF35322B };
    inline const juce::Colour tooltipText     { 0xFFECE6D8 };

    inline const juce::Colour tickMark        { 0xFF7D786E };
    inline const juce::Colour tickMarkMid     { 0xFF827D72 };
    inline const juce::Colour tickMarkLarge   { 0xFF85806F };
}

//==============================================================================
namespace Font
{
    /** Function-local statics: created once, lazily, thread-safely. JUCE's binary-data name
        mangling STRIPS non-alphanumerics rather than converting them, so BarlowCondensed-SemiBold
        becomes BarlowCondensedSemiBold_ttf. */
    inline juce::Typeface::Ptr barlowMedium()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::BarlowCondensedMedium_ttf, (size_t) BinaryData::BarlowCondensedMedium_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr barlowSemiBold()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::BarlowCondensedSemiBold_ttf, (size_t) BinaryData::BarlowCondensedSemiBold_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr barlowBold()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::BarlowCondensedBold_ttf, (size_t) BinaryData::BarlowCondensedBold_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr shareTechMono()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::ShareTechMonoRegular_ttf, (size_t) BinaryData::ShareTechMonoRegular_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr permanentMarker()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PermanentMarkerRegular_ttf, (size_t) BinaryData::PermanentMarkerRegular_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr specialElite()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::SpecialEliteRegular_ttf, (size_t) BinaryData::SpecialEliteRegular_ttfSize);
        return t;
    }

    /** Builds a font whose em size equals the design's CSS px value.

        This is what `font-size: 12px` means, and it is NOT juce::Font::withHeight(), which sets
        ascent+descent - a typeface-specific multiple of the em, so passing a CSS px straight to
        withHeight() renders visibly small. Gatecrasher and CHORUS-60 both needed a calibration
        constant for this; JUCE 8's withPointHeight() expresses it directly. */
    inline juce::Font of (juce::Typeface::Ptr face, float cssPx)
    {
        return juce::Font (juce::FontOptions (face).withPointHeight (cssPx));
    }

    inline juce::Font label (float cssPx)  { return of (barlowSemiBold(), cssPx); }
    inline juce::Font labelMedium (float cssPx) { return of (barlowMedium(), cssPx); }
    inline juce::Font labelBold (float cssPx) { return of (barlowBold(), cssPx); }
    inline juce::Font mono (float cssPx)   { return of (shareTechMono(), cssPx); }
    inline juce::Font marker (float cssPx) { return of (permanentMarker(), cssPx); }
    inline juce::Font stencil (float cssPx){ return of (specialElite(), cssPx); }

    /** CSS letter-spacing is in em, so its pixel value scales with the font size. Every tracking
        figure in the design is quoted in em - always convert through here. */
    inline constexpr float trackingPx (float em, float cssPx) noexcept { return em * cssPx; }
}

//==============================================================================
namespace Text
{
    inline juce::String middleDot()
    {
        // Built from the codepoint: juce::String's const char* constructor decodes Latin-1, not
        // UTF-8, so a "\xc2\xb7" literal renders as a stray "A-circumflex" on the panel.
        return juce::String::charToString ((juce::juce_wchar) 0x00B7);
    }

    inline float trackedWidth (const juce::String& text, const juce::Font& font, float tracking)
    {
        float width = 0.0f;

        for (int i = 0; i < text.length(); ++i)
        {
            width += juce::GlyphArrangement::getStringWidth (font, juce::String::charToString (text[i]));

            if (i < text.length() - 1)
                width += tracking;
        }

        return width;
    }

    /** juce::Font has no absolute-pixel letter-spacing, so tracked text is drawn glyph by glyph.
        Ported from TapeRot, which needed the same thing for its SVG's letter-spacing attribute. */
    inline void drawTracked (juce::Graphics& g, const juce::String& text, const juce::Font& font,
                             float tracking, juce::Rectangle<float> area,
                             juce::Justification justification, juce::Colour colour)
    {
        g.setFont (font);
        g.setColour (colour);

        const float total = trackedWidth (text, font, tracking);
        float x = area.getX();

        if (justification.testFlags (juce::Justification::horizontallyCentred))
            x = area.getCentreX() - total * 0.5f;
        else if (justification.testFlags (juce::Justification::right))
            x = area.getRight() - total;

        for (int i = 0; i < text.length(); ++i)
        {
            const auto ch = juce::String::charToString (text[i]);
            const float w = juce::GlyphArrangement::getStringWidth (font, ch);

            g.drawText (ch, juce::Rectangle<float> (x, area.getY(), w + 1.0f, area.getHeight()),
                        juce::Justification::centredLeft, false);

            x += w + tracking;
        }
    }
}

//==============================================================================
namespace Layout
{
    inline constexpr float canvasWidth = 1240.0f;
    inline constexpr float canvasHeight = 855.0f;
    inline constexpr float unitRadius = 5.0f;

    inline constexpr float earWidth = 52.0f;
    inline constexpr float fasciaX = earWidth;
    inline constexpr float fasciaContentX = 18.0f;      // fascia padding is 18px horizontally
    inline constexpr float fasciaContentW = 1100.0f;    // NOT the doc's 1084 - see the file comment

    inline constexpr float screwDiameter = 24.0f;
    inline constexpr std::array<juce::Point<float>, 4> screwCentres { {
        { 26.0f, 34.0f }, { 26.0f, 821.0f }, { 1214.0f, 34.0f }, { 1214.0f, 821.0f } } };
    /** Kept distinct on purpose: four identical screws read as CG. */
    inline constexpr std::array<float, 4> screwSlotAngles { { 28.0f, -14.0f, 52.0f, 9.0f } };

    // --- header --------------------------------------------------------------
    inline constexpr float headerContentX = 22.0f;
    inline constexpr float headerContentW = 1092.0f;
    inline constexpr float headerRuleY = 127.34f;

    inline constexpr float nameplateX = 22.0f;
    inline constexpr float nameplateY = 16.0f;
    inline constexpr float nameplateW = 326.0f;
    inline constexpr float nameplateH = 52.34f;
    inline constexpr float nameplateRotationDegrees = -1.2f;
    inline constexpr float nameplateTextSize = 33.0f;

    inline constexpr float taglineX = 26.0f;
    inline constexpr float taglineY1 = 80.34f;
    inline constexpr float taglineY2 = 98.34f;
    inline constexpr float taglineSize = 12.0f;
    inline constexpr float taglineLineHeight = 15.0f;

    inline constexpr float programLabelX = 376.0f;
    inline constexpr float programLabelY = 32.42f;
    inline constexpr float lcdX = 374.0f;
    inline constexpr float lcdY = 50.92f;
    inline constexpr float lcdW = 391.0f;
    inline constexpr float lcdH = 46.0f;
    inline constexpr float lcdRadius = 3.0f;
    inline constexpr float bankTagW = 61.0f;
    inline constexpr float caretW = 36.0f;

    inline constexpr float saveX = 775.0f;
    inline constexpr float saveW = 68.0f;
    inline constexpr float deleteX = 850.0f;
    inline constexpr float deleteW = 76.0f;
    inline constexpr float headerButtonY = 50.92f;
    inline constexpr float headerButtonH = 46.0f;

    inline constexpr float meterCaptionY = 38.92f;
    inline constexpr float meterBoxY = 56.42f;
    inline constexpr float meterBoxW = 76.0f;
    inline constexpr float meterBoxH = 34.0f;
    inline constexpr float meterInX = 952.0f;
    inline constexpr float meterOutX = 1038.0f;

    // --- scope ---------------------------------------------------------------
    inline constexpr float scopeCaptionY = 142.34f;
    inline constexpr float scopeCaptionH = 15.0f;
    inline constexpr juce::Point<float> lampCentre { 28.5f, 149.84f };
    inline constexpr float lampDiameter = 13.0f;
    inline constexpr float lampLabelX = 46.0f;

    inline constexpr float scopeX = 22.0f;
    inline constexpr float scopeY = 165.34f;
    inline constexpr float scopeW = 1092.0f;
    inline constexpr float scopeH = 98.0f;

    /** The dark box splits into a readout strip over a plot area. All variable text lives in the
        strip, in Share Tech Mono - the same face as the PROGRAM LCD - and never in the plot. That
        combination of placement and typeface is what makes changing text read as a screen rather
        than a printed label. */
    inline constexpr float scopeInnerX = 23.0f;
    inline constexpr float scopeInnerY = 166.34f;
    inline constexpr float scopeInnerW = 1090.0f;
    inline constexpr float readoutStripH = 22.0f;
    inline constexpr float plotY = 188.34f;
    inline constexpr float plotH = 74.0f;
    inline constexpr float readoutTextSize = 11.0f;
    inline constexpr float readoutTracking = 0.12f;
    inline constexpr float readoutPadX = 9.0f;
    inline constexpr float readoutGap = 16.0f;

    // Scope draw model, from the prototype's canvas loop.
    inline constexpr float pulseWindowMs = 2600.0f;
    inline constexpr float baselineInset = 9.0f;        // baseline y = plotH - 9
    inline constexpr float spanInset = 8.0f;            // span = baseline - 8
    inline constexpr float heightExponent = 0.45f;      // late repeats stay visible; do not linearise
    inline constexpr float dryResetThreshold = 0.03f;
    inline constexpr int   gridDivisions = 8;

    // --- control row A -------------------------------------------------------
    inline constexpr float rowAY = 285.34f;
    inline constexpr float rowAH = 244.0f;
    inline constexpr float legendTextSize = 10.0f;
    inline constexpr float legendTracking = 0.28f;

    inline constexpr float timingX = 22.0f;
    inline constexpr float timingW = 296.0f;
    inline constexpr float repeatsX = 334.0f;
    inline constexpr float repeatsW = 464.0f;
    inline constexpr float outputX = 814.0f;
    inline constexpr float outputW = 300.0f;

    inline constexpr float syncSwitchX = 39.0f;
    inline constexpr float syncSwitchY = 306.34f;
    inline constexpr float syncSwitchW = 54.0f;
    inline constexpr float syncSwitchH = 26.0f;
    inline constexpr float syncThumbOffX = 42.0f;
    inline constexpr float syncThumbOnX = 67.0f;
    inline constexpr float syncThumbY = 309.34f;
    inline constexpr float syncThumbW = 23.0f;
    inline constexpr float syncThumbH = 20.0f;
    inline constexpr float syncThumbAnimMs = 180.0f;
    inline constexpr float syncCaptionX = 105.0f;
    inline constexpr float syncCaptionY = 312.465f;

    inline constexpr juce::Point<float> divisionLedCentre { 42.5f, 352.59f };
    inline constexpr float divisionLabelX = 53.0f;
    inline constexpr float divisionLabelY = 346.34f;
    inline constexpr float divisionButtonY = 365.84f;
    inline constexpr float divisionButtonW = 48.4f;
    inline constexpr float divisionButtonH = 32.0f;
    inline constexpr float divisionButtonPitch = 53.4f;
    inline constexpr float divisionButtonX0 = 39.0f;

    inline constexpr juce::Point<float> timeKnobCentre { 170.0f, 447.84f };
    inline constexpr float timeLabelY = 495.84f;
    inline constexpr juce::Point<float> timeLedCentre { 154.0f, 502.09f };

    inline constexpr juce::Point<float> feedbackKnobCentre { 458.0f, 351.34f };
    inline constexpr juce::Point<float> crossFeedKnobCentre { 678.0f, 363.34f };
    inline constexpr float repeatsLabelY = 409.34f;
    inline constexpr juce::Point<float> crossFeedLedCentre { 642.0f, 415.59f };

    inline constexpr float stereoLabelX = 351.0f;
    inline constexpr float stereoLabelY = 437.84f;
    inline constexpr float stereoButtonY = 457.34f;
    inline constexpr float stereoButtonW = 140.0f;
    inline constexpr float stereoButtonH = 34.0f;
    inline constexpr std::array<float, 3> stereoButtonX { { 351.0f, 496.0f, 641.0f } };

    inline constexpr juce::Point<float> mixKnobCentre { 909.85f, 395.59f };
    inline constexpr juce::Point<float> trimKnobCentre { 1025.0f, 406.59f };
    inline constexpr float outputLabelY = 452.59f;

    // --- control row B: DELAY CHARACTER --------------------------------------
    inline constexpr float rowBY = 551.34f;
    inline constexpr float rowBH = 242.0f;

    inline constexpr float modeButtonX = 43.0f;
    inline constexpr float modeButtonW = 168.0f;
    inline constexpr float modeButtonH = 56.0f;
    inline constexpr float modeButtonPitch = 68.0f;
    inline constexpr float modeButtonY0 = 578.34f;
    inline constexpr float modeLedX = 63.0f;
    inline constexpr float modeLedDiameter = 10.0f;
    inline constexpr float modeLabelX = 80.0f;
    inline constexpr float modeLabelSize = 13.0f;

    inline constexpr float dialsDividerX = 237.0f;
    inline constexpr float sharedDividerX = 843.0f;
    inline constexpr float rowBDividerY = 578.34f;
    inline constexpr float rowBDividerH = 192.0f;

    inline constexpr std::array<float, 3> dialCentreX { { 356.833f, 540.5f, 724.167f } };
    inline constexpr float dialCentreY = 643.34f;
    inline constexpr float dialLabelStackTop = 697.34f;
    inline constexpr float dialLabelRowPitch = 17.5f;
    inline constexpr float dialLabelSize = 10.0f;
    inline constexpr float dialLabelTracking = 0.18f;
    inline constexpr float dialLedDiameter = 6.0f;
    inline constexpr float dialLedOffsetX = 3.0f;
    inline constexpr float dialLedOffsetY = 6.25f;
    inline constexpr float dialLabelTextOffsetX = 13.0f;

    inline constexpr float sharedHeadingX = 870.0f;
    inline constexpr float sharedHeadingY = 568.34f;
    inline constexpr juce::Point<float> dampingKnobCentre { 923.25f, 649.84f };
    inline constexpr juce::Point<float> saturationKnobCentre { 1039.75f, 649.84f };
    inline constexpr float sharedLabelY = 697.84f;

    // --- foot ----------------------------------------------------------------
    inline constexpr float footRuleY = 805.34f;
    inline constexpr float footWindowX = 22.0f;
    inline constexpr float footWindowY = 816.34f;
    inline constexpr float footWindowH = 26.2f;
    inline constexpr float footWindowTextX = 35.0f;
    inline constexpr float footWindowTextY = 821.34f;
    inline constexpr float footSpecX = 216.0f;
    inline constexpr float footTextY = 823.19f;

    // --- knobs ---------------------------------------------------------------
    inline constexpr float knobArcStartDegrees = -135.0f;
    inline constexpr float knobArcEndDegrees   =  135.0f;
    /** 190 px of vertical drag spans the full range - the design's figure, not JUCE's default. */
    inline constexpr int knobDragPixels = 190;

    enum class KnobSize { small62, standard66, dial76, primary82, primary84 };

    struct KnobVariant
    {
        float diameter;         // the body, content box
        float pointerWidth;
        float pointerLength;
        float pointerTopOffset;
        float tickInset;        // negative: the ring sits outside the body
        float tickStepDegrees;
        float tickWidth;
        juce::Colour faceTop, faceMid, faceBottom;
        juce::Colour tickColour;
        float slewMs;           // the character dials re-set more slowly than the rest
    };

    inline const KnobVariant knob62 { 62.0f, 2.0f, 24.0f, 6.0f, -10.0f, 27.0f, 1.4f,
                                      juce::Colour (0xFF4C4942), juce::Colour (0xFF262420),
                                      juce::Colour (0xFF121110), Colour::tickMark, 620.0f };
    inline const KnobVariant knob66 { 66.0f, 2.0f, 26.0f, 6.0f, -10.0f, 27.0f, 1.4f,
                                      juce::Colour (0xFF4C4942), juce::Colour (0xFF262420),
                                      juce::Colour (0xFF121110), Colour::tickMark, 620.0f };
    inline const KnobVariant knob76 { 76.0f, 2.5f, 30.0f, 7.0f, -11.0f, 24.0f, 1.3f,
                                      juce::Colour (0xFF514E46), juce::Colour (0xFF282621),
                                      juce::Colour (0xFF121110), Colour::tickMarkMid, 660.0f };
    inline const KnobVariant knob82 { 82.0f, 3.0f, 32.0f, 7.0f, -11.0f, 22.5f, 1.2f,
                                      juce::Colour (0xFF55524A), juce::Colour (0xFF2A2823),
                                      juce::Colour (0xFF131210), Colour::tickMarkLarge, 620.0f };
    inline const KnobVariant knob84 { 84.0f, 3.0f, 33.0f, 7.0f, -11.0f, 22.5f, 1.2f,
                                      juce::Colour (0xFF55524A), juce::Colour (0xFF2A2823),
                                      juce::Colour (0xFF131210), Colour::tickMarkLarge, 620.0f };

    inline const KnobVariant& variantFor (KnobSize s)
    {
        switch (s)
        {
            case KnobSize::small62:    return knob62;
            case KnobSize::dial76:     return knob76;
            case KnobSize::primary82:  return knob82;
            case KnobSize::primary84:  return knob84;
            case KnobSize::standard66:
            default:                   return knob66;
        }
    }

    // --- LEDs ----------------------------------------------------------------
    inline constexpr float ledSmall = 6.0f;     // character dial labels
    inline constexpr float ledStandard = 7.0f;  // knob captions, division/stereo buttons
    inline constexpr float ledLarge = 10.0f;    // character mode buttons

    // --- animation -----------------------------------------------------------
    /** Chorus-60's slew law: 1 - 0.002^(dt/settleMs), time-based so travel takes the same wall
        time whatever the frame rate and a dropped frame lengthens the step rather than shortening
        the motion. */
    inline constexpr float slewRemainderAtSettle = 0.002f;
    inline constexpr int animationHz = 60;

    /** The character mode-change "re-arm": every dial snaps to minimum, then sweeps up to its new
        value. design/README.md calls this "the panel physically re-setting itself, the way real
        recall works" and asks for it to be kept. */
    inline constexpr float rearmHoldMs = 40.0f;
}

//==============================================================================
namespace Geometry
{
    /** Degrees clockwise from 12 o'clock - the design's own rotation convention, not JUCE's. */
    inline float knobAngleForValue (float value01) noexcept
    {
        return Layout::knobArcStartDegrees
             + value01 * (Layout::knobArcEndDegrees - Layout::knobArcStartDegrees);
    }

    inline juce::Point<float> directionFor (float degrees) noexcept
    {
        const float r = juce::degreesToRadians (degrees);
        return { std::sin (r), -std::cos (r) };
    }

    inline juce::Point<float> pointOnCircle (juce::Point<float> centre, float radius, float degrees) noexcept
    {
        return centre + directionFor (degrees) * radius;
    }
}

//==============================================================================
namespace Paint
{
    inline juce::ColourGradient vertical (juce::Rectangle<float> r, juce::Colour top, juce::Colour bottom)
    {
        return { top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false };
    }

    /** A CSS radial-gradient(circle at fx%, fy%, ...). CSS sizes to the farthest corner, so the
        radius is the distance from the offset centre to the furthest corner - not the box's own
        radius. Getting that wrong turns a dome into a flat disc with an off-centre blob. */
    inline juce::ColourGradient radial (juce::Rectangle<float> box, float fx, float fy,
                                        juce::Colour c0, juce::Colour c1, float stop1, juce::Colour c2)
    {
        const juce::Point<float> centre { box.getX() + box.getWidth() * fx,
                                          box.getY() + box.getHeight() * fy };
        const float dx = juce::jmax (centre.x - box.getX(), box.getRight() - centre.x);
        const float dy = juce::jmax (centre.y - box.getY(), box.getBottom() - centre.y);
        const float radius = std::sqrt (dx * dx + dy * dy);

        juce::ColourGradient g { c0, centre.x, centre.y, c2, centre.x + radius, centre.y, true };
        g.addColour ((double) stop1, c1);
        return g;
    }

    /** Every LED on the panel. Warm white, never coloured - the accent belongs to the scope lamp
        alone. Sizes: 6 px on the dial labels, 7 px on knob captions and buttons, 10 px on the
        character mode buttons. */
    inline void drawLed (juce::Graphics& g, juce::Point<float> centre, float diameter, bool lit)
    {
        const juce::Rectangle<float> bounds { centre.x - diameter * 0.5f, centre.y - diameter * 0.5f,
                                              diameter, diameter };

        if (lit)
        {
            // Glow scales with the LED, so a 6 px label lamp does not get a 10 px button's halo -
            // and it stays tight. The design quotes these as blur radii on a small lamp; drawn as
            // wide discs of light they wash over the label sitting a few pixels away, which is
            // exactly what happened to NOTE DIVISION, CROSS-FEED and TIME.
            const float glow = diameter * 1.45f;
            juce::ColourGradient halo { Colour::ledOnMid.withAlpha (0.55f), centre.x, centre.y,
                                        Colour::ledOnMid.withAlpha (0.0f), centre.x + glow, centre.y, true };
            halo.addColour (0.40, Colour::ledOnMid.withAlpha (0.22f));
            halo.addColour (0.75, Colour::ledOnMid.withAlpha (0.06f));
            g.setGradientFill (halo);
            g.fillEllipse (centre.x - glow, centre.y - glow, glow * 2.0f, glow * 2.0f);

            g.setGradientFill (radial (bounds, 0.35f, 0.30f, Colour::ledOnCore,
                                       Colour::ledOnMid, 0.45f, Colour::ledOnEdge));
            g.fillEllipse (bounds);
        }
        else
        {
            const bool small = diameter <= Layout::ledSmall;
            g.setGradientFill (radial (bounds, 0.35f, 0.30f,
                                       small ? Colour::ledOffCoreSmall : Colour::ledOffCore,
                                       small ? Colour::ledOffCoreSmall : Colour::ledOffCore, 0.5f,
                                       small ? Colour::ledOffEdgeSmall : Colour::ledOffEdge));
            g.fillEllipse (bounds);

            g.setColour (juce::Colours::black.withAlpha (0.8f));
            g.drawEllipse (bounds.reduced (0.5f), 1.0f);
        }
    }

    /** A division / stereo / character-mode button. Selection is signalled three ways at once -
        the pressed shadow, the label brightening and the LED lighting - and never by colour. That
        redundancy is deliberate: it survives on a dim stage. */
    inline void drawButtonFace (juce::Graphics& g, juce::Rectangle<float> r, bool selected, bool hovered)
    {
        g.setGradientFill (vertical (r,
                                     hovered ? Colour::buttonTopHover : Colour::buttonTop,
                                     hovered ? Colour::buttonBotHover : Colour::buttonBottom));
        g.fillRoundedRectangle (r, 3.0f);

        g.setColour (Colour::buttonBorder);
        g.drawRoundedRectangle (r, 3.0f, 1.0f);

        if (selected)
        {
            // Pressed: a hard inner shadow at the top, and only a thin lit lip at the bottom.
            juce::ColourGradient inner { juce::Colours::black.withAlpha (0.75f), r.getCentreX(), r.getY(),
                                         juce::Colours::transparentBlack, r.getCentreX(), r.getY() + 8.0f, false };
            g.setGradientFill (inner);
            g.fillRoundedRectangle (r, 3.0f);

            g.setColour (juce::Colours::white.withAlpha (0.10f));
            g.drawLine (r.getX() + 3.0f, r.getBottom() - 0.5f, r.getRight() - 3.0f, r.getBottom() - 0.5f, 1.0f);
        }
        else
        {
            g.setColour (juce::Colours::white.withAlpha (0.13f));
            g.drawLine (r.getX() + 3.0f, r.getY() + 1.5f, r.getRight() - 3.0f, r.getY() + 1.5f, 1.0f);
        }
    }

    inline void drawLcdWell (juce::Graphics& g, juce::Rectangle<float> r, float radius = Layout::lcdRadius)
    {
        g.setGradientFill (vertical (r, Colour::lcdTop, Colour::lcdBottom));
        g.fillRoundedRectangle (r, radius);

        g.setColour (Colour::lcdBorder);
        g.drawRoundedRectangle (r, radius, 1.0f);
    }
}

} // namespace FifthMemberTheme
