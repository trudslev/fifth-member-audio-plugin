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
        case Region::deleteOrCancel: return ! displayedIsFactory;
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

    const int index = manager.getCurrentProgram();
    const auto name = manager.getProgramName (index);
    const bool factory = ProgramManager::isFactoryProgram (index);
    const bool modified = manager.isModifiedFromLoadedProgram();

    if (index == displayedIndex && name == displayedName
        && factory == displayedIsFactory && modified == displayedIsModified)
        return false;

    displayedIndex = index;
    displayedName = name;
    displayedIsFactory = factory;
    displayedIsModified = modified;
    return true;
}

void ProgramHeader::timerCallback()
{
    bool needsRepaint = refreshFromProcessor();

    // The live readout reverts on its own clock rather than a second timer (section 6.3).
    if (readoutRevertAtMs != 0 && juce::Time::getMillisecondCounter() >= readoutRevertAtMs)
    {
        readoutRevertAtMs = 0;
        liveReadout.clear();
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
            if (! displayedIsFactory)
                processorRef.getProgramManager().deleteUserProgram (displayedIndex);
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

    const int current = manager.getCurrentProgram();
    const int total = manager.getNumPrograms();

    const auto rowFor = [&manager] (int i)
    {
        return juce::String (i + 1).paddedLeft ('0', 2) + " " + manager.getProgramName (i);
    };

    menu.addSectionHeader ("Factory");

    for (int i = 0; i < kNumFactoryPrograms; ++i)
        menu.addItem (i + 1, rowFor (i), true, i == current);

    if (total > kNumFactoryPrograms)
    {
        menu.addSeparator();
        menu.addSectionHeader ("User");

        for (int i = kNumFactoryPrograms; i < total; ++i)
            menu.addItem (i + 1, rowFor (i), true, i == current);
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

                            if (result != 0)
                                safeThis->processorRef.setCurrentProgram (result - 1);
                        });
}

void ProgramHeader::enterNamingMode()
{
    liveReadout.clear();
    readoutRevertAtMs = 0;

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
void ProgramHeader::paintButton (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& text,
                                 bool enabled, bool isHovered, bool isDelete)
{
    const auto top = isDelete ? Colour::deleteTop : (isHovered && enabled ? Colour::saveTopHover : Colour::saveTop);
    const auto bottom = isDelete ? Colour::deleteBottom : (isHovered && enabled ? Colour::saveBotHover : Colour::saveBottom);

    g.setGradientFill (Paint::vertical (area, top, bottom));
    g.fillRoundedRectangle (area, 3.0f);

    g.setColour (juce::Colour (0xFF0A0908));
    g.drawRoundedRectangle (area, 3.0f, 1.0f);

    g.setColour (juce::Colours::white.withAlpha (isDelete ? 0.10f : 0.14f));
    g.drawLine (area.getX() + 3.0f, area.getY() + 1.5f, area.getRight() - 3.0f, area.getY() + 1.5f, 1.0f);

    Text::drawTracked (g, text, Font::label (12.0f), Font::trackingPx (0.18f, 12.0f),
                       area, juce::Justification::centred,
                       enabled ? Colour::buttonText : Colour::labelDisabled);
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
    if (liveReadout.isNotEmpty())
        return liveReadout;

    // A trailing " *" while the loaded Program has been edited, matching every other casting. It
    // clears on store, on delete and on loading another Program - all three reset displayedIsModified
    // through refreshFromProcessor, so nothing extra is needed here. Worst case is the 26-character
    // name cap plus "NN " and the marker, 31, which is inside the 16px guard's own 31.
    return juce::String (displayedIndex + 1).paddedLeft ('0', 2) + " " + displayedName
         + (displayedIsModified ? " *" : "");
}

void ProgramHeader::showParameter (const juce::RangedAudioParameter& param)
{
    if (namingMode)
        return;   // the glass belongs to the name field until it commits or cancels

    // Straight through the parameter's own name and JUCE's own text conversion, so the LCD and the
    // host never disagree about what a control reads. Section 6.3's examples set the shape:
    // "FEEDBACK: 62 %", "TIME: 375 ms", "OUTPUT TRIM: +2.5 dB".
    const auto name = param.getName (Layout::lcdCharacterBudget).toUpperCase();
    // Name, value, unit - section 6.3's "FEEDBACK: 62 %", "TIME: 375 ms". The label is joined here
    // rather than baked into the value text so JUCE's own generic editor does not double it.
    const auto unit = param.getLabel();
    const auto text = name + ": " + param.getCurrentValueAsText()
                    + (unit.isEmpty() ? juce::String() : " " + unit);

    if (text != liveReadout)
    {
        liveReadout = text;
        repaint();
    }

    readoutRevertAtMs = 0;
}

void ProgramHeader::releaseParameter()
{
    if (liveReadout.isNotEmpty())
        readoutRevertAtMs = juce::Time::getMillisecondCounter()
                                + (juce::uint32) Layout::lcdReadoutHoldMs;
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
        const bool showUser = namingMode || ! displayedIsFactory;

        const juce::Rectangle<float> tag { display.getX(), display.getY(), Layout::bankTagW, display.getHeight() };

        // Section 6.2: identical to the program name in face, size, tracking and colour. It sits
        // inside a display, so it is display text - it is no longer set smaller and dimmer.
        Text::drawTracked (g, showUser ? "USER" : "FACT", Font::mono (Layout::lcdTextSize),
                            Font::trackingPx (Layout::lcdTracking, Layout::lcdTextSize),
                            tag, juce::Justification::centred, Colour::lcdText);

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

    paintButton (g, saveArea(), namingMode ? "STORE" : "SAVE",
                 isEnabled (Region::save), hovered == Region::save, false);
    paintButton (g, deleteArea(), namingMode ? "CANCEL" : "DELETE",
                 isEnabled (Region::deleteOrCancel), hovered == Region::deleteOrCancel, true);

    paintMeters (g);
}
