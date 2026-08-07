#include "DelayCore.h"

#include "../Parameters.h"

#include <cmath>

namespace
{
    float onePoleCoeff (float hz, double sampleRate) noexcept
    {
        const float c = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * hz / (float) sampleRate);
        return juce::jlimit (0.0f, 1.0f, c);
    }

    /** Interpolation margin. The read position is delayMs plus a signed character offset, and both
        ends of that excursion have to land on genuinely written samples. */
    constexpr float readMarginSamples = 8.0f;
}

//==============================================================================
void DelayCore::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const int maxSamples = (int) std::ceil (Timing::delayBufferMs * 0.001 * sampleRate)
                         + (int) readMarginSamples * 2;

    line.setMaximumDelayInSamples (maxSamples);

    juce::dsp::ProcessSpec lineSpec = spec;
    lineSpec.numChannels = (juce::uint32) maxChannels;
    line.prepare (lineSpec);

    character.prepare (spec);

    // The delay time is smoothed per sample rather than stepped per block. A tempo change, a
    // division change or a Time sweep therefore glides, producing the tape-style pitch slur a
    // delay is expected to make as it re-times. Stepping it per block would click instead.
    delaySamplesSmoothed.reset (sampleRate, 0.06);
    feedbackSmoothed.reset (sampleRate, 0.02);
    crossFeedSmoothed.reset (sampleRate, 0.02);

    reset();
}

void DelayCore::reset()
{
    line.reset();
    character.reset();
    dampingState.fill (0.0f);
}

float DelayCore::saturate (float x, float drive, float amount) const noexcept
{
    // tanh(x*d)/d, NOT tanh(x*d)/tanh(d). The normalised form is unity only at full scale: below
    // it, the gain is d/tanh(d), which at drive 7 is a factor of 7. Inside a feedback loop that
    // silently multiplies the feedback coefficient and a nominally 80 % patch runs away.
    // Dividing by the drive instead is unity for small signals at any drive, which is the property
    // a recirculating path needs.
    const float driven = std::tanh (x * drive) / drive;

    // Blended so Saturation at 0 % is an exact bypass rather than "nearly transparent".
    const float wet = x * (1.0f - amount) + driven * amount;

    // The always-on ceiling, applied after the user's saturation so it is a floor of protection
    // rather than something the Saturation control can switch off.
    return safetyCeiling * std::tanh (wet / safetyCeiling);
}

//==============================================================================
void DelayCore::process (juce::AudioBuffer<float>& buffer, const DelayCoreParams& params)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (maxChannels, buffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    character.setParams (params.character);

    dampingCoeff = onePoleCoeff (juce::jlimit (200.0f, 20000.0f, params.dampingHz), sampleRate);

    const float feedback = juce::jlimit (0.0f, 1.1f, params.feedbackPercent * 0.01f);
    const float cross = juce::jlimit (0.0f, 1.0f, params.crossFeedPercent * 0.01f);

    const float targetSamples = juce::jlimit (readMarginSamples,
                                              (float) line.getMaximumDelayInSamples() - readMarginSamples,
                                              params.delayMs * 0.001f * (float) sampleRate);
    delaySamplesSmoothed.setTargetValue (targetSamples);
    feedbackSmoothed.setTargetValue (feedback);
    crossFeedSmoothed.setTargetValue (cross);

    const float sat01 = juce::jlimit (0.0f, 1.0f, params.saturationPercent * 0.01f);
    const float drive = 1.0f + sat01 * 6.0f;

    const auto mode = (StereoMode) juce::jlimit (0, numStereoModes - 1, params.stereoMode);
    const bool isMono = mode == StereoMode::mono;
    const bool isPingPong = mode == StereoMode::pingPong;

    std::array<float*, maxChannels> out {};

    for (int ch = 0; ch < numChannels; ++ch)
        out[(size_t) ch] = buffer.getWritePointer (ch);

    // Mono collapses to one line: the character engine's wow and flutter are per-channel
    // randomised, so running two lines on the same input would drift apart into a stereo image.
    const int activeChannels = isMono ? 1 : numChannels;

    for (int n = 0; n < numSamples; ++n)
    {
        const float baseSamples = delaySamplesSmoothed.getNextValue();
        const float fb = feedbackSmoothed.getNextValue();
        const float x = crossFeedSmoothed.getNextValue();

        const float inL = out[0][n];
        const float inR = numChannels > 1 ? out[(size_t) 1][n] : inL;
        const float monoIn = numChannels > 1 ? (inL + inR) * 0.5f : inL;

        std::array<float, maxChannels> delayed {};

        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const size_t c = (size_t) ch;

            const float offsetSamples = character.nextOffsetMs (ch) * 0.001f * (float) sampleRate;
            const float readSamples = juce::jlimit (readMarginSamples,
                                                    (float) line.getMaximumDelayInSamples() - readMarginSamples,
                                                    baseSamples + offsetSamples);

            float d = line.popSample (ch, readSamples, true);

            // Everything below is inside the loop, so it is applied once per recirculation.
            d = character.processSample (ch, d);

            dampingState[c] += dampingCoeff * (d - dampingState[c]);
            d = dampingState[c];

            d = saturate (d, drive, sat01);

            delayed[c] = d;
        }

        if (isMono)
        {
            line.pushSample (0, monoIn + delayed[0] * fb);

            for (int ch = 0; ch < numChannels; ++ch)
                out[(size_t) ch][n] = delayed[0];
        }
        else
        {
            const float dL = delayed[0];
            const float dR = numChannels > 1 ? delayed[1] : delayed[0];

            // At cross = 0 this is exactly Stereo; at cross = 1 each line feeds only the other.
            const float fbL = isPingPong ? fb * ((1.0f - x) * dL + x * dR) : fb * dL;
            const float fbR = isPingPong ? fb * ((1.0f - x) * dR + x * dL) : fb * dR;

            // Ping-Pong drives the left line only, so the first repeat lands on one side and the
            // repeats alternate from there.
            line.pushSample (0, (isPingPong ? monoIn : inL) + fbL);

            if (numChannels > 1)
                line.pushSample (1, (isPingPong ? 0.0f : inR) + fbR);

            out[0][n] = dL;

            if (numChannels > 1)
                out[(size_t) 1][n] = dR;
        }
    }

    perPassGain = juce::jlimit (0.0f, 1.5f, feedback * character.getPerPassGain());
}
