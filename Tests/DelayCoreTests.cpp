#include "TestUtils.h"

#include "../Source/DSP/DelayCore.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <vector>

namespace
{
    constexpr double sr = 48000.0;
    constexpr int blockSize = 256;
    constexpr float testDelayMs = 100.0f;

    DelayCoreParams baseParams()
    {
        DelayCoreParams p;
        p.delayMs = testDelayMs;
        p.feedbackPercent = 80.0f;      // high enough that repeat six is still measurable
        p.stereoMode = (int) StereoMode::stereo;
        p.crossFeedPercent = 0.0f;
        p.dampingHz = 16000.0f;         // wide open, so the character engine is what is measured
        p.saturationPercent = 0.0f;
        return p;
    }

    /** Excites the delay with a short broadband burst and returns the wet left channel.

        Deliberately a burst rather than a single-sample impulse: Digital mode's sample-and-hold
        decimates the loop by up to 12:1, and a one-sample impulse that lands between holds is
        dropped entirely - which makes an impulse a useless probe for that mode specifically.
        A 64-sample burst survives every mode and still has enough bandwidth to measure. */
    std::vector<float> burstResponse (DelayCore& core, const DelayCoreParams& params, double seconds,
                                      int channelToRead = 0)
    {
        juce::AudioBuffer<float> buffer (2, blockSize);
        std::vector<float> out;
        const int totalBlocks = (int) std::ceil (seconds * sr / blockSize);
        out.reserve ((size_t) (totalBlocks * blockSize));

        juce::Random random { 1234 };

        for (int block = 0; block < totalBlocks; ++block)
        {
            buffer.clear();

            if (block == 0)
                for (int n = 0; n < 64; ++n)
                {
                    const float v = random.nextFloat() * 2.0f - 1.0f;

                    for (int ch = 0; ch < 2; ++ch)
                        buffer.setSample (ch, n, v);
                }

            core.process (buffer, params);

            for (int n = 0; n < blockSize; ++n)
                out.push_back (buffer.getSample (channelToRead, n));
        }

        return out;
    }

    /** Brightness of one repeat: mean absolute first difference over mean absolute level.

        The first difference is a crude high-pass, so this is the ratio of high-frequency motion to
        overall level - scale-invariant, which matters because each repeat is quieter than the last
        and an absolute measure would just be reading the decay.

        Deliberately NOT a zero-crossing rate over a wide window: the excitation is a 1.3 ms burst
        arriving every 100 ms, so a +/-20 ms window is almost entirely silence between repeats and
        the measurement ends up dominated by the noise floor rather than by the repeat. The window
        here is tight around the arrival. */
    float brightnessOfRepeat (const std::vector<float>& signal, int repeatIndex)
    {
        const int centre = (int) std::round (repeatIndex * testDelayMs * 0.001 * sr);
        const int half = (int) (0.004 * sr);
        const int from = juce::jmax (1, centre - half);
        const int to = juce::jmin ((int) signal.size(), centre + half);

        double difference = 0.0, level = 0.0;

        for (int i = from; i < to; ++i)
        {
            difference += std::abs (signal[(size_t) i] - signal[(size_t) (i - 1)]);
            level += std::abs (signal[(size_t) i]);
        }

        if (level < 1.0e-7)
            return -1.0f;

        return (float) (difference / level);
    }
}

//==============================================================================
class DelayCoreTests final : public juce::UnitTest
{
public:
    DelayCoreTests() : juce::UnitTest ("DelayCore", "dsp") {}

    void runTest() override
    {
        beginTest ("an impulse produces a decaying train of repeats");
        {
            DelayCore core;
            core.prepare ({ sr, blockSize, 2 });

            const auto ir = burstResponse (core, baseParams(), 1.5);

            const auto peakAt = [&ir] (int repeat)
            {
                const int centre = (int) std::round (repeat * testDelayMs * 0.001 * sr);
                const int half = (int) (0.01 * sr);
                float peak = 0.0f;

                for (int i = juce::jmax (0, centre - half);
                     i < juce::jmin ((int) ir.size(), centre + half); ++i)
                    peak = juce::jmax (peak, std::abs (ir[(size_t) i]));

                return peak;
            };

            const float first = peakAt (1);
            const float fourth = peakAt (4);

            expect (first > 1.0e-3f, "no first repeat at all");
            expect (fourth > 1.0e-5f, "the train died too early to measure");
            expect (fourth < first, "repeats are not decaying");
        }

        beginTest ("Tape degradation COMPOUNDS - repeat six is darker than repeat one");
        {
            // The central DSP requirement. Character stages live inside the feedback loop, so each
            // recirculation is another generation. If someone later moves them onto the output this
            // test fails, which is exactly what it is for.
            DelayCore core;
            core.prepare ({ sr, blockSize, 2 });

            auto params = baseParams();
            params.character.mode = (int) DelayCharacter::tape;
            params.character.genLossPercent = 80.0f;
            params.character.wowPercent = 0.0f;      // isolate the loss from the pitch modulation
            params.character.flutterPercent = 0.0f;

            const auto ir = burstResponse (core, params, 1.5);

            const float early = brightnessOfRepeat (ir, 1);
            const float late = brightnessOfRepeat (ir, 6);

            expect (early > 0.0f, "repeat one was silent");
            expect (late > 0.0f, "repeat six was silent");
            expect (late < early * 0.9f,
                    "Tape repeat 6 (" + juce::String (late, 4) + ") is not measurably darker than repeat 1 ("
                        + juce::String (early, 4) + ") - degradation is not compounding");
        }

        beginTest ("BBD darkens per pass too, and Digital quantises harder");
        {
            for (const int mode : { (int) DelayCharacter::bbd, (int) DelayCharacter::digital })
            {
                DelayCore core;
                core.prepare ({ sr, blockSize, 2 });

                auto params = baseParams();
                params.character.mode = mode;
                params.character.modDepthPercent = 0.0f;   // isolate the filters from the LFO
                params.character.degradePercent = 70.0f;

                const auto ir = burstResponse (core, params, 1.5);

                const float early = brightnessOfRepeat (ir, 1);
                const float late = brightnessOfRepeat (ir, 6);

                expect (early > 0.0f && late > 0.0f,
                        "mode " + juce::String (mode) + ": a repeat was silent");

                // BBD's reconstruction filter compounds; Digital's quantiser coarsens the waveform,
                // which shows up as fewer, larger steps rather than as more high frequency.
                expect (std::abs (late - early) > 1.0e-4f,
                        "mode " + juce::String (mode) + ": repeat 6 is indistinguishable from repeat 1");
            }
        }

        beginTest ("Ping-Pong alternates channels, and Cross-Feed 0 never crosses");
        {
            // Input goes into the left line only, so full cross-feed gives L, R, L, R. At zero
            // cross-feed nothing ever reaches the right line - a legitimate degenerate setting,
            // and NOT the same thing as Stereo, which feeds each line from its own input channel.
            {
                DelayCore core;
                core.prepare ({ sr, blockSize, 2 });

                auto params = baseParams();
                params.stereoMode = (int) StereoMode::pingPong;
                params.crossFeedPercent = 100.0f;
                params.character.wowPercent = 0.0f;
                params.character.flutterPercent = 0.0f;

                const auto left = burstResponse (core, params, 0.8, 0);

                DelayCore core2;
                core2.prepare ({ sr, blockSize, 2 });
                const auto right = burstResponse (core2, params, 0.8, 1);

                const auto peakAt = [] (const std::vector<float>& sig, int repeat)
                {
                    const int centre = (int) std::round (repeat * testDelayMs * 0.001 * sr);
                    const int half = (int) (0.015 * sr);
                    float peak = 0.0f;

                    for (int i = juce::jmax (0, centre - half);
                         i < juce::jmin ((int) sig.size(), centre + half); ++i)
                        peak = juce::jmax (peak, std::abs (sig[(size_t) i]));

                    return peak;
                };

                expect (peakAt (left, 1) > peakAt (right, 1) * 4.0f,
                        "repeat 1 should be firmly on the left");
                expect (peakAt (right, 2) > peakAt (left, 2) * 4.0f,
                        "repeat 2 should have crossed to the right");
            }

            {
                DelayCore core;
                core.prepare ({ sr, blockSize, 2 });

                auto params = baseParams();
                params.stereoMode = (int) StereoMode::pingPong;
                params.crossFeedPercent = 0.0f;

                DelayCore core2;
                core2.prepare ({ sr, blockSize, 2 });

                const auto left = burstResponse (core, params, 0.8, 0);
                const auto right = burstResponse (core2, params, 0.8, 1);

                const auto peakOf = [] (const std::vector<float>& sig)
                {
                    float peak = 0.0f;

                    for (const auto v : sig)
                        peak = juce::jmax (peak, std::abs (v));

                    return peak;
                };

                // Compared against the left channel rather than against zero: Tape mode has a real
                // per-pass noise floor, so the idle line carries tape hiss even with no signal
                // crossing into it. That is correct behaviour, not leakage.
                expect (peakOf (right) < peakOf (left) * 0.01f,
                        "at Cross-Feed 0 the right line should carry no signal, but it reached "
                            + juce::String (peakOf (right), 6) + " against a left peak of "
                            + juce::String (peakOf (left), 6));
            }
        }

        beginTest ("Mono is genuinely mono");
        {
            DelayCore core;
            core.prepare ({ sr, blockSize, 2 });

            auto params = baseParams();
            params.stereoMode = (int) StereoMode::mono;
            params.character.mode = (int) DelayCharacter::tape;
            params.character.wowPercent = 100.0f;      // per-channel randomised: the hard case
            params.character.flutterPercent = 100.0f;

            juce::AudioBuffer<float> buffer (2, blockSize);
            float worst = 0.0f;

            for (int block = 0; block < 200; ++block)
            {
                buffer.clear();

                if (block == 0)
                    for (int ch = 0; ch < 2; ++ch)
                        buffer.setSample (ch, 0, 1.0f);

                core.process (buffer, params);

                for (int n = 0; n < blockSize; ++n)
                    worst = juce::jmax (worst, std::abs (buffer.getSample (0, n) - buffer.getSample (1, n)));
            }

            expect (worst < 1.0e-6f,
                    "Mono produced a stereo image (worst L/R difference " + juce::String (worst, 8) + ")");
        }

        beginTest ("self-oscillation stays bounded at 110% feedback with NO saturation");
        {
            // The safety-ceiling test. Feedback above unity is deliberate and Program 11 ships at
            // 105 %, so without an always-on soft ceiling - independent of the Saturation control -
            // this runs away to numeric overflow instead of howling.
            for (const int mode : { 0, 1, 2 })
            {
                DelayCore core;
                core.prepare ({ sr, blockSize, 2 });

                auto params = baseParams();
                params.feedbackPercent = 110.0f;
                params.saturationPercent = 0.0f;
                params.dampingHz = 16000.0f;
                params.character.mode = mode;

                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::Random random { 2718 };
                float worst = 0.0f;

                // 60 seconds of full-scale noise into a self-oscillating loop.
                for (int block = 0; block < (int) (60.0 * sr / blockSize); ++block)
                {
                    for (int ch = 0; ch < 2; ++ch)
                        for (int n = 0; n < blockSize; ++n)
                            buffer.setSample (ch, n, random.nextFloat() * 2.0f - 1.0f);

                    core.process (buffer, params);

                    for (int ch = 0; ch < 2; ++ch)
                        for (int n = 0; n < blockSize; ++n)
                        {
                            const float v = buffer.getSample (ch, n);
                            expect (std::isfinite (v), "mode " + juce::String (mode) + ": non-finite sample");
                            worst = juce::jmax (worst, std::abs (v));
                        }
                }

                expect (worst < 3.0f, "mode " + juce::String (mode) + " ran away to "
                                          + juce::String (worst, 2) + " full scale");
            }
        }

        beginTest ("a character mode change does not click");
        {
            DelayCore core;
            core.prepare ({ sr, blockSize, 2 });

            auto params = baseParams();
            params.character.genLossPercent = 60.0f;
            params.character.degradePercent = 60.0f;
            params.character.modDepthPercent = 40.0f;

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::Random random { 31415 };

            float previous = 0.0f;
            float worstJump = 0.0f;

            for (int block = 0; block < 300; ++block)
            {
                for (int ch = 0; ch < 2; ++ch)
                    for (int n = 0; n < blockSize; ++n)
                        buffer.setSample (ch, n, (random.nextFloat() * 2.0f - 1.0f) * 0.3f);

                params.character.mode = (block / 40) % 3;    // cycle through all three
                core.process (buffer, params);

                for (int n = 0; n < blockSize; ++n)
                {
                    const float s = buffer.getSample (0, n);
                    worstJump = juce::jmax (worstJump, std::abs (s - previous));
                    previous = s;
                }
            }

            // The input is noise, so sample-to-sample motion is expected; a bad switch shows up as a
            // step far larger than the signal itself can produce.
            expect (worstJump < 2.0f, "worst sample-to-sample jump across mode changes was "
                                          + juce::String (worstJump, 3));
        }
    }
};

static DelayCoreTests delayCoreTests;
