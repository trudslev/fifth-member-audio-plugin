#include "PanelControls.h"

#include "../PluginProcessor.h"

using namespace FifthMemberTheme;

namespace
{
    const std::array<const char*, 3> characterNames { { "TAPE", "BBD", "DIGITAL" } };
    const std::array<const char*, 3> stereoNames { { "MONO", "STEREO", "PING-PONG" } };
}

const std::array<std::array<PanelControls::DialLabel, 3>, 3>& PanelControls::dialLabels()
{
    // [dial][mode]. Read down a column to see one mode's three dials; read across a row to see
    // every label that dial position carries. All of them are printed on the panel at all times -
    // only the LED moves.
    static const std::array<std::array<DialLabel, 3>, 3> table { {
        { { { "WOW" },     { "MOD RATE" },  { "REPEAT DEGRADE" } } },
        { { { "FLUTTER" }, { "MOD DEPTH" }, { nullptr } } },
        { { { "GENERATION LOSS" }, { nullptr }, { nullptr } } } } };

    return table;
}

//==============================================================================
PanelControls::PanelControls (FifthMemberAudioProcessor& processor)
    : processorRef (processor)
{
    setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);
    rebuildHitAreas();

    displayedSync = boolParam (ParamIDs::sync);
    displayedDivision = intParam (ParamIDs::noteDivision);
    displayedStereo = intParam (ParamIDs::stereoMode);
    displayedCharacter = intParam (ParamIDs::character);
    thumbPosition = displayedSync ? 1.0f : 0.0f;

    lastFrameMs = juce::Time::getMillisecondCounter();
    startTimerHz (Layout::animationHz);
}

PanelControls::~PanelControls()
{
    stopTimer();
}

//==============================================================================
int PanelControls::intParam (const char* id) const
{
    if (const auto* p = dynamic_cast<const juce::AudioParameterChoice*> (processorRef.apvts.getParameter (id)))
        return p->getIndex();

    return 0;
}

bool PanelControls::boolParam (const char* id) const
{
    const auto* raw = processorRef.apvts.getRawParameterValue (id);
    return raw != nullptr && raw->load() > 0.5f;
}

void PanelControls::setChoice (const char* id, int index)
{
    if (auto* p = processorRef.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) index));
        p->endChangeGesture();
    }
}

//==============================================================================
void PanelControls::rebuildHitAreas()
{
    hits.clear();

    hits.push_back ({ { Layout::syncSwitchX, Layout::syncSwitchY,
                        Layout::syncSwitchW, Layout::syncSwitchH }, Region::syncSwitch, 0 });

    for (int i = 0; i < numNoteDivisions; ++i)
        hits.push_back ({ { Layout::divisionButtonX0 + (float) i * Layout::divisionButtonPitch,
                            Layout::divisionButtonY, Layout::divisionButtonW, Layout::divisionButtonH },
                          Region::division, i });

    for (int i = 0; i < numStereoModes; ++i)
        hits.push_back ({ { Layout::stereoButtonX[(size_t) i], Layout::stereoButtonY,
                            Layout::stereoButtonW, Layout::stereoButtonH }, Region::stereo, i });

    for (int i = 0; i < numDelayCharacters; ++i)
        hits.push_back ({ { Layout::modeButtonX, Layout::modeButtonY0 + (float) i * Layout::modeButtonPitch,
                            Layout::modeButtonW, Layout::modeButtonH }, Region::character, i });
}

const PanelControls::Hit* PanelControls::hitAt (juce::Point<float> position) const
{
    for (const auto& hit : hits)
        if (hit.area.contains (position))
            return &hit;

    return nullptr;
}

bool PanelControls::hitTest (int x, int y)
{
    // Narrowed to the union of the interactive rects. Without this the component would cover the
    // whole canvas and swallow every click meant for a knob beneath it.
    return hitAt ({ (float) x, (float) y }) != nullptr;
}

void PanelControls::mouseDown (const juce::MouseEvent& e)
{
    const auto* hit = hitAt (e.position);

    if (hit == nullptr)
        return;

    switch (hit->region)
    {
        case Region::syncSwitch:
            if (auto* p = processorRef.apvts.getParameter (ParamIDs::sync))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (boolParam (ParamIDs::sync) ? 0.0f : 1.0f);
                p->endChangeGesture();
            }
            break;

        case Region::division:  setChoice (ParamIDs::noteDivision, hit->index); break;
        case Region::stereo:    setChoice (ParamIDs::stereoMode, hit->index); break;

        case Region::character:
            if (hit->index != displayedCharacter)
            {
                setChoice (ParamIDs::character, hit->index);

                // The panel physically re-sets itself: every dial snaps to minimum and sweeps back
                // up to its new value. design/README.md calls this the moment that sells the
                // hardware premise.
                if (onCharacterChanged)
                    onCharacterChanged();
            }
            break;

        case Region::none:
            break;
    }

    repaint();
}

void PanelControls::mouseMove (const juce::MouseEvent& e)
{
    const auto* hit = hitAt (e.position);

    if (hit != hovered)
    {
        hovered = hit;
        setMouseCursor (hit != nullptr ? juce::MouseCursor::PointingHandCursor
                                       : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void PanelControls::mouseExit (const juce::MouseEvent&)
{
    if (hovered != nullptr)
    {
        hovered = nullptr;
        repaint();
    }
}

//==============================================================================
void PanelControls::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounter();
    const float dt = juce::jlimit (1.0f, 100.0f, (float) (now - lastFrameMs));
    lastFrameMs = now;

    bool changed = false;

    const auto poll = [&changed] (auto& mirror, auto value)
    {
        if (mirror != value) { mirror = value; changed = true; }
    };

    poll (displayedSync, boolParam (ParamIDs::sync));
    poll (displayedDivision, intParam (ParamIDs::noteDivision));
    poll (displayedStereo, intParam (ParamIDs::stereoMode));

    const int character = intParam (ParamIDs::character);

    if (character != displayedCharacter)
    {
        displayedCharacter = character;
        changed = true;
    }

    // Thumb travel, time-based so it takes 180 ms whatever the frame rate.
    const float target = displayedSync ? 1.0f : 0.0f;

    if (std::abs (thumbPosition - target) > 1.0e-4f)
    {
        const float step = dt / Layout::syncThumbAnimMs;
        thumbPosition = thumbPosition < target ? juce::jmin (target, thumbPosition + step)
                                               : juce::jmax (target, thumbPosition - step);
        changed = true;
    }

    if (changed)
        repaint();
}

//==============================================================================
void PanelControls::paint (juce::Graphics& g)
{
    paintSyncSwitch (g, displayedSync);
    paintDivisionRow (g, displayedSync, displayedDivision);
    paintStereoRow (g, displayedStereo);
    paintCharacterColumn (g, displayedCharacter);
    paintDialLabels (g, displayedCharacter);
    paintConditionalLeds (g, displayedSync, displayedStereo);
}

void PanelControls::paintSyncSwitch (juce::Graphics& g, bool sync)
{
    const juce::Rectangle<float> track { Layout::syncSwitchX, Layout::syncSwitchY,
                                         Layout::syncSwitchW, Layout::syncSwitchH };

    g.setColour (Colour::switchTrack);
    g.fillRoundedRectangle (track, 2.0f);
    g.setColour (Colour::switchBorder);
    g.drawRoundedRectangle (track, 2.0f, 1.0f);

    {
        juce::ColourGradient recess { juce::Colours::black.withAlpha (0.8f), track.getCentreX(), track.getY(),
                                      juce::Colours::transparentBlack, track.getCentreX(), track.getY() + 7.0f, false };
        g.setGradientFill (recess);
        g.fillRoundedRectangle (track, 2.0f);
    }

    const float x = Layout::syncThumbOffX
                  + thumbPosition * (Layout::syncThumbOnX - Layout::syncThumbOffX);
    const juce::Rectangle<float> thumb { x, Layout::syncThumbY, Layout::syncThumbW, Layout::syncThumbH };

    {
        juce::Path path;
        path.addRoundedRectangle (thumb, 1.0f);
        juce::DropShadow shadow { juce::Colours::black.withAlpha (0.7f), 3, { 0, 1 } };
        shadow.drawForPath (g, path);
    }

    g.setGradientFill (Paint::vertical (thumb, Colour::switchThumbTop, Colour::switchThumbBot));
    g.fillRoundedRectangle (thumb, 1.0f);

    Text::drawTracked (g, sync ? "SYNC ON" : "SYNC OFF",
                       Font::label (11.0f), Font::trackingPx (0.22f, 11.0f),
                       { Layout::syncCaptionX, Layout::syncCaptionY, 200.0f, 13.75f },
                       juce::Justification::left, Colour::panelText);
}

void PanelControls::paintDivisionRow (juce::Graphics& g, bool, int division)
{
    const auto font = Font::mono (12.0f);

    for (int i = 0; i < numNoteDivisions; ++i)
    {
        const juce::Rectangle<float> r { Layout::divisionButtonX0 + (float) i * Layout::divisionButtonPitch,
                                         Layout::divisionButtonY,
                                         Layout::divisionButtonW, Layout::divisionButtonH };

        const bool selected = i == division;
        const bool isHovered = hovered != nullptr && hovered->region == Region::division && hovered->index == i;

        Paint::drawButtonFace (g, r, selected, isHovered);

        // The button's own lamp, plus the label - three redundant selection signals with no colour
        // change, so it still reads on a dim stage.
        const float ledX = r.getX() + 9.0f;
        Paint::drawLed (g, { ledX, r.getCentreY() }, Layout::ledStandard - 1.0f, selected);

        g.setFont (font);
        g.setColour (selected ? Colour::labelBright : Colour::buttonLabelUnselected);
        g.drawText (Timing::divisionLabel (i),
                    r.withTrimmedLeft (16.0f), juce::Justification::centred, false);
    }
}

void PanelControls::paintStereoRow (juce::Graphics& g, int stereoMode)
{
    for (int i = 0; i < numStereoModes; ++i)
    {
        const juce::Rectangle<float> r { Layout::stereoButtonX[(size_t) i], Layout::stereoButtonY,
                                         Layout::stereoButtonW, Layout::stereoButtonH };

        const bool selected = i == stereoMode;
        const bool isHovered = hovered != nullptr && hovered->region == Region::stereo && hovered->index == i;

        Paint::drawButtonFace (g, r, selected, isHovered);
        Paint::drawLed (g, { r.getX() + 14.0f, r.getCentreY() }, Layout::ledStandard, selected);

        Text::drawTracked (g, stereoNames[(size_t) i], Font::label (11.0f),
                           Font::trackingPx (0.18f, 11.0f),
                           r.withTrimmedLeft (24.0f), juce::Justification::centred,
                           selected ? Colour::labelBright : Colour::buttonLabelUnselected);
    }
}

void PanelControls::paintCharacterColumn (juce::Graphics& g, int character)
{
    for (int i = 0; i < numDelayCharacters; ++i)
    {
        const juce::Rectangle<float> r { Layout::modeButtonX,
                                         Layout::modeButtonY0 + (float) i * Layout::modeButtonPitch,
                                         Layout::modeButtonW, Layout::modeButtonH };

        const bool selected = i == character;
        const bool isHovered = hovered != nullptr && hovered->region == Region::character && hovered->index == i;

        Paint::drawButtonFace (g, r, selected, isHovered);
        Paint::drawLed (g, { Layout::modeLedX, r.getCentreY() }, Layout::modeLedDiameter, selected);

        Text::drawTracked (g, characterNames[(size_t) i], Font::label (Layout::modeLabelSize),
                           Font::trackingPx (0.24f, Layout::modeLabelSize),
                           { Layout::modeLabelX, r.getCentreY() - 9.0f, 120.0f, 18.0f },
                           juce::Justification::left,
                           selected ? Colour::labelBright : Colour::buttonLabelUnselected);
    }
}

void PanelControls::paintDialLabels (juce::Graphics& g, int character)
{
    const auto& table = dialLabels();
    const auto font = Font::label (Layout::dialLabelSize);
    const float tracking = Font::trackingPx (Layout::dialLabelTracking, Layout::dialLabelSize);

    for (size_t dial = 0; dial < table.size(); ++dial)
    {
        // The stack is left-aligned internally but centred as a block on the dial, so measure the
        // widest label this position can ever show - the layout must not shift when the mode does.
        float widest = 0.0f;

        for (const auto& label : table[dial])
            if (label.text != nullptr)
                widest = juce::jmax (widest, Text::trackedWidth (label.text, font, tracking));

        const float stackWidth = widest + Layout::dialLabelTextOffsetX;
        const float stackX = Layout::dialCentreX[dial] - stackWidth * 0.5f;

        for (size_t mode = 0; mode < table[dial].size(); ++mode)
        {
            const auto* text = table[dial][mode].text;

            if (text == nullptr)
                continue;

            const float rowY = Layout::dialLabelStackTop + (float) mode * Layout::dialLabelRowPitch;
            const bool lit = (int) mode == character;

            Paint::drawLed (g, { stackX + Layout::dialLedOffsetX, rowY + Layout::dialLedOffsetY },
                            Layout::dialLedDiameter, lit);

            Text::drawTracked (g, text, font, tracking,
                               { stackX + Layout::dialLabelTextOffsetX, rowY, widest + 4.0f, 12.5f },
                               juce::Justification::left,
                               lit ? Colour::stackLabelLit : Colour::stackLabelUnlit);
        }
    }
}

void PanelControls::paintConditionalLeds (juce::Graphics& g, bool sync, int stereoMode)
{
    // Exactly three, and nowhere else on the panel. NOTE DIVISION and TIME are mutually exclusive;
    // CROSS-FEED is live only in Ping-Pong. Feedback, Mix, Output Trim, Damping and Saturation
    // are always in circuit and deliberately carry no lamp.
    Paint::drawLed (g, Layout::divisionLedCentre, Layout::ledStandard, sync);
    Paint::drawLed (g, Layout::timeLedCentre, Layout::ledStandard, ! sync);
    Paint::drawLed (g, Layout::crossFeedLedCentre, Layout::ledStandard,
                    stereoMode == (int) StereoMode::pingPong);

    // The TIME and CROSS-FEED captions are NOT drawn here. They used to be, so they could brighten
    // with their own lamp - but revision 2's plate bakes every knob name (handoff section 1), and
    // drawing them again double-printed both at a one-pixel offset. It is visible on CROSS-FEED,
    // where the live and baked copies sit ~11px apart.
    //
    // Section 2.4 lists what changes with these two states and it is the LED, nothing else. So the
    // lamp above carries it and the printed name stays put, which is also what section 2.2 asks of
    // every other control on the panel.
}
