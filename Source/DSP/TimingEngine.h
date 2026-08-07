#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
    Turns the host transport into a delay time.

    This is the first code in the Neon Foundry suite to read host tempo at all. No sibling calls
    getPlayHead() for BPM or PPQ - TapeRot's play/stop gate is the only transport code anywhere, and
    only its null-guard idiom and its "engage only when the host actually reports it" philosophy
    transfer. Everything below is new ground.

    Three responsibilities:

    1. **Delay time.** Synced, `delayMs = (60000 / bpm) x divisionMultiplier`. Free, it is the Time
       parameter verbatim. At the defaults - 120 BPM, Dotted 1/8, Time 375 ms - both come to 375 ms,
       so toggling Sync at the defaults is inaudible.

    2. **Free-running fallback.** A Standalone build has no transport at all, and some hosts report a
       play head but no tempo. Either way the last known good BPM is retained, initially 120, so a
       synced delay still has a defensible time rather than dividing by zero.

    3. **Transport discontinuity.** A loop wrap, a scrub, or a locate makes PPQ jump. That is
       reported so the *scope* can re-anchor its pulse grid to the beat. It deliberately does NOT
       flush the delay line: hard-locking repeats to the grid would mean cutting the tail dead on
       every loop wrap, which is musically wrong for a delay. Delay time follows tempo continuously;
       only the timeline display re-anchors.
*/
class TimingEngine
{
public:
    struct Snapshot
    {
        float  delayMs = 375.0f;      ///< what DelayCore should target
        double bpm = 120.0;           ///< host tempo, or the retained fallback
        double ppqPosition = 0.0;     ///< beats since the timeline origin; 0 when unknown
        bool   hostTempoValid = false;///< false means the figure above is the fallback
        bool   isPlaying = false;
        bool   transportJumped = false;///< PPQ moved discontinuously since the last block
    };

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    /** Call once per block, before processing. `playHead` may be null. */
    Snapshot update (juce::AudioPlayHead* playHead,
                     bool sync,
                     int division,
                     float timeMs,
                     int numSamples);

    /** The delay time a given state would produce, without touching any transport state. Exposed
        so tests and the GUI can ask the same question the audio path asks. */
    static float delayMsFor (bool sync, int division, float timeMs, double bpm) noexcept;

private:
    double sampleRate = 44100.0;
    double lastKnownBpm = 120.0;
    double lastPpq = 0.0;
    bool   hadPpq = false;

    /** Forward slack on top of four blocks' worth of beats, so ordinary jitter and a tempo ramp
        never read as a locate. Backwards motion of any size is always a discontinuity. */
    static constexpr double jumpToleranceBeats = 0.02;
};
