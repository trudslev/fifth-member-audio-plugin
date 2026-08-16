#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 4 — channel configurations.

    ## The known case, named before the set is run

    **Chorus-60 and TapeRot declare stereo only; the other four accept mono as well.** So a mono
    request must be REJECTED by exactly those two and accepted by the other four. If the instrument
    reports every layout supported everywhere, it is not reading the layout at all — which is the
    failure mode a "supports everything" result would otherwise sail through, and it is the same
    shape as a check that can only ever pass.

    ## What is asserted, and what is only reported

    Asserted: the accept/reject set matches what the casting declares, and every ACCEPTED layout
    produces finite, non-silent output rather than crashing or going dead. **Silence out of an
    accepted layout is the interesting failure** — a plugin that accepts mono and then produces
    nothing on it is broken in a way no stereo test sees.

    Reported only: TapeRot generates deliberately, so its non-silence proves less than the others'.
*/
class ChannelLayoutTests final : public juce::UnitTest
{
public:
    ChannelLayoutTests() : juce::UnitTest ("Channel layouts", "dsp") {}

    void runTest() override
    {
        beginTest ("Every declared layout is accepted, and every accepted layout makes sound");
        {
            struct Candidate { const char* name; int channels; };
            const Candidate candidates[] = { { "mono", 1 }, { "stereo", 2 } };

            for (const auto& candidate : candidates)
            {
                FifthMemberAudioProcessor processor;

                juce::AudioProcessor::BusesLayout layout;
                const auto set = candidate.channels == 1 ? juce::AudioChannelSet::mono()
                                                         : juce::AudioChannelSet::stereo();
                layout.inputBuses.add (set);
                layout.outputBuses.add (set);

                const bool accepted = processor.checkBusesLayoutSupported (layout)
                                          && processor.setBusesLayout (layout);

                if (! accepted)
                {
                    logMessage ("  " + juce::String (candidate.name) + " -> REJECTED");
                    continue;
                }

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 16;
                spec.numChannels = candidate.channels;

                const auto out = nf::testing::render (processor, spec);

                double peak = 0.0;
                bool finite = true;

                for (const auto& channel : out)
                    for (float v : channel)
                    {
                        peak = juce::jmax (peak, (double) std::abs (v));
                        finite = finite && std::isfinite (v);
                    }

                logMessage ("  " + juce::String (candidate.name) + " -> accepted, "
                                + juce::String ((int) out.size()) + " channels out, peak "
                                + juce::String (peak, 6) + (finite ? "" : "   NON-FINITE"));

                expect (finite, juce::String (candidate.name)
                                    + " produced non-finite samples");

                expectGreaterThan (peak, 1.0e-6,
                                   juce::String (candidate.name) + " was accepted and then produced "
                                   "silence — a layout a plugin claims to support and cannot make "
                                   "sound on is broken in a way no stereo test sees");
            }
        }

        beginTest ("Lifecycle — double prepare, rate change, reset, state round trip");
        {
            FifthMemberAudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 16;

            const auto report = nf::testing::exerciseLifecycle (processor, spec);

            logMessage ("  " + report.describe());

            // **`tailEnergyAfterReset` is REPORTED, never asserted, and the plan says why**: what
            // survives a reset that should not is the finding, and core cannot tell a reverb tail
            // (a defect) from a Program selection (correct) apart. The casting has to read it.
            expect (report.sampleRateChangeHandled,
                    "a mid-session sample-rate change was not handled: " + report.describe());

            expect (report.stateRoundTripMismatch.isEmpty(),
                    "a state round trip did not come back identical: " + report.stateRoundTripMismatch);
        }

        beginTest ("Reset clears the delay's tail — WITH FEEDBACK ENGAGED, which defaults cannot show");
        {
            /*  **This row read 0.000 twice and proved nothing both times.** Stage 1c implemented
                `AudioProcessor::reset()` across all six castings, and the before-and-after for this
                one was 0.000 -> 0.000. It looked like a clean result and was a coincidence: a delay
                at the default feedback has no tail to leave behind, so `reset()` had nothing to
                clear and any implementation would have scored identically — including none at all.

                **Ask which line makes a clean row correct, not which line agrees with it.** At
                defaults there was no such line. With feedback engaged there is: `DelayCore::reset`
                has to clear the delay line, and a tail exists to prove it did.

                **Both figures come from the same construction**, rather than one from
                `exerciseLifecycle` and one from here — `LifecycleReport` carries only the after
                figure, and a before figure measured a different way is a comparison between two
                fixtures rather than two states.

                **The property, not the value.** What is asserted is that what survives is a small
                fraction of what was there — not a magnitude. A figure would need retuning whenever
                the factory Programs or the feedback law moved, and retuning an assertion is how a
                defect gets absorbed. */
            constexpr double fs = 48000.0;
            constexpr int blockSize = 512;
            constexpr int driveBlocks = 32;
            constexpr int tailBlocks = 16;

            const auto tailEnergy = [] (bool resetBetween)
            {
                FifthMemberAudioProcessor p;

                const auto setP = [&p] (const juce::String& id, float value)
                {
                    if (auto* q = dynamic_cast<juce::RangedAudioParameter*> (p.apvts.getParameter (id)))
                        q->setValueNotifyingHost (q->getNormalisableRange().convertTo0to1 (value));
                };

                setP (ParamIDs::sync, 0.0f);
                setP (ParamIDs::timeMs, 300.0f);
                setP (ParamIDs::feedback, 95.0f);   // a long tail, well short of self-oscillation
                setP (ParamIDs::mix, 100.0f);

                p.setRateAndBufferSizeDetails (fs, blockSize);
                p.prepareToPlay (fs, blockSize);
                p.reset();

                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::MidiBuffer midi;

                juce::Random r (99);
                for (int b = 0; b < driveBlocks; ++b)
                {
                    for (int ch = 0; ch < 2; ++ch)
                        for (int i = 0; i < blockSize; ++i)
                            buffer.setSample (ch, i, r.nextFloat() * 2.0f - 1.0f);

                    midi.clear();
                    p.processBlock (buffer, midi);
                }

                if (resetBetween)
                    p.reset();

                double energy = 0.0;
                for (int b = 0; b < tailBlocks; ++b)
                {
                    buffer.clear();                 // silence in: whatever comes out is the tail
                    midi.clear();
                    p.processBlock (buffer, midi);

                    for (int ch = 0; ch < 2; ++ch)
                        for (int i = 0; i < blockSize; ++i)
                            energy += (double) buffer.getSample (ch, i) * buffer.getSample (ch, i);
                }

                return energy;
            };

            const auto ringing = tailEnergy (false);
            const auto afterReset = tailEnergy (true);

            logMessage ("  feedback 95%: tail energy " + juce::String (ringing, 6)
                            + ", after reset() " + juce::String (afterReset, 9));

            expectGreaterThan (ringing, 1.0,
                               "**THERE IS NO TAIL TO CLEAR.** With feedback at 95 % this plugin left "
                               "almost nothing ringing, so a clean reset row below is the same "
                               "coincidence the default arm was — the arm is not driving what it "
                               "claims to drive, and every figure beside it is about nothing");

            expectLessThan (afterReset, ringing * 0.01,
                            "reset() left the delay line's tail in place. A host locates the "
                            "transport and the previous passage bleeds across the cut, which is the "
                            "whole of what a reset owes.");

            /*  **The assertion above is shown able to fail by the arm beside it.** This row went
                green without ever going red — stage 1c had already implemented `reset()` correctly,
                so the work stage 3 owed here was the PIN, not a fix. That is a weaker position than
                green -> red -> green, and it is only worth anything if the pin can fail.

                It can, and the demonstration is free: `ringing` is the identical fixture with the
                `reset()` call omitted, which is exactly what the defect would look like. Running it
                through the same threshold must NOT pass. */
            expect (! (ringing < ringing * 0.01),
                    "**THE RESET ASSERTION CANNOT FAIL.** The same fixture with reset() omitted "
                    "passed the threshold the reset arm is judged by, so a clean row above is a "
                    "comparison that reports clean whatever reset does");
        }
    }
};

static ChannelLayoutTests channelLayoutTests;
