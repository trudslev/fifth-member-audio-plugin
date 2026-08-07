#include "TimingEngine.h"

#include "../Parameters.h"

#include <cmath>

void TimingEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void TimingEngine::reset()
{
    lastPpq = 0.0;
    hadPpq = false;
    // lastKnownBpm deliberately survives a reset: the host's tempo does not change because the
    // plugin was re-prepared, and losing it would drop a synced delay back to 120 for one block.
}

float TimingEngine::delayMsFor (bool sync, int division, float timeMs, double bpm) noexcept
{
    if (! sync)
        return juce::jlimit (Timing::minDelayMs, Timing::maxDelayMs, timeMs);

    const double safeBpm = bpm > 1.0 ? bpm : Timing::fallbackBpm;
    const double quarterMs = 60000.0 / safeBpm;
    const double ms = quarterMs * (double) Timing::divisionMultiplier (division);

    return juce::jlimit (Timing::minDelayMs, Timing::maxDelayMs, (float) ms);
}

TimingEngine::Snapshot TimingEngine::update (juce::AudioPlayHead* playHead,
                                             bool sync,
                                             int division,
                                             float timeMs,
                                             int numSamples)
{
    Snapshot snapshot;

    // The JUCE 8 shape: getPlayHead() may be null, and getPosition() returns an optional. Both
    // guards are needed - Standalone has no transport concept at all.
    if (playHead != nullptr)
    {
        if (const auto position = playHead->getPosition())
        {
            snapshot.isPlaying = position->getIsPlaying();

            if (const auto hostBpm = position->getBpm(); hostBpm.hasValue() && *hostBpm > 1.0)
            {
                lastKnownBpm = *hostBpm;
                snapshot.hostTempoValid = true;
            }

            if (const auto ppq = position->getPpqPosition(); ppq.hasValue())
            {
                snapshot.ppqPosition = *ppq;

                // A discontinuity is either backwards motion - a loop wrap or a locate, which can
                // be arbitrarily small - or a forward skip of several blocks at once. Comparing
                // against a single tolerance band misses short loops entirely: a two-bar loop
                // wrapping at the top moves PPQ back by less than one block's worth of beats.
                const double blockBeats = ((double) numSamples / sampleRate) * (lastKnownBpm / 60.0);
                const double delta = *ppq - lastPpq;

                if (hadPpq && (delta < -1.0e-6 || delta > blockBeats * 4.0 + jumpToleranceBeats))
                    snapshot.transportJumped = true;

                lastPpq = *ppq;
                hadPpq = true;
            }
            else
            {
                hadPpq = false;
            }
        }
    }
    else
    {
        hadPpq = false;
    }

    snapshot.bpm = lastKnownBpm;
    snapshot.delayMs = delayMsFor (sync, division, timeMs, lastKnownBpm);

    return snapshot;
}
