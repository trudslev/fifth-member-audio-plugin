#include "CharacterEngine.h"

#include <cmath>

namespace
{
    /** The house seeding convention, so the two channels' random streams decorrelate rather than
        phase-locking. Same golden-ratio constant TapeRot uses. */
    juce::int64 seedFor (int index) noexcept
    {
        return (juce::int64) (0x9E3779B97F4A7C15ULL * (juce::uint64) (index + 1));
    }

    float onePoleCoeff (float hz, double sampleRate) noexcept
    {
        const float c = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * hz / (float) sampleRate);
        return juce::jlimit (0.0f, 1.0f, c);
    }

    // Tape
    constexpr float wowRateHz = 0.5f;
    constexpr float wowNoiseLpHz = 0.2f;
    constexpr float flutterNoiseLpHz = 11.0f;
    constexpr float flutterNoiseHpHz = 7.0f;
    constexpr float maxWowMs = 6.0f;        // scaled for a delay, not TapeRot's 25 ms tape path
    constexpr float maxFlutterMs = 1.2f;

    // BBD - the two constants that are the character, from CHORUS-60's BBDDelayLine.
    constexpr float bbdInputPreFilterHz = 12000.0f;
    constexpr float bbdReconstructionHz = 7000.0f;
    constexpr float maxBbdMs = 3.0f;
}

//==============================================================================
void CharacterEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    for (int ch = 0; ch < maxChannels; ++ch)
    {
        mod[(size_t) ch].random = juce::Random (seedFor (ch));
        // Randomised per channel so the two sides never flutter in lockstep, the way two spots on
        // one piece of tape never do.
        mod[(size_t) ch].flutterRateHz = 7.0f + mod[(size_t) ch].random.nextFloat() * 5.0f;
    }

    modeBlend.reset (sampleRate, modeBlendMs * 0.001);
    modeBlend.setCurrentAndTargetValue (1.0f);

    reset();
}

void CharacterEngine::reset()
{
    for (auto& perChannel : state)
        perChannel.fill (ModeState {});

    activeState = 0;

    for (auto& m : mod)
    {
        m.wowPhase = 0.0;
        m.flutterPhase = 0.0;
        m.bbdPhase = 0.0;
        m.wowNoiseLp = 0.0f;
        m.flutterNoiseHp = 0.0f;
        m.flutterNoisePrev = 0.0f;
        m.flutterNoiseLp = 0.0f;
    }

    modeBlend.setCurrentAndTargetValue (1.0f);
}

//==============================================================================
void CharacterEngine::setParams (const CharacterParams& params)
{
    if (params.mode != current.mode)
    {
        previousMode = current.mode;

        // The incoming configuration takes the other buffer, seeded from the outgoing one so it
        // starts from the signal already in the loop rather than from silence.
        const int incoming = 1 - activeState;

        for (auto& perChannel : state)
            perChannel[(size_t) incoming] = perChannel[(size_t) activeState];

        activeState = incoming;

        modeBlend.setCurrentAndTargetValue (0.0f);
        modeBlend.setTargetValue (1.0f);
    }

    current = params;

    const float wow01 = juce::jlimit (0.0f, 1.0f, params.wowPercent * 0.01f);
    const float flutter01 = juce::jlimit (0.0f, 1.0f, params.flutterPercent * 0.01f);
    const float gen01 = juce::jlimit (0.0f, 1.0f, params.genLossPercent * 0.01f);
    const float depth01 = juce::jlimit (0.0f, 1.0f, params.modDepthPercent * 0.01f);
    const float degrade01 = juce::jlimit (0.0f, 1.0f, params.degradePercent * 0.01f);

    wowDepthMs = wow01 * maxWowMs;
    flutterDepthMs = flutter01 * maxFlutterMs;
    bbdDepthMs = depth01 * maxBbdMs;

    // Generation Loss is a single continuous dial rather than a discrete generation count,
    // precisely because every recirculation is already its own generation - the dial sets how much
    // each one costs. At 100 % the loss filter closes to 1.6 kHz and the drive reaches 3.5x, so six
    // passes are unmistakably sixth-generation.
    const float lossHz = juce::jmap (gen01, 18000.0f, 1600.0f);
    tapeDrive = 1.0f + gen01 * 2.5f;
    tapeNoiseAmount = gen01 * 0.0022f;

    switch (params.mode)
    {
        case 1:   // BBD - fixed character, the filters do the work
            lossLpCoeff = onePoleCoeff (bbdInputPreFilterHz, sampleRate);
            reconLpCoeff = onePoleCoeff (bbdReconstructionHz, sampleRate);
            perPassGain = 0.985f;
            break;

        case 2:   // Digital - compounding quantisation, no filtering
        {
            // Word length falls from ~13 bits to ~6 as Degrade rises; the hold factor decimates the
            // loop's update rate up to 12:1 with no anti-alias filter, so the fold-back is part of
            // the sound rather than something to clean up.
            const float bits = juce::jmap (degrade01, 13.0f, 6.0f);
            quantStep = 2.0f / std::pow (2.0f, bits);
            holdPeriod = juce::jmax (1, juce::roundToInt (1.0f + degrade01 * 11.0f));
            lossLpCoeff = 1.0f;
            reconLpCoeff = 1.0f;
            perPassGain = 1.0f - degrade01 * 0.12f;
            break;
        }

        case 0:   // Tape
        default:
            lossLpCoeff = onePoleCoeff (lossHz, sampleRate);
            reconLpCoeff = 1.0f;
            perPassGain = 1.0f - gen01 * 0.22f;
            break;
    }
}

//==============================================================================
float CharacterEngine::nextOffsetMs (int channel) noexcept
{
    auto& m = mod[(size_t) juce::jlimit (0, maxChannels - 1, channel)];

    // The modulation sources advance regardless of mode, so switching modes never produces a phase
    // jump in whichever one comes back into circuit.
    const float wowSine = std::sin ((float) m.wowPhase);
    const float wowNoiseRaw = m.random.nextFloat() * 2.0f - 1.0f;
    m.wowNoiseLp += onePoleCoeff (wowNoiseLpHz, sampleRate) * (wowNoiseRaw - m.wowNoiseLp);
    const float wow = wowSine * 0.7f + m.wowNoiseLp * 0.3f;

    const float flutterSine = std::sin ((float) m.flutterPhase);
    const float flutterNoiseRaw = m.random.nextFloat() * 2.0f - 1.0f;
    const float hpCoeff = 1.0f - onePoleCoeff (flutterNoiseHpHz, sampleRate);
    m.flutterNoiseHp = hpCoeff * (m.flutterNoiseHp + flutterNoiseRaw - m.flutterNoisePrev);
    m.flutterNoisePrev = flutterNoiseRaw;
    m.flutterNoiseLp += onePoleCoeff (flutterNoiseLpHz, sampleRate) * (m.flutterNoiseHp - m.flutterNoiseLp);
    const float flutter = flutterSine * 0.8f + m.flutterNoiseLp * 0.2f;

    const float bbd = std::sin ((float) m.bbdPhase);

    m.wowPhase += juce::MathConstants<double>::twoPi * wowRateHz / sampleRate;
    m.flutterPhase += juce::MathConstants<double>::twoPi * m.flutterRateHz / sampleRate;
    m.bbdPhase += juce::MathConstants<double>::twoPi * juce::jmax (0.01f, current.modRateHz) / sampleRate;

    if (m.wowPhase > juce::MathConstants<double>::twoPi) m.wowPhase -= juce::MathConstants<double>::twoPi;
    if (m.flutterPhase > juce::MathConstants<double>::twoPi) m.flutterPhase -= juce::MathConstants<double>::twoPi;
    if (m.bbdPhase > juce::MathConstants<double>::twoPi) m.bbdPhase -= juce::MathConstants<double>::twoPi;

    switch (current.mode)
    {
        case 1:  return bbd * bbdDepthMs;
        case 2:  return 0.0f;                                    // Digital does not wobble
        case 0:
        default: return wow * wowDepthMs + flutter * flutterDepthMs;
    }
}

float CharacterEngine::applyMode (int mode, ModeState& s, int channel, float x) noexcept
{
    switch (mode)
    {
        case 1:   // BBD: band-limit going in, reconstruction filter coming out. Both compound.
        {
            s.lossLp += lossLpCoeff * (x - s.lossLp);
            s.reconLp += reconLpCoeff * (s.lossLp - s.reconLp);
            return s.reconLp;
        }

        case 2:   // Digital: sample-and-hold, then truncate. No anti-alias filter, by design.
        {
            if (s.holdCounter == 0)
                s.held = quantStep > 0.0f ? std::trunc (x / quantStep) * quantStep : x;

            if (++s.holdCounter >= holdPeriod)
                s.holdCounter = 0;

            return s.held;
        }

        case 0:   // Tape: loss filter, soft compression, a little noise. All three compound.
        default:
        {
            s.lossLp += lossLpCoeff * (x - s.lossLp);

            // tanh(x*d)/d rather than /tanh(d): the normalised form is unity only at full scale
            // and boosts everything below it, which inside a feedback loop compounds into runaway.
            float y = std::tanh (s.lossLp * tapeDrive) / tapeDrive;

            if (tapeNoiseAmount > 0.0f)
                y += (mod[(size_t) juce::jlimit (0, maxChannels - 1, channel)].random.nextFloat() * 2.0f - 1.0f)
                     * tapeNoiseAmount;

            return y;
        }
    }
}

float CharacterEngine::processSample (int channel, float x) noexcept
{
    const size_t c = (size_t) juce::jlimit (0, maxChannels - 1, channel);
    const float blend = modeBlend.getNextValue();

    float y = applyMode (current.mode, state[c][(size_t) activeState], channel, x);

    if (blend < 1.0f)
    {
        // Mid mode-change. The outgoing configuration runs on its OWN state buffer, so neither
        // advances the other's filter memory - running both against one set would double-filter
        // for the length of the ramp.
        const float previous = applyMode (previousMode, state[c][(size_t) (1 - activeState)], channel, x);
        y = previous * (1.0f - blend) + y * blend;
    }

    return y;
}
