#pragma once

#include <juce_dsp/juce_dsp.h>

#include <array>

/** Everything the character engine needs for a block, in physical units. */
struct CharacterParams
{
    int   mode = 0;             // 0 Tape, 1 BBD, 2 Digital

    float wowPercent = 25.0f;
    float flutterPercent = 20.0f;
    float genLossPercent = 30.0f;

    float modRateHz = 0.6f;
    float modDepthPercent = 20.0f;

    float degradePercent = 20.0f;
};

/**
    The Delay Character stage: ONE reconfigurable engine with three parameter sets, not three
    parallel engines behind a selector.

    There is a single modulation source, a single loss filter and a single nonlinearity. What
    changes per mode is how they are configured and which of them is in circuit - the topology is
    shared. That is the whole point: running three engines and listening to one would cost three
    times the CPU to hear a third of it.

    This is called from INSIDE the feedback loop, once per sample per channel, which is what makes
    degradation compound. Repeat six has been through it six times. A delay whose character sits on
    the output applies a fixed amount uniformly and can never produce "repeat six is measurably more
    degraded than repeat one".

    Per mode:

    - **Tape** - Wow and Flutter modulate the read position (0.5 Hz sine blended with LPF'd noise;
      a per-channel randomised 7-12 Hz sine blended with band-passed noise), the shape TapeRot's
      WowFlutter established, but returning an OFFSET rather than owning a delay line - CHORUS-60's
      ModulationEngine is the precedent for that inversion. Generation Loss drives a progressively
      lower loss filter, soft compression, and a small noise floor. All of it compounds, including
      the pitch modulation, which is what real tape does.

    - **BBD** - fixed-character analog-chip modulation: an input pre-filter and a reconstruction
      filter (12 kHz and 7 kHz, the two constants that ARE the BBD character in CHORUS-60's
      BBDDelayLine), plus one LFO on the read position at Mod Rate and Mod Depth. The filters
      compound and darken each repeat; the modulation depth stays fixed, which is what makes it
      "fixed character" rather than progressive.

    - **Digital** - compounding quantisation and aliasing: bit-depth truncation plus sample-and-hold
      decimation with no anti-alias filter, re-applied every recirculation so the artefact accretes.
      Reflect-84's GrainStage is the working precedent for exactly this shape.

    A mode change ramps `modeBlend` over ~40 ms rather than switching on a sample boundary. During
    that ramp both configurations of the shared nonlinearity are evaluated; it is a transient, not a
    steady state, and it is what stops the switch clicking.
*/
class CharacterEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    /** Once per block. Handles the mode-change ramp. */
    void setParams (const CharacterParams& params);

    /** Delay-time offset in milliseconds for this sample, applied to the read position. Advances
        the modulation sources, so it must be called exactly once per sample per channel. */
    float nextOffsetMs (int channel) noexcept;

    /** Processes one sample inside the feedback loop, after the tap read. */
    float processSample (int channel, float x) noexcept;

    /** Broadband amplitude loss this mode applies per recirculation, 0-1. Drives the scope's pulse
        decay so the display tracks what the loop is really doing rather than re-deriving it. */
    float getPerPassGain() const noexcept { return perPassGain; }

private:
    /** The per-channel state the shared topology runs on. Two sets exist so a mode change can
        cross-fade without the outgoing and incoming configurations both advancing the same filter
        memory - that would double-filter for the duration of the ramp. This is a double-buffered
        single engine, not a second engine: only one set is live outside a 40 ms transition. */
    struct ModeState
    {
        float lossLp = 0.0f;
        float reconLp = 0.0f;
        float held = 0.0f;
        int   holdCounter = 0;
    };

    float applyMode (int mode, ModeState& state, int channel, float x) noexcept;

    static constexpr int maxChannels = 2;
    static constexpr float modeBlendMs = 40.0f;

    double sampleRate = 44100.0;

    CharacterParams current;
    int previousMode = 0;
    juce::SmoothedValue<float> modeBlend;

    // --- shared modulation ---------------------------------------------------
    struct ModState
    {
        double wowPhase = 0.0;
        double flutterPhase = 0.0;
        double bbdPhase = 0.0;
        float  wowNoiseLp = 0.0f;
        float  flutterNoiseHp = 0.0f;
        float  flutterNoisePrev = 0.0f;
        float  flutterNoiseLp = 0.0f;
        float  flutterRateHz = 9.0f;
        juce::Random random { 1 };
    };

    std::array<ModState, maxChannels> mod;

    // --- shared filter / nonlinearity state ----------------------------------
    // [channel][buffer]. The topology is one loss filter, one reconstruction filter and one
    // sample-and-hold, reconfigured per mode - not duplicated per mode.
    std::array<std::array<ModeState, 2>, maxChannels> state {};
    int activeState = 0;

    float lossLpCoeff = 1.0f;
    float reconLpCoeff = 1.0f;
    float perPassGain = 1.0f;

    // Digital
    float quantStep = 0.0f;
    int   holdPeriod = 1;

    // Tape
    float wowDepthMs = 0.0f;
    float flutterDepthMs = 0.0f;
    float tapeDrive = 1.0f;
    float tapeNoiseAmount = 0.0f;

    // BBD
    float bbdDepthMs = 0.0f;
};
