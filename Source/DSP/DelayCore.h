#pragma once

#include "CharacterEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include <array>

struct DelayCoreParams
{
    float delayMs = 375.0f;
    float feedbackPercent = 35.0f;      // 0-110; above 100 is self-oscillation, by design
    int   stereoMode = 2;               // 0 Mono, 1 Stereo, 2 Ping-Pong
    float crossFeedPercent = 80.0f;
    float dampingHz = 6000.0f;
    float saturationPercent = 15.0f;
    CharacterParams character;
};

/**
    The delay line and its feedback loop.

    Owns the two delay buffers and everything that lives inside the recirculation path: the
    CharacterEngine, the damping filter and the saturator. They are members rather than stages the
    processor chains around this class because that is precisely what makes degradation compound -
    each one is applied once per pass, so repeat six has been through all of them six times. Chain
    them outside the loop instead and every repeat gets an identical, fixed amount.

    Routing:
      - **Mono** - the input sum drives one line; the result is copied to both outputs. Genuinely
        mono, rather than two lines fed the same signal: the character engine's wow and flutter are
        per-channel randomised, so two lines would drift apart and produce a stereo image.
      - **Stereo** - two independent lines, feedback within each channel, no cross-coupling.
      - **Ping-Pong** - the input sum is injected into the left line only, which is what makes the
        first repeat land on one side and the rest alternate. Cross-Feed `x` sets how much of each
        line's output crosses to the other rather than feeding itself:
            fbL = feedback x ((1-x)*dL + x*dR)
            fbR = feedback x ((1-x)*dR + x*dL)
        At x = 1 each line feeds only the other, giving full L,R,L,R alternation - which is why the
        default is 80 %. At x = 0 nothing ever crosses, so the left line self-feeds and the right
        stays silent; that is a legitimate degenerate setting, not Stereo. (Stereo differs
        structurally: it feeds both lines from their own input channel.) Cross-Feed does nothing
        outside Ping-Pong, which is why its LED is the conditional one.
*/
class DelayCore
{
public:
    /*  **`initial` is where the three smoothers START, and `reset (rate, seconds)` does not set
        one.** It is `setCurrentAndTargetValue (this->target)` internally — it sets the ramp LENGTH
        and snaps to whatever target the smoother last held, zero on a constructed object. So delay
        time, feedback and cross-feed all glided up from nothing across the first block of an
        instance's first playback: a delay whose first repeat is at the wrong time, with the wrong
        feedback, on the wrong side.

        The whole `DelayCoreParams` rather than three floats, because that is what `process` already
        takes — three separate arguments would be three chances for the caller to convert one
        differently here than it does there. */
    void prepare (const juce::dsp::ProcessSpec& spec, const DelayCoreParams& initial);
    void reset();

    /** Replaces the buffer's contents with the wet signal. The caller keeps its own dry copy. */
    void process (juce::AudioBuffer<float>& buffer, const DelayCoreParams& params);

private:
    // Shared by prepare and process so the two cannot clamp differently — see their definitions.
    float delaySamplesFor (float delayMs) const noexcept;
    static float feedbackGainFor (float feedbackPercent) noexcept;
    static float crossGainFor (float crossFeedPercent) noexcept;

public:

    /** Broadband loop gain currently being applied per recirculation - feedback times the character
        engine's own loss. Drives the scope's pulse decay, so the display tracks the real loop
        rather than re-deriving it from a parallel simulation. */
    float getPerPassGain() const noexcept { return perPassGain; }

private:
    static constexpr int maxChannels = 2;

    /** The delay never runs at unity or above without something bounding it. Feedback reaches
        110 % by design and Program 11 ships at 105 %, so without this a self-oscillating patch at
        Saturation 0 % has nothing limiting it and runs away to numeric overflow instead of howling.
        Soft, and high enough to be transparent at ordinary levels. */
    static constexpr float safetyCeiling = 1.4f;

    float saturate (float x, float drive, float amount) const noexcept;

    double sampleRate = 44100.0;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> line { 1 };
    CharacterEngine character;

    std::array<float, maxChannels> dampingState {};
    float dampingCoeff = 1.0f;

    juce::SmoothedValue<float> delaySamplesSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> crossFeedSmoothed;

    float perPassGain = 0.35f;
};
