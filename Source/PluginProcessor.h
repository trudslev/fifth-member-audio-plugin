#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <nf/UserEditGate.h>
#include <juce_dsp/juce_dsp.h>

#include "Parameters.h"
#include "DSP/DelayCore.h"
#include "DSP/ProgramManager.h"
#include "DSP/TimingEngine.h"

/**
    Fifth Member - a tempo-synced stereo delay, model DL-88. The fifth Neon Foundry casting.

    Signal chain (fixed order, all in processBlock):

        in --+------------------------------- dry ----------------------------+
             |                                                                 |
             |  TimingEngine -> delayMs                                        |
             |         |                                                       |
             +-> DelayCore  [ read -> CharacterEngine -> Damping -> Sat ] -----+-> wet
                      ^                                              |
                      +--- feedback, routed per Stereo Mode ---------+
                                                                      |
                                    Mix ------------------------------+-> Trim -> out

    Everything that shapes character lives inside DelayCore's feedback loop, which is what makes
    degradation compound: repeat six has been through the filter, the saturator and the character
    engine six times over. Put those stages on the output instead and every repeat gets an
    identical fixed dose.
*/
class FifthMemberAudioProcessor final : public juce::AudioProcessor
{
public:
    FifthMemberAudioProcessor();
    ~FifthMemberAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return NF_PRODUCT_NAME; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    /** The longest tail a 2 s delay at self-oscillating feedback can plausibly leave behind. Not a
        measurement - hosts use it for freeze/bounce trailing silence. */
    double getTailLengthSeconds() const override { return 12.0; }

    //==============================================================================
    //==============================================================================
    /** **The host adapter - the ONLY place a Program is addressed by position.**

        **The list is the Factory bank and nothing else.** juce_AudioProcessor.h documents
        getNumPrograms as "The value returned must be valid as soon as this object is created, and
        must not change over its lifetime"; a count including User Programs changed on every save.
        JUCE's VST3 wrapper builds the automatable Program parameter ONCE from this value, so a
        Program saved afterwards was unreachable from the host - the API keeping its documented
        promise, not a bug. Excluding INIT too means host index n IS Factory Program n+1.

        **Accepted divergence.** getCurrentProgram answers 0 while a User Program is loaded, so a
        host's menu shows a Factory name while the panel shows the user's Program. Sound and panel
        are both correct; only the host's own menu is wrong. */
    int getNumPrograms() override { return programManager.getNumPrograms(); }
    int getCurrentProgram() override { return programManager.getCurrentFactoryPosition(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override { return programManager.getProgramName (index); }

    /** Renaming in place would be an overwrite by another name, and there is deliberately no
        overwrite path - Save always creates a new Program. */
    /** Deliberately a no-op: with Factory-only exposure nothing on the host's list can be renamed.
        Implementing it would be a back door into the Factory bank. */
    void changeProgramName (int, const juce::String&) override {}

    ProgramManager& getProgramManager() noexcept { return programManager; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** **Guards a host replaying a stale program index over a just-restored session.** Armed by
        setStateInformation, consumed by the next setCurrentProgram (which ignores it only when the
        index matches what getCurrentProgram already reports — the shape of a replay), disarmed by
        the first USER-originated edit. **Automation must not disarm it**: a host may write
        automation on load before replaying, and that would reopen the hole.

        Public because the editor hands it to `nf::connectUserEdit` for every control, which is the
        point of it living in core: Reflect-84 once shipped this guard with zero call sites for its
        disarm, and coupling the disarm to the LCD hand-off is what makes that omission
        inexpressible. See nf/UserEditGate.h. */
    nf::UserEditGate userEdits;

    juce::AudioProcessorValueTreeState apvts;

    // GUI-facing derived display state. Plain relaxed atomics polled by the editor's timers - the
    // house pattern across every sibling.
    float getInputMeterDb()  const noexcept { return inputMeterDb.load  (std::memory_order_relaxed); }
    float getOutputMeterDb() const noexcept { return outputMeterDb.load (std::memory_order_relaxed); }

    /** What the delay is actually doing, for the repeat-timeline scope. */
    float  getDelayMs()      const noexcept { return displayDelayMs.load (std::memory_order_relaxed); }
    double getBpm()          const noexcept { return displayBpm.load (std::memory_order_relaxed); }
    bool   isHostTempoValid()const noexcept { return displayTempoValid.load (std::memory_order_relaxed); }

    /** The loop gain being applied per recirculation - feedback times the character engine's own
        loss. design/README.md asks that the scope be driven from the delay line rather than
        re-simulating decay in the UI; this is that number. */
    float getPerPassGain()   const noexcept { return displayPerPassGain.load (std::memory_order_relaxed); }

private:

    ProgramManager programManager { apvts };

    TimingEngine timingEngine;
    DelayCore delayCore;

    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float> mixSmoothed;
    juce::SmoothedValue<float> trimSmoothed;

    std::atomic<float> inputMeterDb  { -100.0f };
    std::atomic<float> outputMeterDb { -100.0f };
    std::atomic<float> displayDelayMs { 375.0f };
    std::atomic<double> displayBpm { 120.0 };
    std::atomic<bool> displayTempoValid { false };
    std::atomic<float> displayPerPassGain { 0.35f };

    double displaySampleRate = 44100.0;

    std::atomic<float>* rawParam (const char* id) { return apvts.getRawParameterValue (id); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FifthMemberAudioProcessor)
};
