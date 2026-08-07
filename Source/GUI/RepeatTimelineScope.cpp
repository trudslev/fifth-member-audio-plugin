#include "RepeatTimelineScope.h"

#include "../PluginProcessor.h"

#include <cmath>

using namespace FifthMemberTheme;

namespace
{
    double nowMs()
    {
        return (double) juce::Time::getMillisecondCounterHiRes();
    }
}

RepeatTimelineScope::RepeatTimelineScope (FifthMemberAudioProcessor& processor)
    : processorRef (processor)
{
    setInterceptsMouseClicks (false, false);
    setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);

    lastSpawnMs = nowMs();
    startTimerHz (Layout::animationHz);
}

RepeatTimelineScope::~RepeatTimelineScope()
{
    stopTimer();
}

//==============================================================================
void RepeatTimelineScope::timerCallback()
{
    const double now = nowMs();
    const float delayMs = juce::jmax (30.0f, processorRef.getDelayMs());

    // The decay comes from the loop's own measured gain - feedback times whatever the character
    // engine is really costing each pass - not from a parallel simulation of it.
    const float perPass = juce::jlimit (0.0f, 1.2f, processorRef.getPerPassGain());

    // Catch-up loop, so spawn times stay exact multiples of the delay rather than drifting with
    // the frame rate.
    while (now - lastSpawnMs >= (double) delayMs)
    {
        lastSpawnMs += (double) delayMs;
        runningAmplitude *= perPass;

        const bool dry = runningAmplitude < Layout::dryResetThreshold;

        if (dry)
            runningAmplitude = 1.0f;

        pulses.push_back ({ lastSpawnMs, juce::jlimit (0.0f, 1.0f, runningAmplitude), dry });
    }

    while (! pulses.empty() && now - pulses.front().spawnMs > (double) Layout::pulseWindowMs)
        pulses.pop_front();

    // The lamp breathes on a 1.6 s cycle, per the design's keyframes.
    lampPhase = std::fmod (lampPhase + 1.0f / (1.6f * (float) Layout::animationHz), 1.0f);

    repaint();
}

//==============================================================================
juce::String RepeatTimelineScope::modeDescriptor() const
{
    const auto* p = dynamic_cast<const juce::AudioParameterChoice*> (
        processorRef.apvts.getParameter (ParamIDs::character));

    switch (p != nullptr ? p->getIndex() : 0)
    {
        case 1:  return "BBD BUCKET-BRIGADE";
        case 2:  return "DIGITAL " + Text::middleDot() + " 16-BIT";
        default: return "TAPE ECHO " + Text::middleDot() + " 3-HEAD";
    }
}

void RepeatTimelineScope::paint (juce::Graphics& g)
{
    // --- the lamp: the panel's one piece of coloured light ---------------------
    {
        const float breathe = 0.72f + 0.28f * (0.5f + 0.5f * std::cos (lampPhase * juce::MathConstants<float>::twoPi));
        const auto centre = Layout::lampCentre;
        const float r = Layout::lampDiameter * 0.5f;

        // Tight, and low-alpha at its edge. The design's "0 0 22px" is a blur radius on a 13 px
        // lamp, not a 22 px disc of colour - drawn as the latter it washed straight over the
        // REPEATS LIVE caption 17 px to its right.
        const float glow = 10.0f;
        juce::ColourGradient halo { Colour::accent.withAlpha (0.40f * breathe), centre.x, centre.y,
                                    Colour::accent.withAlpha (0.0f), centre.x + glow, centre.y, true };
        halo.addColour (0.45, Colour::accent.withAlpha (0.12f * breathe));
        g.setGradientFill (halo);
        g.fillEllipse (centre.x - glow, centre.y - glow, glow * 2.0f, glow * 2.0f);

        const juce::Rectangle<float> bulb { centre.x - r, centre.y - r, Layout::lampDiameter, Layout::lampDiameter };
        g.setColour (Colour::accent.withMultipliedBrightness (breathe));
        g.fillEllipse (bulb);

        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawEllipse (bulb.reduced (0.5f).translated (0.0f, 0.5f), 1.0f);
    }

    // --- the box --------------------------------------------------------------
    const juce::Rectangle<float> box { Layout::scopeX, Layout::scopeY, Layout::scopeW, Layout::scopeH };

    g.setColour (Colour::scopeBg);
    g.fillRoundedRectangle (box, 2.0f);

    {
        juce::ColourGradient recess { juce::Colours::black.withAlpha (0.9f), box.getCentreX(), box.getY(),
                                      juce::Colours::transparentBlack, box.getCentreX(), box.getY() + 14.0f, false };
        g.setGradientFill (recess);
        g.fillRoundedRectangle (box, 2.0f);
    }

    g.setColour (Colour::scopeBorder);
    g.drawRoundedRectangle (box, 2.0f, 1.0f);

    const juce::Rectangle<float> strip { Layout::scopeInnerX, Layout::scopeInnerY,
                                         Layout::scopeInnerW, Layout::readoutStripH };
    const juce::Rectangle<float> plot { Layout::scopeInnerX, Layout::plotY,
                                        Layout::scopeInnerW, Layout::plotH };

    paintReadoutStrip (g, strip);
    paintPlot (g, plot);
}

//==============================================================================
void RepeatTimelineScope::paintReadoutStrip (juce::Graphics& g, juce::Rectangle<float> strip)
{
    g.setColour (Colour::scopeStripRule);
    g.fillRect (strip.getX(), strip.getBottom(), strip.getWidth(), 1.0f);

    const auto font = Font::mono (Layout::readoutTextSize);
    const float tracking = Font::trackingPx (Layout::readoutTracking, Layout::readoutTextSize);
    const auto dot = Text::middleDot();

    const auto* stereo = dynamic_cast<const juce::AudioParameterChoice*> (
        processorRef.apvts.getParameter (ParamIDs::stereoMode));
    const auto* division = dynamic_cast<const juce::AudioParameterChoice*> (
        processorRef.apvts.getParameter (ParamIDs::noteDivision));

    const float feedback = processorRef.apvts.getRawParameterValue (ParamIDs::feedback)->load();
    const bool sync = processorRef.apvts.getRawParameterValue (ParamIDs::sync)->load() > 0.5f;
    const float delayMs = processorRef.getDelayMs();

    const juce::String stereoText = stereo != nullptr ? stereo->getCurrentChoiceName().toUpperCase() : "STEREO";

    const juce::String descriptor = modeDescriptor() + " " + dot + " " + stereoText
                                  + " " + dot + " FB " + juce::String (juce::roundToInt (feedback)) + "%"
                                  + " " + dot + " " + juce::String (juce::roundToInt (delayMs)) + " ms / DIV";

    const juce::String timing = (sync
            ? juce::String (division != nullptr ? Timing::divisionLabel (division->getIndex()) : "1/8")
                  + " " + dot + " " + juce::String (juce::roundToInt (processorRef.getBpm())) + " BPM"
            : juce::String ("FREE"))
        + " " + dot + " " + juce::String (juce::roundToInt (delayMs)) + " ms";

    const auto inner = strip.reduced (Layout::readoutPadX, 0.0f);

    // Left-anchored pair.
    float x = inner.getX();

    const auto drawLeft = [&] (const juce::String& text, juce::Colour colour)
    {
        const float w = Text::trackedWidth (text, font, tracking);
        Text::drawTracked (g, text, font, tracking, { x, strip.getY(), w + 2.0f, strip.getHeight() },
                           juce::Justification::left, colour);
        x += w + Layout::readoutGap;
    };

    drawLeft ("RPT ENV", Colour::scopeReadout);
    drawLeft (descriptor, Colour::scopeReadoutHi);

    // Right-anchored pair, laid out right to left so the flex spacer's behaviour is reproduced.
    float right = inner.getRight();

    const auto drawRight = [&] (const juce::String& text, juce::Colour colour)
    {
        const float w = Text::trackedWidth (text, font, tracking);
        Text::drawTracked (g, text, font, tracking,
                           { right - w, strip.getY(), w + 2.0f, strip.getHeight() },
                           juce::Justification::left, colour);
        right -= w + Layout::readoutGap;
    };

    drawRight ("0 dB", Colour::scopeReadout);
    drawRight (timing, Colour::scopeReadout);
}

//==============================================================================
void RepeatTimelineScope::paintPlot (juce::Graphics& g, juce::Rectangle<float> plot)
{
    juce::Graphics::ScopedSaveState save { g };
    g.reduceClipRegion (plot.getSmallestIntegerContainer());

    const float W = plot.getWidth();
    const float H = plot.getHeight();
    const float baseline = plot.getY() + H - Layout::baselineInset;
    const float span = (H - Layout::baselineInset) - Layout::spanInset;

    // --- grid ----------------------------------------------------------------
    g.setColour (Colour::scopeGrid);

    for (int i = 1; i < Layout::gridDivisions; ++i)
    {
        const float x = plot.getX() + (W / (float) Layout::gridDivisions) * (float) i;
        g.fillRect (x, plot.getY(), 1.0f, H);
    }

    g.fillRect (plot.getX(), plot.getY() + H * 0.5f, W, 1.0f);

    const double now = nowMs();

    const auto heightFor = [span] (float amplitude)
    {
        // The 0.45 exponent is what keeps late repeats visible; linearising it would make the tail
        // vanish long before it is inaudible.
        return span * juce::jlimit (0.04f, 1.0f,
                                    std::pow (juce::jmax (0.0f, amplitude), Layout::heightExponent));
    };

    const auto xFor = [&] (double spawnMs)
    {
        return plot.getRight() - (float) ((now - spawnMs) / (double) Layout::pulseWindowMs) * W;
    };

    // --- ghost tails, behind everything ---------------------------------------
    g.setColour (Colour::scopeGhost);

    for (const auto& pulse : pulses)
    {
        const float h = heightFor (pulse.amplitude);
        g.fillRect (xFor (pulse.spawnMs), baseline - h, juce::jmax (8.0f, W * 0.014f), h);
    }

    // --- baseline -------------------------------------------------------------
    g.setColour (Colour::scopeBaseline);
    g.fillRect (plot.getX(), baseline, W, 1.0f);

    // --- pulses ---------------------------------------------------------------
    for (const auto& pulse : pulses)
    {
        const float x = xFor (pulse.spawnMs);
        const float h = heightFor (pulse.amplitude);
        const float age = (float) ((now - pulse.spawnMs) / 260.0);
        const float glow = juce::jmax (4.0f, 16.0f - age * 6.0f);

        const auto colour = Colour::accent.withAlpha (pulse.dry ? 1.0f : 0.92f);

        // Freshly-spawned pulses bloom; the glow decays to a floor over about half a second.
        g.setColour (colour.withMultipliedAlpha (0.28f));
        g.fillRect (x - glow * 0.18f, baseline - h, glow * 0.36f, h);

        g.setColour (colour);
        g.fillRect (x - (pulse.dry ? 1.5f : 1.1f), baseline - h, pulse.dry ? 3.0f : 2.2f, h);

        const float capR = pulse.dry ? 3.2f : 2.2f;
        g.fillEllipse (x - capR, baseline - h - capR, capR * 2.0f, capR * 2.0f);
    }

    // --- ping-pong marker ------------------------------------------------------
    {
        const auto* stereo = dynamic_cast<const juce::AudioParameterChoice*> (
            processorRef.apvts.getParameter (ParamIDs::stereoMode));

        if (stereo != nullptr && stereo->getIndex() == (int) StereoMode::pingPong)
        {
            g.setColour (Colour::scopePingMarker);
            const float y = baseline - span * 0.5f;

            for (float x = plot.getX(); x < plot.getRight(); x += 7.0f)
                g.fillRect (x, y, 3.0f, 1.0f);
        }
    }

    // Deliberately NO text in the plot zone. Every variable readout - and the "RPT ENV" and
    // "0 dB" legends with them - lives in the strip above, in Share Tech Mono. Repeating them down
    // here was the one thing the brief rules out: variable text among the pulses reads as a
    // printed label rather than as a screen.
}
