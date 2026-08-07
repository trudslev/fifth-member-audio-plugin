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
        case Region::display:        return true;
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
    menu.setLookAndFeel (&getLookAndFeel());

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

    menu.showMenuAsync (juce::PopupMenu::Options()
                            .withTargetComponent (this)
                            .withTargetScreenArea (localAreaToGlobal (displayArea().getSmallestIntegerContainer())),
                        [safeThis] (int result)
                        {
                            if (safeThis != nullptr && result != 0)
                                safeThis->processorRef.setCurrentProgram (result - 1);
                        });
}

void ProgramHeader::enterNamingMode()
{
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

        g.setFont (Font::mono (14.0f));
        g.setColour (Colour::lcdTextDim);
        g.drawText (showUser ? "USER" : "FACT", tag, juce::Justification::centred, false);

        g.setColour (juce::Colours::white.withAlpha (0.09f));
        g.fillRect (tag.getRight(), display.getY() + 4.0f, 1.0f, display.getHeight() - 8.0f);
    }

    // --- the name, or the naming field ---------------------------------------
    {
        const auto nameArea = display.withTrimmedLeft (Layout::bankTagW)
                                     .withTrimmedRight (Layout::caretW);

        const auto font = Font::mono (19.0f);
        const float tracking = Font::trackingPx (0.14f, 19.0f);

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
            drawPhosphor (juce::String (displayedIndex + 1).paddedLeft ('0', 2) + " " + displayedName,
                          juce::Justification::centred);
        }
    }

    // --- caret ---------------------------------------------------------------
    if (! namingMode)
    {
        const float cx = display.getRight() - 18.0f;
        const float cy = display.getCentreY() - 1.0f;

        juce::Path chevron;
        chevron.startNewSubPath (cx - 5.0f, cy - 2.0f);
        chevron.lineTo (cx, cy + 3.5f);
        chevron.lineTo (cx + 5.0f, cy - 2.0f);

        g.setColour (juce::Colour (0xFF6F7A70));
        g.strokePath (chevron, { 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::square });
    }

    paintButton (g, saveArea(), namingMode ? "STORE" : "SAVE",
                 isEnabled (Region::save), hovered == Region::save, false);
    paintButton (g, deleteArea(), namingMode ? "CANCEL" : "DELETE",
                 isEnabled (Region::deleteOrCancel), hovered == Region::deleteOrCancel, true);

    paintMeters (g);
}
