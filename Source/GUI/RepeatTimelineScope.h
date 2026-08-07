#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FifthMemberTheme.h"

#include <deque>

class FifthMemberAudioProcessor;

/**
    The repeat-timeline scope: a short, wide strip that plots discrete pulses at fixed timeline
    positions.

    Deliberately not the taller proportions Gatecrasher, CHORUS-60 and Reflect-84 use, because
    those plot a continuous waveform and this plots events. 96 px tall in a 1100 px fascia.

    The box splits in two:

      - a **22 px readout strip** carrying ALL the variable text - mode descriptor, stereo mode,
        feedback %, ms/DIV, note division, BPM, time - in Share Tech Mono, the same face as the
        PROGRAM LCD;
      - a **74 px plot area** carrying nothing but the grid, the baseline, the pulses and two
        corner legends.

    That division is the point. Variable text in the panel's silk-screen face, or sitting among the
    pulses, reads as a printed label; variable text in segment type inside its own strip reads as a
    screen. Nothing dynamic is ever drawn in the plot zone.

    The pulse train is driven from the processor's own measured loop gain rather than
    re-simulating the decay here - design/README.md: "drive it from the actual delay line's tap
    amplitudes rather than re-simulating decay in the UI".
*/
class RepeatTimelineScope final : public juce::Component,
                                  private juce::Timer
{
public:
    explicit RepeatTimelineScope (FifthMemberAudioProcessor& processor);
    ~RepeatTimelineScope() override;

    void paint (juce::Graphics& g) override;

private:
    struct Pulse
    {
        double spawnMs;
        float amplitude;
        bool dry;
    };

    void timerCallback() override;
    void paintReadoutStrip (juce::Graphics& g, juce::Rectangle<float> strip);
    void paintPlot (juce::Graphics& g, juce::Rectangle<float> plot);

    juce::String modeDescriptor() const;

    FifthMemberAudioProcessor& processorRef;

    std::deque<Pulse> pulses;
    double lastSpawnMs = 0.0;
    float runningAmplitude = 1.0f;
    float lampPhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatTimelineScope)
};
