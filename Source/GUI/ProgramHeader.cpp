#include "ProgramHeader.h"

#include "../PluginProcessor.h"

using namespace FifthMemberTheme;

namespace
{
    bool caretIsOn()
    {
        return (juce::Time::getMillisecondCounter() % 1000) < 500;
    }

    juce::String formatMeter (float db)
    {
        return db <= -99.0f ? juce::String ("-99.0") : juce::String (db, 1);
    }
}

//==============================================================================
juce::Rectangle<float> ProgramHeader::displayArea()
{
    return { Layout::lcdX, Layout::lcdY, Layout::lcdW, Layout::lcdH };
}

juce::Rectangle<float> ProgramHeader::saveArea()
{
    return { Layout::saveX, Layout::headerButtonY, Layout::saveW, Layout::headerButtonH };
}

juce::Rectangle<float> ProgramHeader::deleteArea()
{
    return { Layout::deleteX, Layout::headerButtonY, Layout::deleteW, Layout::headerButtonH };
}

//==============================================================================
ProgramHeader::ProgramHeader (FifthMemberAudioProcessor& processor)
    : processorRef (processor)
{
    setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setWantsKeyboardFocus (true);
    refreshFromProcessor();
    startTimerHz (20);
}

ProgramHeader::~ProgramHeader()
{
    stopTimer();
}

ProgramHeader::Region ProgramHeader::regionAt (juce::Point<float> p) const
{
    if (displayArea().contains (p)) return Region::display;
    if (saveArea().contains (p))    return Region::save;
    if (deleteArea().contains (p))  return Region::deleteOrCancel;

    return Region::none;
}

bool ProgramHeader::hitTest (int x, int y)
{
    return regionAt ({ (float) x, (float) y }) != Region::none;
}

bool ProgramHeader::isEnabled (Region region) const
{
    if (namingMode)
        return region == Region::save || region == Region::deleteOrCancel;

    switch (region)
    {
        // Not while naming: opening the list then would apply a Program underneath a
        // half-typed name, leaving a stale field over a Program that never had it. TapeRot has
        // this bug; it is not worth replicating.
        case Region::display:        return ! namingMode;
        case Region::save:           return displayedIsModified;
        // Only a User Program can be deleted. INIT and an unresolved id are not stored things.
        case Region::deleteOrCancel: return displayedId.bank == ProgramBank::user;
        case Region::none:           break;
    }

    return false;
}

//==============================================================================
bool ProgramHeader::refreshFromProcessor()
{
    // Never written while naming, which is what makes cancelling free: leaving the mode reverts the
    // display with nothing to undo.
    if (namingMode)
        return false;

    auto& manager = processorRef.getProgramManager();

    const auto id = manager.getCurrentProgramId();
    const bool modified = manager.isModifiedFromLoadedProgram();

    if (id == displayedId && modified == displayedIsModified)
        return false;

    displayedId = id;
    displayedIsModified = modified;
    return true;
}

void ProgramHeader::timerCallback()
{
    bool needsRepaint = refreshFromProcessor();

    // The live readout reverts on its own clock rather than a second timer (section 6.3). The
    // deadline is core's; polling it from a timer this component already runs is the local choice,
    // and the two castings that use a one-shot juce::Timer instead read the same deadline.
    if (const bool showing = readout.isShowing (juce::Time::getMillisecondCounter());
        showing != readoutWasShowing)
    {
        readoutWasShowing = showing;
        needsRepaint = true;
    }

    if (namingMode)
        if (const bool on = caretIsOn(); on != caretVisible)
        {
            caretVisible = on;
            needsRepaint = true;
        }

    // The meters move continuously, so this component repaints on their account regardless.
    repaint (displayArea().getUnion (juce::Rectangle<float> {
                 Layout::meterInX, Layout::meterCaptionY,
                 Layout::meterOutX + Layout::meterBoxW - Layout::meterInX,
                 Layout::meterBoxY + Layout::meterBoxH - Layout::meterCaptionY })
                 .getSmallestIntegerContainer());

    if (needsRepaint)
        repaint();
}

//==============================================================================
void ProgramHeader::mouseDown (const juce::MouseEvent& e)
{
    const auto region = regionAt (e.position);

    if (! isEnabled (region))
        return;

    if (namingMode)
    {
        if (region == Region::save)                  commitNaming();
        else if (region == Region::deleteOrCancel)   cancelNaming();
        return;
    }

    switch (region)
    {
        case Region::display: showProgramMenu(); break;
        case Region::save:    enterNamingMode(); break;

        case Region::deleteOrCancel:
            if (displayedId.bank == ProgramBank::user)
                processorRef.getProgramManager().deleteUserProgram (displayedId);
            break;

        case Region::none: break;
    }
}

void ProgramHeader::mouseMove (const juce::MouseEvent& e)
{
    const auto region = regionAt (e.position);

    setMouseCursor (isEnabled (region) ? juce::MouseCursor::PointingHandCursor
                                       : juce::MouseCursor::NormalCursor);

    if (region != hovered)
    {
        hovered = region;
        repaint();
    }
}

void ProgramHeader::mouseExit (const juce::MouseEvent&)
{
    if (hovered != Region::none)
    {
        hovered = Region::none;
        repaint();
    }
}

//==============================================================================
void ProgramHeader::showProgramMenu()
{
    auto& manager = processorRef.getProgramManager();

    juce::PopupMenu menu;
    menu.setLookAndFeel (&menuLookAndFeel);

    const auto current = manager.getCurrentProgramId();

    // **Row IDs are positions in THIS menu, not Program indices.** PopupMenu needs an int per row
    // and reserves 0 for "dismissed"; the callback maps the row back to the ProgramId it was built
    // from, so no Program is addressed by a bank position here.
    menuRows = manager.listPrograms();

    bool factoryHeaderDone = false;
    bool userHeaderDone = false;

    for (size_t i = 0; i < menuRows.size(); ++i)
    {
        const auto& id = menuRows[i];

        if (id.bank == ProgramBank::factory && ! std::exchange (factoryHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader ("Factory");
        }

        if (id.bank == ProgramBank::user && ! std::exchange (userHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader ("User");
        }

        menu.addItem ((int) i + 1, manager.displayLabelFor (id), true, id == current);
    }

    /*  **The USER section is always shown, with a placeholder when the bank is empty.** An absent
        section is ambiguous between "nothing saved yet" and "this plugin does not do that", and the
        player cannot tell which without saving something to find out. Reflect-84 had it first.

        Added disabled, so this is the one row in the menu that takes drawPopupMenuItem's inactive
        path - which is why that path had to clear the 3:1 state floor before this could ship. */
    if (! userHeaderDone)
    {
        menu.addSeparator();
        menu.addSectionHeader ("User");
        menu.addItem (-1, Text::emDash() + " none saved " + Text::emDash(), false, false);
    }

    const juce::Component::SafePointer<ProgramHeader> safeThis { this };

    // The list hangs off the LCD and has to read as an extension of it, so it takes the glass's
    // width rather than sizing itself to the longest Program name - section 6.5 gives it the "full
    // LCD border-box width". localAreaToGlobal already carries the editor's scale transform, so this
    // stays right at every window size. withMaximumNumColumns(1) because JUCE otherwise wraps a long
    // bank into columns and the list stops matching the bar it drops from.
    const auto glassOnScreen = localAreaToGlobal (displayArea().getSmallestIntegerContainer());

    auto options = juce::PopupMenu::Options()
                       .withTargetComponent (this)
                       .withTargetScreenArea (glassOnScreen)
                       .withMaximumNumColumns (1);

    if (menuParent != nullptr)
    {
        // Anchor to a 1px strip on the glass's bottom EDGE, not to the glass itself.
        //
        // JUCE opens a menu at targetArea.getBottom(), but when a parent component is given it
        // first does `target = pc->getLocalArea (nullptr, target).constrainedWithin (parentArea)`.
        // The LCD sits above menuHost, so constrainedWithin slides the whole 34px glass down into
        // the host before measuring - and the menu then opens 34px below where the glass actually
        // ends, floating clear of the bar it belongs to. A thin target on the edge has nothing to
        // slide: its bottom is the edge, wherever it is clamped.
        //
        // 1px, NOT zero: createWindow derives its whole alignment mode from
        // `! getTargetScreenArea().isEmpty()`, and a zero-height rectangle is empty by JUCE's
        // definition. That drops the list out of align-to-rectangle into the side placement used
        // for submenus, which sends it out to the right of the panel instead of under the bar.
        //
        // Keeping the glass's x and width matters too - the menu takes its left edge from the
        // target, so this is what keeps the list flush with the bar rather than merely near it.
        const auto glass = displayArea().getSmallestIntegerContainer();
        const auto anchorOnScreen =
            localAreaToGlobal (juce::Rectangle<int> { glass.getX(), menuAnchorY() - 1, glass.getWidth(), 1 });

        options = options.withTargetScreenArea (anchorOnScreen);

        // Laid out inside menuHost, whose top edge is the LCD's bottom and whose bottom is the
        // panel's. JUCE places a menu against its parent area and clamps it there, so the list
        // opens hard against the glass every time and scrolls rather than growing past the panel.
        //
        // Width is the glass in COMPONENT units here, not the screen rectangle above: inside a
        // parent the menu is laid out in that parent's coordinate space, so feeding it screen
        // pixels would size the list by the editor's scale factor and only look right at 100%.
        options = options.withParentComponent (menuParent)
                         .withMinimumWidth ((int) std::ceil (displayArea().getWidth()));
    }
    else
    {
        options = options.withMinimumWidth (glassOnScreen.getWidth());
    }

    // Set before showing and cleared in the callback, which JUCE also runs on a dismissal
    // (result 0) - so the mark cannot be left inverted by clicking away from the list.
    menuOpen = true;
    repaint();

    menu.showMenuAsync (options,
                        [safeThis] (int result)
                        {
                            if (safeThis == nullptr)
                                return;

                            safeThis->menuOpen = false;
                            safeThis->repaint();

                            if (result == 0)
                                return;

                            const auto row = (size_t) (result - 1);

                            if (row < safeThis->menuRows.size())
                                safeThis->processorRef.getProgramManager()
                                    .requestProgramChange (safeThis->menuRows[row]);
                        });
}

void ProgramHeader::enterNamingMode()
{
    // The glass belongs to the name field until it commits or cancels, so a knob moved just before
    // SAVE must not reappear over a half-typed name. suppress() cancels rather than hides.
    readout.suppress();
    readoutWasShowing = false;

    namingMode = true;
    typedName.clear();
    caretVisible = true;
    grabKeyboardFocus();
    repaint();
}

void ProgramHeader::commitNaming()
{
    const auto name = typedName;

    namingMode = false;
    typedName.clear();
    giveAwayKeyboardFocus();

    processorRef.getProgramManager().saveNewUserProgram (name);

    refreshFromProcessor();
    repaint();
}

void ProgramHeader::cancelNaming()
{
    // Must NOT touch the APVTS - the tweaked-but-unsaved values survive a cancel. The displayed
    // mirrors were never written while naming, so leaving the mode is all the revert needed.
    namingMode = false;
    typedName.clear();
    giveAwayKeyboardFocus();
    repaint();
}

bool ProgramHeader::keyPressed (const juce::KeyPress& key)
{
    if (! namingMode)
        return false;

    if (key == juce::KeyPress::returnKey) { commitNaming(); return true; }
    if (key == juce::KeyPress::escapeKey) { cancelNaming(); return true; }

    if (key == juce::KeyPress::backspaceKey)
    {
        typedName = typedName.dropLastCharacters (1);
        repaint();
        return true;
    }

    const auto character = key.getTextCharacter();

    if (character >= 32 && character != 127 && typedName.length() < ProgramManager::maxProgramNameLength)
    {
        typedName += juce::String::charToString (character).toUpperCase();
        repaint();
    }

    return true;   // swallow everything else while naming
}

void ProgramHeader::focusLost (FocusChangeType)
{
    if (namingMode)
        cancelNaming();
}

//==============================================================================
void ProgramHeader::paintButton (juce::Graphics& g, juce::Rectangle<float> area,
                                 const juce::String& topLegend, const juce::String& bottomLegend,
                                 bool topLit, bool bottomLit, bool isHovered, bool isDelete)
{
    /*  **One face each. No disabled face, no relabelling** (BUILD-HANDOFF §1.3/§1.3.1). The face
        varies only on hover, which is an affordance rather than a state; nothing here branches on
        enablement. The `labelDisabled` treatment this replaced measured 1.78-1.94:1 - absent
        rather than dim - and it was answering the wrong question anyway: a button with nothing to
        do is not a button that has been switched off. */
    const auto top = isDelete ? Colour::deleteTop : (isHovered ? Colour::saveTopHover : Colour::saveTop);
    const auto bottom = isDelete ? Colour::deleteBottom : (isHovered ? Colour::saveBotHover : Colour::saveBottom);

    g.setGradientFill (Paint::vertical (area, top, bottom));
    g.fillRoundedRectangle (area, 3.0f);

    g.setColour (juce::Colour (0xFF0A0908));
    g.drawRoundedRectangle (area, 3.0f, 1.0f);

    g.setColour (juce::Colours::white.withAlpha (isDelete ? 0.10f : 0.14f));
    g.drawLine (area.getX() + 3.0f, area.getY() + 1.5f, area.getRight() - 3.0f, area.getY() + 1.5f, 1.0f);

    /*  Two legends, column, centred, 2px gap, both Barlow Condensed 600 at 10px / .14em with
        line-height 1. **Both are set at the same size and weight in both states**: 10px is
        BRAND.md's floor for functional text and both legends are functional, so neither is set
        smaller to make the pair fit, and neither is emboldened to stand in for illumination.

        The change between states is luminance plus bloom, which reads as a lamp. A bolder or
        blacker legend would read as emphasis - a different thing, and the one this construction
        exists to avoid. */
    const auto font = Font::label (Layout::programLegendTextSize);
    const float tracking = Font::trackingPx (Layout::programLegendTracking, Layout::programLegendTextSize);
    const float lineH = Layout::programLegendTextSize;
    const float blockH = lineH * 2.0f + Layout::programLegendGap;
    const float blockTop = area.getCentreY() - blockH * 0.5f;

    const auto legend = [&] (const juce::String& text, float y, bool lit)
    {
        const juce::Rectangle<float> line { area.getX(), y, area.getWidth(), lineH };

        if (lit)
        {
            /*  §1.3.1's `0 0 7px rgba(242,236,224,.55), 0 0 15px rgba(242,236,224,.25)`. JUCE has
                no text-shadow and no cheap blur for a string, so each radius is the same tracked
                text drawn at eight points around a circle; overlapping copies sum to a halo.
                Alphas are tuned rather than quoted - eight copies at alpha a reach 1-(1-a)^8 where
                they coincide, so the CSS figures cannot be used directly. */
            for (auto [radius, alpha] : { std::pair { 7.5f, 0.030f }, std::pair { 3.5f, 0.075f } })
                for (int i = 0; i < 8; ++i)
                {
                    const float angle = juce::MathConstants<float>::twoPi * (float) i / 8.0f;

                    Text::drawTracked (g, text, font, tracking,
                                       line.translated (std::cos (angle) * radius,
                                                        std::sin (angle) * radius),
                                       juce::Justification::centred,
                                       Colour::legendLit.withAlpha (alpha));
                }
        }

        Text::drawTracked (g, text, font, tracking, line, juce::Justification::centred,
                           lit ? Colour::legendLit : Colour::legendDark);
    };

    const juce::Graphics::ScopedSaveState state (g);
    g.reduceClipRegion (area.getSmallestIntegerContainer());

    legend (topLegend, blockTop, topLit);
    legend (bottomLegend, blockTop + lineH + Layout::programLegendGap, bottomLit);
}

void ProgramHeader::paintMeters (juce::Graphics& g)
{
    const auto draw = [&g] (float x, const juce::String& text)
    {
        const juce::Rectangle<float> box { x, Layout::meterBoxY, Layout::meterBoxW, Layout::meterBoxH };

        g.setColour (Colour::meterBg);
        g.fillRoundedRectangle (box, 2.0f);

        {
            juce::ColourGradient recess { juce::Colours::black.withAlpha (0.9f), box.getCentreX(), box.getY(),
                                          juce::Colours::transparentBlack, box.getCentreX(), box.getY() + 9.0f, false };
            g.setGradientFill (recess);
            g.fillRoundedRectangle (box, 2.0f);
        }

        g.setColour (Colour::lcdBorder);
        g.drawRoundedRectangle (box, 2.0f, 1.0f);

        g.setFont (Font::mono (16.0f));
        g.setColour (Colour::meterText);
        g.drawText (text, box, juce::Justification::centred, false);
    };

    draw (Layout::meterInX, formatMeter (processorRef.getInputMeterDb()));
    draw (Layout::meterOutX, formatMeter (processorRef.getOutputMeterDb()));
}

//==============================================================================
juce::String ProgramHeader::currentLcdString() const
{
    if (const auto takeover = readout.textAt (juce::Time::getMillisecondCounter());
        takeover.isNotEmpty())
        return takeover;

    // A trailing " *" while the loaded Program has been edited, matching every other casting. It
    // clears on store, on delete and on loading another Program - all three reset displayedIsModified
    // through refreshFromProcessor, so nothing extra is needed here. Worst case is the 26-character
    // name cap plus "NN " and the marker, 31, which is inside the 16px guard's own 31.
    // An identifier the session named but the bank no longer has: the VALUES are correct and
    // untouched, only the name is unknown, so the panel says so. No dirty asterisk either - there
    // is no baseline to differ from.
    if (displayedId.bank == ProgramBank::unresolved)
        return displayedId.displayName + "?";

    // Otherwise the number is a label computed from the Factory position at paint time. INIT and
    // User Programs carry none.
    return processorRef.getProgramManager().displayLabelFor (displayedId)
         + (displayedIsModified ? " *" : "");
}

void ProgramHeader::showParameter (const juce::RangedAudioParameter& param)
{
    if (namingMode)
        return;   // the glass belongs to the name field until it commits or cancels

    // **Straight through nf::describeParameter**, which is straight through the parameter's own
    // name and JUCE's own text conversion - so the LCD and the host cannot disagree about what a
    // control reads. Section 6.3's examples set the shape and are unchanged: "FEEDBACK: 62 %",
    // "TIME: 375 ms", "OUTPUT TRIM: +2.5 dB". The label is joined by core rather than baked into
    // the value text, so JUCE's own generic editor does not double it.
    const auto text = nf::describeParameter (param, readoutFormat());
    const auto now = juce::Time::getMillisecondCounter();

    // Repaint only on a CHANGE. This fires on every value change through a drag, which is what
    // keeps the readout live - Reflect-84 wired only onDragStart and its LCD froze at the value the
    // knob held when it was grabbed.
    if (text != readout.textAt (now))
        repaint();

    readout.show (text);
    readoutWasShowing = true;
}

void ProgramHeader::releaseParameter()
{
    readout.release (juce::Time::getMillisecondCounter());
}

void ProgramHeader::paint (juce::Graphics& g)
{
    const auto display = displayArea();

    Paint::drawLcdWell (g, display);

    {
        juce::ColourGradient recess { juce::Colours::black.withAlpha (0.9f), display.getCentreX(), display.getY(),
                                      juce::Colours::transparentBlack, display.getCentreX(), display.getY() + 12.0f, false };
        g.setGradientFill (recess);
        g.fillRoundedRectangle (display, Layout::lcdRadius);
    }

    // --- bank tag: a single dynamic value, FACT or USER. Never both with one greyed. ----------
    {
        // **On INIT the tag reads an em-dash at 42% ink, not FACT and not USER.** INIT sits
        // outside both banks, so either word would name a bank it is not in.
        const bool onInit = ! namingMode && (displayedId.bank == ProgramBank::init
                                              || displayedId.bank == ProgramBank::unresolved);
        /*  **NAME while typing, not USER.** The Program is not in the user bank until STORE
            commits it, and if the user cancels it never will be - so USER there names a thing that
            does not exist yet. Elmer had this right first; it is the suite standard now. */
        const auto tagText = namingMode ? juce::String ("NAME")
                           : onInit     ? Text::emDash()
                           : juce::String (displayedId.bank == ProgramBank::user ? "USER" : "FACT");

        const juce::Rectangle<float> tag { display.getX(), display.getY(), Layout::bankTagW, display.getHeight() };

        // Section 6.2: identical to the program name in face, size, tracking and colour. It sits
        // inside a display, so it is display text - it is no longer set smaller and dimmer.
        Text::drawTracked (g, tagText, Font::mono (Layout::lcdTextSize),
                            Font::trackingPx (Layout::lcdTracking, Layout::lcdTextSize),
                            tag, juce::Justification::centred,
                            onInit ? Colour::lcdText.withAlpha (0.42f) : Colour::lcdText);

        g.setColour (juce::Colours::white.withAlpha (0.09f));
        g.fillRect (tag.getRight(), display.getY() + 4.0f, 1.0f, display.getHeight() - 8.0f);
    }

    // --- the name, or the naming field ---------------------------------------
    {
        const auto nameArea = juce::Rectangle<float> (Layout::lcdNameCellX, display.getY(),
                                                       Layout::lcdNameCellW, display.getHeight());

        // Section 6.2's guard: 19px holds 26 characters in this cell at 12.54 px/char. Nothing
        // authored reaches that - the longest live readout, "GENERATION LOSS: 100 %", is 22 - but a
        // user program name can, so anything longer steps to 16px rather than overrunning the cell.
        // The step is instantaneous; there is no animation between the two sizes.
        const juce::String longest = namingMode ? typedName : currentLcdString();
        const float size = longest.length() > Layout::lcdCharacterBudget ? Layout::lcdTextSizeGuard
                                                                          : Layout::lcdTextSize;
        const auto font = Font::mono (size);
        const float tracking = Font::trackingPx (Layout::lcdTracking, size);

        const auto drawPhosphor = [&] (const juce::String& text, juce::Justification justification)
        {
            // The glow is the same text drawn soft and wide under a crisp pass.
            Text::drawTracked (g, text, font, tracking, nameArea, justification,
                               Colour::lcdText.withAlpha (0.35f));
            Text::drawTracked (g, text, font, tracking, nameArea, justification, Colour::lcdText);
        };

        if (namingMode)
        {
            const auto caret = juce::String::charToString ((juce::juce_wchar) 0x2588);
            const auto shown = typedName.isEmpty() && ! caretVisible ? juce::String ("NAME PROGRAM")
                                                                     : typedName + (caretVisible ? caret : juce::String());
            drawPhosphor (shown, juce::Justification::centred);
        }
        else
        {
            drawPhosphor (currentLcdString(), juce::Justification::centred);
        }
    }

    // --- chevron: section 6.4's drawn path -----------------------------------
    //
    // "M1 1.6 L7 6.4 L13 1.6" in a 14 x 8 viewBox, scaled to that box and stroked at 1.6 with round
    // caps and joins - built as a Path rather than a typographic glyph so it renders identically
    // across platforms and font fallbacks. It marks the window as a picker, so it is hidden while
    // naming: there is nothing to pick then.
    //
    // It inverts while the list is open, mirrored about the viewBox's centre line rather than
    // rotated, so the apex stays on the same vertical axis and the mark does not appear to shift
    // sideways as it flips. Without it the only thing saying the picker is open is the list itself,
    // and the mark still points down at a list that is already down.
    if (! namingMode)
    {
        const float left = display.getRight() - Layout::chevronPadX - Layout::chevronW;
        const float top = display.getCentreY() - Layout::chevronH * 0.5f;
        const float outer = menuOpen ? 6.4f : 1.6f;
        const float apex  = menuOpen ? 1.6f : 6.4f;

        juce::Path chevron;
        chevron.startNewSubPath (left + 1.0f,  top + outer);
        chevron.lineTo          (left + 7.0f,  top + apex);
        chevron.lineTo          (left + 13.0f, top + outer);

        g.setColour (Colour::lcdChevron);
        g.strokePath (chevron, { Layout::chevronStroke,
                                  juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    }

    /*  §1.3.1's "which legend is live" table:

        | Panel state                 | SAVE | STORE | DELETE | CANCEL |
        | Factory Program, unmodified | dark | dark  | dark   | dark   |
        | Factory Program, edited     | LIT  | dark  | dark   | dark   |
        | User Program, unmodified    | dark | dark  | LIT    | dark   |
        | User Program, edited        | LIT  | dark  | LIT    | dark   |
        | Naming a Program            | dark | LIT   | dark   | LIT    |

        SAVE's lamp and the LCD's trailing " *" read the same edited flag, so they cannot
        disagree - which is why `isEnabled(save)` is the source here rather than a second test. */
    paintButton (g, saveArea(), "SAVE", "STORE",
                 ! namingMode && isEnabled (Region::save), namingMode,
                 hovered == Region::save, false);
    paintButton (g, deleteArea(), "DELETE", "CANCEL",
                 ! namingMode && isEnabled (Region::deleteOrCancel), namingMode,
                 hovered == Region::deleteOrCancel, true);

    paintMeters (g);
}
