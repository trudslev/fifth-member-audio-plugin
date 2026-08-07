#include "TestUtils.h"

#include "../Source/DSP/TimingEngine.h"

#include <juce_core/juce_core.h>

/** A play head that reports exactly what a test tells it to, including "no tempo at all". */
class FakePlayHead final : public juce::AudioPlayHead
{
public:
    juce::Optional<PositionInfo> getPosition() const override
    {
        if (! hasPosition)
            return juce::nullopt;

        PositionInfo info;
        info.setIsPlaying (playing);

        if (hasTempo)
            info.setBpm (bpm);

        if (hasPpq)
            info.setPpqPosition (ppq);

        return info;
    }

    bool canControlTransport() override { return false; }

    bool hasPosition = true, hasTempo = true, hasPpq = true, playing = true;
    double bpm = 120.0, ppq = 0.0;
};

class TimingEngineTests final : public juce::UnitTest
{
public:
    TimingEngineTests() : juce::UnitTest ("TimingEngine", "dsp") {}

    void runTest() override
    {
        beginTest ("note division converts to milliseconds correctly");
        {
            // (60000 / bpm) x multiplier, for 1/4, 1/8., 1/8, 1/8T, 1/16.
            const std::array<float, 5> at120 { 500.0f, 375.0f, 250.0f, 500.0f / 3.0f, 125.0f };

            for (int d = 0; d < numNoteDivisions; ++d)
                expectWithinAbsoluteError (TimingEngine::delayMsFor (true, d, 0.0f, 120.0),
                                           at120[(size_t) d], 0.01f);

            expectWithinAbsoluteError (TimingEngine::delayMsFor (true, 0, 0.0f, 60.0), 1000.0f, 0.01f);
            expectWithinAbsoluteError (TimingEngine::delayMsFor (true, 4, 0.0f, 200.0), 75.0f, 0.01f);

            // Unsynced ignores tempo entirely.
            expectWithinAbsoluteError (TimingEngine::delayMsFor (false, 0, 375.0f, 200.0), 375.0f, 0.01f);
        }

        beginTest ("the defaults make Sync inaudible at 120 BPM");
        {
            // Dotted 1/8 at 120 BPM is 375 ms, which is also the free-run Time default - so
            // toggling Sync at the defaults changes nothing audible.
            expectWithinAbsoluteError (
                TimingEngine::delayMsFor (true, ParamDefaults::noteDivision, 0.0f, 120.0),
                ParamDefaults::timeMs, 0.01f);
        }

        beginTest ("no play head falls back to free-running tempo");
        {
            TimingEngine engine;
            engine.prepare ({ 48000.0, 512, 2 });

            const auto snapshot = engine.update (nullptr, true, (int) NoteDivision::quarter, 375.0f, 512);

            expect (! snapshot.hostTempoValid, "claimed a host tempo with no play head");
            expectWithinAbsoluteError ((float) snapshot.bpm, (float) Timing::fallbackBpm, 0.01f);
            expectWithinAbsoluteError (snapshot.delayMs, 500.0f, 0.01f);
        }

        beginTest ("a play head reporting no tempo also falls back");
        {
            TimingEngine engine;
            engine.prepare ({ 48000.0, 512, 2 });

            FakePlayHead head;
            head.hasTempo = false;

            const auto snapshot = engine.update (&head, true, (int) NoteDivision::quarter, 375.0f, 512);
            expect (! snapshot.hostTempoValid);
            expectWithinAbsoluteError ((float) snapshot.bpm, 120.0f, 0.01f);
        }

        beginTest ("the last known tempo is retained when the host stops reporting one");
        {
            TimingEngine engine;
            engine.prepare ({ 48000.0, 512, 2 });

            FakePlayHead head;
            head.bpm = 90.0;
            engine.update (&head, true, (int) NoteDivision::quarter, 375.0f, 512);

            head.hasTempo = false;
            const auto snapshot = engine.update (&head, true, (int) NoteDivision::quarter, 375.0f, 512);

            expect (! snapshot.hostTempoValid, "should report the figure as a fallback");
            expectWithinAbsoluteError ((float) snapshot.bpm, 90.0f, 0.01f,
                                       "dropped back to 120 instead of holding the last known tempo");
        }

        beginTest ("continuous playback does not read as a transport jump");
        {
            TimingEngine engine;
            engine.prepare ({ 48000.0, 512, 2 });

            FakePlayHead head;
            head.bpm = 120.0;

            const double beatsPerBlock = (512.0 / 48000.0) * (120.0 / 60.0);

            for (int block = 0; block < 40; ++block)
            {
                const auto snapshot = engine.update (&head, true, 2, 375.0f, 512);

                if (block > 0)
                    expect (! snapshot.transportJumped,
                            "block " + juce::String (block) + " read as a jump during steady playback");

                head.ppq += beatsPerBlock;
            }
        }

        beginTest ("a loop wrap is detected as a jump");
        {
            TimingEngine engine;
            engine.prepare ({ 48000.0, 512, 2 });

            FakePlayHead head;
            head.bpm = 120.0;
            const double beatsPerBlock = (512.0 / 48000.0) * 2.0;

            for (int block = 0; block < 8; ++block)
            {
                engine.update (&head, true, 2, 375.0f, 512);
                head.ppq += beatsPerBlock;
            }

            head.ppq = 0.0;   // the loop wraps back to the top
            const auto snapshot = engine.update (&head, true, 2, 375.0f, 512);

            expect (snapshot.transportJumped, "a backwards PPQ jump was not detected");
        }

        beginTest ("delay time is bounded by the parameter range");
        {
            // A quarter note at a pathological 20 BPM is 3000 ms; the line only goes to 2000.
            expectWithinAbsoluteError (TimingEngine::delayMsFor (true, 0, 0.0f, 20.0),
                                       Timing::maxDelayMs, 0.01f);
            expect (TimingEngine::delayMsFor (true, 4, 0.0f, 999.0) >= Timing::minDelayMs);
        }
    }
};

static TimingEngineTests timingEngineTests;
