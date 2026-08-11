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

    2. The panel height is never stated. Summing the CSS gives ~854.5, but that over-counts every
       line box; measured off the rendered prototype it is **848**, and the interior landmarks that
       drifted with it (row A's and row B's heights, the whole foot block) are corrected to match.

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
    /** Section 8.2: 9.69:1 on the LCD substrate. Only the bank tag was promoted to the program
        name's treatment; the chevron keeps this dimmer green. */
    inline const juce::Colour lcdChevron      { 0xFFA9BDA9 };
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
    /** Section 8.2. Nearly every functional role resolves onto two values: #a8a294 at 7.36:1 for
        panel text, #c3bcae at 9.91:1 for knob names. The old ramp (labelMid/labelDim/labelDimSoft/
        labelFaint/...) had a dozen greys, several of which fell below the 7:1 floor for text that
        is functional rather than flavour.

        Four roles stay deliberately dimmer, and section 8.3 documents each: unlit multi-label rows
        (2.82:1) and unselected mode-button labels (7.11:1), because an LED carries the state and
        the brand rule forbids conveying relevance by dimming the control; dial 1's idle ring, the
        same exception in a third place; and disabled DELETE (3.06:1), a disengaged control. */
    inline const juce::Colour labelBright     { 0xFFF0EADE };
    inline const juce::Colour label           { 0xFFC3BCAE };
    inline const juce::Colour panelText       { 0xFFA8A294 };   // 7.36:1 - captions, headings, foot

    /** Section 8.3's two documented exceptions, named so they read as deliberate rather than as
        greys someone forgot to lift. Both stay dim BY DESIGN: an LED carries the state, and
        BRAND.md forbids conveying relevance by dimming the control itself.

        stackLabelUnlit is printed legend for the two modes a dial is not currently driving;
        buttonLabelUnselected is deliberately below the 13.73:1 of a selected label, because it is
        the pairing of a pressed face, a lit lamp and brighter text that marks selection. */
    inline const juce::Colour stackLabelLit        { 0xFFE7E1D4 };   // 14.36:1
    inline const juce::Colour stackLabelUnlit      { 0xFF615C54 };   //  2.82:1 - exception
    inline const juce::Colour buttonLabelUnselected{ 0xFFB0AA9C };   //  7.11:1 - exception
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


    /** Printed scales, section 8.2. These are FUNCTIONAL text at 7.36:1 - they replaced the value
        tooltip as the at-rest reference for what a knob is set to, so they are not decoration and
        do not take a decorative grey. One ink for ticks and numerals alike; minors sit one step
        back only so a labelled mark reads as the primary one.

        The dim set is for dial 1's idle ring (section 4.5) - printed legend for a mode that is not
        live, with an LED and a lit companion ring carrying the current state. Section 8.3 lists it
        as a third instance of the documented multi-label exception, not a new one. */
    inline const juce::Colour scaleTick        { 0xFFA8A294 };
    inline const juce::Colour scaleTickMinor   { 0xFF8A857A };
    inline const juce::Colour scaleNumeral     { 0xFFA8A294 };
    inline const juce::Colour scaleTickDim     { 0xFF5A564E };
    inline const juce::Colour scaleTickMinorDim{ 0xFF4A463F };
    inline const juce::Colour scaleNumeralDim  { 0xFF615C54 };
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
    /** U+2014, from its codepoint: juce::String's const char* constructor decodes Latin-1, so a
        UTF-8 literal would render as stray glyphs. */
    inline juce::String emDash()
    {
        return juce::String::charToString ((juce::juce_wchar) 0x2014);
    }

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

    /** 932, measured off the rendered prototype - NOT the build handoff's stated 996.

        The handoff calls the prototype "authoritative for behaviour and pixel geometry" and the
        spec documents authoritative for values, and every geometric figure the handoff states does
        check out against the render: rack ears 52, LCD 449 x 34, bank cell 75.17 against a stated
        75.2, TIMING 296, OUTPUT 322, the 232 and 144 dial wrappers, the 536 dial row. Only the
        canvas height disagrees, and its own section 4.5 quotes absolute Ys ("body centre 774,
        stack top 895") that are 69.6 and 92.6 px off the render - two different offsets, so those
        figures come from a different layout rather than from a taller version of this one.

        Measured, the chassis is 1240 x 931.45; 932 is that rounded up to a whole pixel. Raised
        with the designers; if a corrected prototype lands, re-measure rather than trusting either
        number. The previous 848 came from the same method on the pre-conformance panel. */
    inline constexpr float canvasHeight = 932.0f;
    inline constexpr float unitRadius = 5.0f;

    /** EVERY coordinate below is absolute against the 1240 x 848 canvas.

        The CSS they came from is nested - the fascia is a flex child sitting after the 52 px left
        rack ear, and its own 18 px padding starts the content box - so the extracted figures were
        all fascia-relative and have had `earWidth` folded in once, here. Anything added later must
        be absolute too: a fascia-relative number lands 52 px left of where it belongs, on top of
        the rack ear, which is exactly the bug this comment exists to prevent recurring. */
    inline constexpr float earWidth = 52.0f;
    inline constexpr float fasciaContentX = 70.0f;      // earWidth + the fascia's own 18px padding
    inline constexpr float fasciaContentW = 1100.0f;    // NOT the doc's 1084 - see the file comment

    inline constexpr float screwDiameter = 24.0f;
    inline constexpr std::array<juce::Point<float>, 4> screwCentres { {
        { 26.0f, 34.0f }, { 26.0f, 898.0f }, { 1214.0f, 34.0f }, { 1214.0f, 898.0f } } };
    /** Kept distinct on purpose: four identical screws read as CG. */
    inline constexpr std::array<float, 4> screwSlotAngles { { 28.0f, -14.0f, 52.0f, 9.0f } };

    // --- header --------------------------------------------------------------
    inline constexpr float headerContentX = 74.0f;
    inline constexpr float headerContentW = 1092.0f;
    inline constexpr float headerRuleY = 126.45f;

    inline constexpr float nameplateX = 73.56f;
    inline constexpr float nameplateY = 13.2f;
    inline constexpr float nameplateW = 268.87f;   // section 7.1 narrowed this column to widen the LCD
    inline constexpr float nameplateH = 50.06f;
    inline constexpr float nameplateRotationDegrees = -1.2f;
        /** 29, not 33. Section 7.1 narrowed this column from 326 to 268.87 to buy the LCD its extra
        width, and the old size no longer fits - it clipped the wordmark to "FIFTH MEMB". Measured
        off the prototype the rendered text box is 234.5 wide inside a 268.87 tape. */
inline constexpr float nameplateTextSize = 29.0f;

    inline constexpr float taglineX = 78.0f;
    inline constexpr float taglineY1 = 72.45f;
    inline constexpr float taglineY2 = 91.45f;
    inline constexpr float taglineSize = 12.0f;
    inline constexpr float taglineLineHeight = 15.0f;

    inline constexpr float programLabelX = 370.0f;
    inline constexpr float programLabelY = 34.22f;

    /** Section 6.1's LCD, and every figure here is BORDER-BOX to match the rest of this file.

        The header came down from 46 px to 34 so the LCD, SAVE, DELETE and the two meter boxes all
        share one height - section 6.1's 447 x 32 content plus its 1 px border. Section 7.1 bought
        the extra width by narrowing the wordmark column: at the old 326 px column the name cell was
        278 px and the two longest live readouts overflowed it, which would have forced a size
        step-down on every readout.

        Cell widths are the render's, and they agree with the spec to a rounding: bank 75.17 against
        a stated 75.2, name 335.83 against 335.8, leaving 36.0 for the chevron. */
    inline constexpr float lcdX = 368.0f;
    inline constexpr float lcdY = 53.22f;
    inline constexpr float lcdW = 449.0f;
    inline constexpr float lcdH = 34.0f;
    inline constexpr float lcdRadius = 3.0f;
    inline constexpr float bankTagW = 75.17f;
    inline constexpr float lcdNameCellX = 444.17f;
    inline constexpr float lcdNameCellW = 335.83f;
    inline constexpr float caretW = 36.0f;

    /** Section 6.4's chevron, drawn as a Path rather than a glyph so it renders identically across
        platforms and font fallbacks. viewBox 0 0 14 8, "M1 1.6 L7 6.4 L13 1.6". */
    inline constexpr float chevronW = 14.0f;
    inline constexpr float chevronH = 8.0f;
    inline constexpr float chevronStroke = 1.6f;
    inline constexpr float chevronPadX = 11.0f;

    /** Section 6.2. 19px Share Tech Mono at .12em advances 12.54 px/char, so the 335.8px name cell
        holds 26 - which is also the cap the build enforces on user Program names. Anything longer
        steps to 16px (10.56 px/char, 31 characters) rather than overrunning. */
    inline constexpr float lcdTextSize = 19.0f;
    inline constexpr float lcdTextSizeGuard = 16.0f;
    inline constexpr float lcdTracking = 0.12f;
    inline constexpr int lcdCharacterBudget = 26;

    /** Section 6.3: the live readout holds this long after the gesture ends, then the Program name
        returns. Only direct manipulation starts it - host automation must never drive it. */
    inline constexpr int lcdReadoutHoldMs = 900;

    inline constexpr float saveX = 827.0f;
    inline constexpr float saveW = 68.0f;
    inline constexpr float deleteX = 902.0f;
    inline constexpr float deleteW = 76.0f;
    inline constexpr float headerButtonY = 53.22f;
    inline constexpr float headerButtonH = 34.0f;

    inline constexpr float meterCaptionY = 34.72f;
    inline constexpr float meterBoxY = 52.72f;
    inline constexpr float meterBoxW = 76.0f;
    inline constexpr float meterBoxH = 34.0f;
    inline constexpr float meterInX = 1004.0f;
    inline constexpr float meterOutX = 1090.0f;

    // --- scope ---------------------------------------------------------------
    inline constexpr float scopeCaptionY = 134.45f;
    inline constexpr float scopeCaptionH = 14.0f;
    inline constexpr juce::Point<float> lampCentre { 80.5f, 141.45f };
    inline constexpr float lampDiameter = 13.0f;
    inline constexpr float lampLabelX = 98.0f;      // lamp right edge (87) + the 11px flex gap

    inline constexpr float scopeX = 74.0f;
    inline constexpr float scopeY = 156.45f;
    inline constexpr float scopeW = 1092.0f;
    inline constexpr float scopeH = 98.0f;

    /** The dark box splits into a readout strip over a plot area. All variable text lives in the
        strip, in Share Tech Mono - the same face as the PROGRAM LCD - and never in the plot. That
        combination of placement and typeface is what makes changing text read as a screen rather
        than a printed label. */
    inline constexpr float scopeInnerX = 75.0f;
    inline constexpr float scopeInnerY = 157.45f;
    inline constexpr float scopeInnerW = 1090.0f;
    inline constexpr float readoutStripH = 23.0f;
    inline constexpr float plotY = 180.45f;
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
    inline constexpr float rowAY = 276.45f;
    inline constexpr float rowAH = 289.0f;
    inline constexpr float legendTextSize = 10.0f;
    inline constexpr float legendTracking = 0.28f;

    inline constexpr float timingX = 74.0f;
    inline constexpr float timingW = 296.0f;
    inline constexpr float repeatsX = 386.0f;
    inline constexpr float repeatsW = 442.0f;
    inline constexpr float outputX = 844.0f;
    inline constexpr float outputW = 322.0f;   // section 4.1: a 300 box overflows by 22

    inline constexpr float syncSwitchX = 91.0f;
    inline constexpr float syncSwitchY = 297.45f;
    inline constexpr float syncSwitchW = 54.0f;
    inline constexpr float syncSwitchH = 26.0f;
    inline constexpr float syncThumbOffX = 94.0f;
    inline constexpr float syncThumbOnX = 119.0f;
    inline constexpr float syncThumbY = 300.45f;
    inline constexpr float syncThumbW = 23.0f;
    inline constexpr float syncThumbH = 20.0f;
    inline constexpr float syncThumbAnimMs = 180.0f;
    inline constexpr float syncCaptionX = 157.0f;
    inline constexpr float syncCaptionY = 303.45f;

    inline constexpr juce::Point<float> divisionLedCentre { 94.5f, 343.95f };
    inline constexpr float divisionLabelX = 105.0f;
    inline constexpr float divisionLabelY = 337.45f;
    inline constexpr float divisionButtonY = 357.45f;
    inline constexpr float divisionButtonW = 48.4f;
    inline constexpr float divisionButtonH = 32.0f;
    inline constexpr float divisionButtonPitch = 53.4f;
    inline constexpr float divisionButtonX0 = 91.0f;

    inline constexpr juce::Point<float> timeKnobCentre { 222.0f, 462.45f };
    inline constexpr float timeLabelY = 535.45f;
    // Measured off the plate's own ink, not derived from the label's box: the caption is baked now,
    // so its printed cap-height band is the only thing the lamp can be centred against. TIME's
    // glyph run spans y 539.0..546.5 at 2x, centre 542.75, and starts at x 215.5.
    //
    // X is set from the label stack, which is the panel's reference for a lamp beside a word: it
    // puts text at stackX + 13 with a 6px lamp centred at stackX + 3, and renders a 7.5px gap from
    // lamp edge to first ink. These two lamps are 7px, so centre = first ink - 7.5 - 3.5.
    inline constexpr juce::Point<float> timeLedCentre { 205.0f, 542.75f };

    inline constexpr juce::Point<float> feedbackKnobCentre { 508.0f, 375.45f };
    inline constexpr juce::Point<float> crossFeedKnobCentre { 717.0f, 375.45f };
    inline constexpr float repeatsLabelY = 457.45f;
    // Likewise measured: CROSS-FEED's baked ink spans y 450.0..457.5, centre 453.75.
    //
    // Measure a caption in an x-window that stops at the caption, not at the next control. The
    // window 660..900 looks like it isolates CROSS-FEED and does not - it also catches the "0"
    // numeral of the knob to its right, which sits ~9px lower and drags the apparent centre down to
    // 458.25. Both wrong values this constant has held came from that: 463.7 from the label box,
    // then 458.25 from a contaminated band. The check that catches it is cap height - both this
    // caption and TIME's measure exactly 7.5px, and a "caption" reading 16.5px tall is two things.
    // Caption ink starts at x 689.5, so the stack's 7.5px gap puts the centre at 679.
    inline constexpr juce::Point<float> crossFeedLedCentre { 679.0f, 453.75f };

    inline constexpr float stereoLabelX = 403.0f;
    inline constexpr float stereoLabelY = 480.45f;
    inline constexpr float stereoButtonY = 500.45f;
    inline constexpr float stereoButtonW = 132.66f;
    inline constexpr float stereoButtonH = 34.0f;
    inline constexpr std::array<float, 3> stereoButtonX { { 403.0f, 540.66f, 678.33f } };

    inline constexpr juce::Point<float> mixKnobCentre { 936.0f, 414.45f };
    inline constexpr juce::Point<float> trimKnobCentre { 1084.0f, 414.45f };
    inline constexpr float outputLabelY = 485.45f;

    // --- control row B: DELAY CHARACTER --------------------------------------
    inline constexpr float rowBY = 579.45f;
    inline constexpr float rowBH = 283.0f;

    inline constexpr float modeButtonX = 95.0f;
    inline constexpr float modeButtonW = 158.0f;
    inline constexpr float modeButtonH = 56.0f;
    inline constexpr float modeButtonPitch = 68.0f;
    inline constexpr float modeButtonY0 = 598.45f;
    inline constexpr float modeLedX = 115.0f;
    inline constexpr float modeLedDiameter = 10.0f;
    inline constexpr float modeLabelX = 132.0f;
    inline constexpr float modeLabelSize = 13.0f;

    /** DELAY CHARACTER's three flex columns.

        The right-hand one declares `width:250px` alongside `border-left:1px; padding-left:26px`,
        and with no box-sizing reset that 250 is its CONTENT width - its border box is 277. Reading
        it as the border box (which is what the first extraction did) walks the divider 27px right
        and, because the middle column is `flex:1`, stretches it by the same 27, spreading the three
        dials on a pitch 9px too wide. Both dividers are measured off the prototype at 289 and 868.  */
    inline constexpr float dialsDividerX = 275.0f;
    inline constexpr float sharedDividerX = 856.0f;
    inline constexpr float rowBDividerY = 598.45f;
    inline constexpr float rowBDividerH = 253.0f;

    /** Section 4.5: the three dials do NOT share a wrapper width. Dial 1 carries the outer Hz arc
        and needs a 232 box; dials 2 and 3 carry one ring each and stay 144. Giving all three the
        232 box costs 176 px the fascia does not have and clips both rack ears off the panel.

        They DO share a pivot Y and a label-stack Y. Dials 2 and 3 are drawn in a 144 x 232
        registration box with their 144 ring inset 44 px from its top, so a row of wrappers
        differing by 88 px in height still lands every pivot and every stack on one line - a matched
        row at three different heights reads as a build error. Uneven horizontal pitch between the
        pivots is correct and expected: the dials genuinely carry different amounts of scale. */
    inline constexpr std::array<float, 3> dialCentreX { { 415.33f, 610.0f, 760.66f } };
    inline constexpr float dialCentreY = 704.45f;
    inline constexpr float dialWrapper1 = 232.0f;      // dial 1, two rings
    inline constexpr float dialWrapper23 = 144.0f;     // dials 2 and 3, one ring
    inline constexpr float dialRingInsetY = 44.0f;     // dials 2/3 ring top inside the 232 box
    inline constexpr float dialLabelStackTop = 802.45f;
    inline constexpr float dialLabelRowPitch = 17.0f;
    inline constexpr float dialLabelSize = 10.0f;
    inline constexpr float dialLabelTracking = 0.18f;
    inline constexpr float dialLedDiameter = 6.0f;
    inline constexpr float dialLedOffsetX = 3.0f;
    inline constexpr float dialLedOffsetY = 6.25f;
    inline constexpr float dialLabelTextOffsetX = 13.0f;

    inline constexpr float sharedHeadingX = 877.0f;
    inline constexpr float sharedHeadingY = 588.45f;
    inline constexpr juce::Point<float> dampingKnobCentre { 944.0f, 680.45f };
    inline constexpr juce::Point<float> saturationKnobCentre { 1078.0f, 680.45f };
    inline constexpr float sharedLabelY = 753.45f;

    // --- foot ----------------------------------------------------------------
    inline constexpr float footRuleY = 874.45f;
    inline constexpr float footWindowX = 262.65f;
    inline constexpr float footWindowY = 883.19f;
    inline constexpr float footWindowH = 38.52f;
    inline constexpr float footWindowTextX = 275.0f;
    inline constexpr float footWindowTextY = 893.0f;
    inline constexpr float footSpecX = 74.0f;
    inline constexpr float footTextY = 895.95f;
    inline constexpr float footSerialRight = 1166.0f;   // right-aligned: 1075.5 + 90.5

    /** Section 9. The foot used to read "BYPASS · v…", borrowed from Chorus-60 where the footer is a
        live readout that flips between ENGAGED and BYPASS. Fifth Member has no bypass parameter and
        no disengaged state, so the word permanently announced a state that does not exist on a unit
        passing audio. It is a static serial now. **Do not add a bypass to justify a caption** - if
        one is ever added it gets BRAND.md's lighting treatment, not a printed word. */
    inline constexpr const char* footSerial = "SN 0417";

    // --- printed scales ------------------------------------------------------
    //
    // A tick is drawn at every printed numeral AND NOWHERE ELSE, and minors fall on real values too
    // (section 4.2a). What this replaced was a fixed-pitch decorative ring: a full 360 degrees at
    // 27 degree pitch, unrelated to the 270 the pointer sweeps, which put marks below the horizontal
    // at both ends of the arc, left a 36 degree seam sitting under the pointer at centre value on
    // the 62 and 66 px knobs, and printed a mark at +135 with no twin at -135.
    //
    // ANGLES ARE NOT STORED HERE. They are computed from the bound parameter's own
    // NormalisableRange at draw time - the same call the pointer uses - so a ring cannot drift from
    // the taper it legends, and changing a range moves its scale with it. Three of these are skewed
    // (TIME, MOD RATE, DAMPING) and their marks are therefore visibly uneven, which is correct.

    struct ScaleMark
    {
        float value;
        const char* printed;   // nullptr = a minor tick with no numeral
        bool major;
    };

    struct KnobScale
    {
        const ScaleMark* marks;
        int count;

        /** The range this ring legends, when that is NOT the knob's currently bound parameter.

            Dial 1 needs it and nothing else does. It carries two rings - a percentage and a
            frequency - but only one parameter is bound at a time, so asking the Slider to map both
            puts every percent mark through the Hz range: 25, 50, 75 and 100 all clamp to 1.0 and
            stack on top of each other at +135 degrees. Leave lo == hi and the ring maps through the
            bound parameter instead, which is right for every single-ring knob and keeps their marks
            tied to the taper they actually legend. */
        float lo = 0.0f, hi = 0.0f, skew = 1.0f;


        /** True once the plate bakes this ring, at which point the build must stop drawing it or it
            double-prints at a one-pixel offset. False for every ring today: the plate specified in
            build-handoff section 1 has not been delivered, so the build draws all of them. Dial 1's
            two rings stay false permanently - they light and dim per mode, and baked pixels cannot
            change ink. */
        bool bakedInPlate = false;
    };

    inline constexpr ScaleMark timeMarks[] {
        {    1.0f, "1",    true }, {    5.0f, nullptr, false }, {   10.0f, "10",  true },
        {   20.0f, nullptr, false }, {   50.0f, nullptr, false }, {  100.0f, "100", true },
        {  200.0f, nullptr, false }, {  375.0f, "375", true }, {  500.0f, nullptr, false },
        {  750.0f, nullptr, false }, { 1000.0f, "1K",  true }, { 1500.0f, nullptr, false },
        { 2000.0f, "2K",   true } };

    inline constexpr ScaleMark dampingMarks[] {
        {  1000.0f, "1K",  true }, {  2000.0f, "2K", true }, {  3000.0f, nullptr, false },
        {  4000.0f, "4K",  true }, {  6000.0f, nullptr, false }, {  8000.0f, "8K", true },
        { 12000.0f, nullptr, false }, { 16000.0f, "16K", true } };

    /** 0 / 50 / 100 labelled, 25 and 75 as minors - used where the body is too small to carry five
        numerals without them touching (SATURATION, CROSS-FEED, section 4.4). */
    inline constexpr ScaleMark percentSparseMarks[] {
        { 0.0f, "0", true }, { 25.0f, nullptr, false }, { 50.0f, "50", true },
        { 75.0f, nullptr, false }, { 100.0f, "100", true } };

    /** All five labelled - MIX, and the three Delay Character dials' percent rings. */
    inline constexpr ScaleMark percentFullMarks[] {
        { 0.0f, "0", true }, { 25.0f, "25", true }, { 50.0f, "50", true },
        { 75.0f, "75", true }, { 100.0f, "100", true } };

    /** Feedback runs to 110 %, which is what makes it self-oscillate. The end stop is a minor with
        no numeral: it is a real value the knob reaches, but printing "110" next to "100" on an 84 px
        body crowds both. */
    inline constexpr ScaleMark feedbackMarks[] {
        {   0.0f, "0",   true }, {  20.0f, "20",  true }, {  40.0f, "40",  true },
        {  60.0f, "60",  true }, {  80.0f, "80",  true }, { 100.0f, "100", true },
        { 110.0f, nullptr, false } };

    /** OUTPUT TRIM is recut against the BUILD's range, -24..+24 dB, not the handoff section 4.4
        table's -24..+12. Printing that table would put 0 dB at +45 degrees where the pointer reaches
        it at 0 - every numeral on the ring would name a rotation the knob does not have. Raised with
        the designers; the parameter is unchanged.
     
        Labels are explicit strings so "+12" and "+24" keep their leading plus: it is a bipolar dB
        scale, the sign is the convention, and the LCD formats the same parameter as "+2.5 dB". */
    inline constexpr ScaleMark trimMarks[] {
        { -24.0f, "-24", true }, { -18.0f, nullptr, false }, { -12.0f, "-12", true },
        {  -6.0f, nullptr, false }, {   0.0f, "0",  true }, {   6.0f, nullptr, false },
        {  12.0f, "+12", true }, {  18.0f, nullptr, false }, {  24.0f, "+24", true } };

    /** Dial 1's outer arc in BBD - Mod Rate, skewed, so these are visibly uneven (section 4.5). */
    inline constexpr ScaleMark modRateMarks[] {
        { 0.1f, "0.1", true }, { 0.25f, nullptr, false }, { 0.5f, "0.5", true },
        { 1.0f, "1",   true }, { 1.5f, nullptr, false }, { 2.0f, "2",   true },
        { 3.0f, "3",   true }, { 4.0f, nullptr, false }, { 5.0f, "5",   true } };

    template <int N>
    inline constexpr KnobScale scaleOf (const ScaleMark (&m)[N]) { return { m, N, 0.0f, 0.0f, 1.0f, false }; }

    /** Marks this ring as carried by the plate, so the build stops drawing it. Everything except
        dial 1's two rings is baked; those two change ink per mode and baked pixels cannot. */
    inline constexpr KnobScale baked (KnobScale s) { s.bakedInPlate = true; return s; }

    /** For a ring whose range is not the bound parameter's - see KnobScale::lo. */
    template <int N>
    inline constexpr KnobScale scaleOf (const ScaleMark (&m)[N], float lo, float hi, float skew)
    { return { m, N, lo, hi, skew, false }; }

    /** Section 4.2a geometry. Both rings end on a common outer tip so the eye reads one arc. */
    inline constexpr float tickMajorInner = 8.0f, tickMajorOuter = 17.0f, tickMajorWidth = 2.0f;
    inline constexpr float tickMinorInner = 12.0f, tickMinorOuter = 17.0f, tickMinorWidth = 1.5f;
    inline constexpr float numeralClearance = 21.0f;   // inner edge of every numeral box
    inline constexpr float numeralHalfCap = 5.0f;
    inline constexpr float numeralSize = 10.0f, numeralTracking = 0.06f;

    /** Dial 1's outer Hz arc. It clears the INNER ring's numerals by their outer extent, not by
        their inner edge - numerals are anchored by the edge facing the dial, so a three-character
        label reaches much further out than its anchor suggests. At R=38 the widest inner label,
        "100", reaches radius 77.3, so the outer ticks cannot begin before ~R+43; they begin at R+44
        (radius 82), clearing by 4.7 px. An earlier revision placed them at R+30 and drove a
        7.7 x 6.9 px collision between "100" and an outer tick. */
    inline constexpr float outerTickInner = 44.0f, outerTickOuter = 53.0f;
    inline constexpr float outerNumeralClearance = 57.0f;

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
        float tickMaskInner;    // the design's mask stops, as fractions of the ring box's
        float tickMaskOuter;    // FARTHEST-CORNER radius - not of the box radius
        float tickStepDegrees;
        float tickWidth;
        juce::Colour faceTop, faceMid, faceBottom;
        juce::Colour tickColour;
        float slewMs;           // the character dials re-set more slowly than the rest
    };

    inline const KnobVariant knob62 { 62.0f, 2.0f, 24.0f, 6.0f, -10.0f, 0.62f, 0.74f, 27.0f, 1.4f,
                                      juce::Colour (0xFF4C4942), juce::Colour (0xFF262420),
                                      juce::Colour (0xFF121110), Colour::scaleTick, 620.0f };
    inline const KnobVariant knob66 { 66.0f, 2.0f, 26.0f, 6.0f, -10.0f, 0.62f, 0.74f, 27.0f, 1.4f,
                                      juce::Colour (0xFF4C4942), juce::Colour (0xFF262420),
                                      juce::Colour (0xFF121110), Colour::scaleTick, 620.0f };
    inline const KnobVariant knob76 { 76.0f, 2.5f, 30.0f, 7.0f, -11.0f, 0.64f, 0.76f, 24.0f, 1.3f,
                                      juce::Colour (0xFF514E46), juce::Colour (0xFF282621),
                                      juce::Colour (0xFF121110), Colour::scaleTick, 660.0f };
    inline const KnobVariant knob82 { 82.0f, 3.0f, 32.0f, 7.0f, -11.0f, 0.64f, 0.76f, 22.5f, 1.2f,
                                      juce::Colour (0xFF55524A), juce::Colour (0xFF2A2823),
                                      juce::Colour (0xFF131210), Colour::scaleTick, 620.0f };
    inline const KnobVariant knob84 { 84.0f, 3.0f, 33.0f, 7.0f, -11.0f, 0.64f, 0.76f, 22.5f, 1.2f,
                                      juce::Colour (0xFF55524A), juce::Colour (0xFF2A2823),
                                      juce::Colour (0xFF131210), Colour::scaleTick, 620.0f };

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

    struct TickAnnulus { float inner, outer; };

    /** Where the tick marks actually start and stop.

        The design draws them as a conic gradient filling a box inset -10/-11px from the body, then
        masks it with `radial-gradient(circle, transparent, #000 <inner>% <outer>%, transparent)`.
        The two opaque stops are what matter, and they are percentages of that box's
        FARTHEST-CORNER radius - `ringHalf * sqrt(2)`, not `ringHalf`. Read as fractions of the box
        radius instead, the marks come out a third too short and stop well inside where the design
        puts them, which reads as a spoke on the body rather than a mark clear of it. */
    inline TickAnnulus tickAnnulus (const Layout::KnobVariant& v) noexcept
    {
        const float ringHalf = v.diameter * 0.5f - v.tickInset;   // tickInset is negative: outset
        const float farthestCorner = ringHalf * juce::MathConstants<float>::sqrt2;
        return { farthestCorner * v.tickMaskInner, farthestCorner * v.tickMaskOuter };
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
