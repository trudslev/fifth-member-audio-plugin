#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <nf/BlockChunking.h>

#include <cmath>

FifthMemberAudioProcessor::FifthMemberAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createFifthMemberParameterLayout())
{
    programManager.onProgramListChanged = [this]
    {
        updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
    };

    programManager.initialise();
}

//==============================================================================
void FifthMemberAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    displaySampleRate = sampleRate;

    const juce::dsp::ProcessSpec spec {
        sampleRate,
        (juce::uint32) juce::jmax (1, samplesPerBlock),
        (juce::uint32) juce::jmax (1, getTotalNumOutputChannels())
    };

    timingEngine.prepare (spec);
    delayCore.prepare (spec);

    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, true);

    mixSmoothed.reset (sampleRate, 0.02);
    trimSmoothed.reset (sampleRate, 0.02);
    mixSmoothed.setCurrentAndTargetValue (rawParam (ParamIDs::mix)->load() * 0.01f);
    trimSmoothed.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (rawParam (ParamIDs::outputTrim)->load()));

    inputMeterDb.store (-100.0f, std::memory_order_relaxed);
    outputMeterDb.store (-100.0f, std::memory_order_relaxed);
}

//==============================================================================
/** A host's reset - a transport locate, a buffer clear - propagated to the DSP.

    **JUCE's base implementation is a no-op, and none of the six castings overrode it**, so until
    stage 1c a host asking every plugin in the session to clear itself was answered by nothing
    anywhere. Measured tails surviving a reset: Gatecrasher 0.679, Chorus-60 0.429, Reflect-84 0.111.

    Routed to the same per-stage `reset()` calls `prepareToPlay` already makes, and deliberately NOT
    to `prepareToPlay` itself: re-preparing would also re-run whatever a prepare re-arms, and this
    suite has a measured example of that being audible.
*/
void FifthMemberAudioProcessor::reset()
{
    timingEngine.reset();
    delayCore.reset();
}

bool FifthMemberAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == out;
}

//==============================================================================
void FifthMemberAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (getTotalNumInputChannels(), getTotalNumOutputChannels());

    for (int ch = numChannels; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (numChannels == 0 || numSamples == 0)
        return;

    // Every parameter is read exactly once, here, into plain locals. No DSP class touches the APVTS.
    const bool  sync       = rawParam (ParamIDs::sync)->load() > 0.5f;
    const int   division   = juce::roundToInt (rawParam (ParamIDs::noteDivision)->load());
    const float timeMs     = rawParam (ParamIDs::timeMs)->load();
    const float mix01      = rawParam (ParamIDs::mix)->load() * 0.01f;
    const float trimGain   = juce::Decibels::decibelsToGain (rawParam (ParamIDs::outputTrim)->load());

    const auto timing = timingEngine.update (getPlayHead(), sync, division, timeMs, numSamples);

    DelayCoreParams delayParams;
    delayParams.delayMs           = timing.delayMs;
    delayParams.feedbackPercent   = rawParam (ParamIDs::feedback)->load();
    delayParams.stereoMode        = juce::roundToInt (rawParam (ParamIDs::stereoMode)->load());
    delayParams.crossFeedPercent  = rawParam (ParamIDs::crossFeed)->load();
    delayParams.dampingHz         = rawParam (ParamIDs::damping)->load();
    delayParams.saturationPercent = rawParam (ParamIDs::saturation)->load();

    delayParams.character.mode            = juce::roundToInt (rawParam (ParamIDs::character)->load());
    delayParams.character.wowPercent      = rawParam (ParamIDs::wow)->load();
    delayParams.character.flutterPercent  = rawParam (ParamIDs::flutter)->load();
    delayParams.character.genLossPercent  = rawParam (ParamIDs::genLoss)->load();
    delayParams.character.modRateHz       = rawParam (ParamIDs::modRate)->load();
    delayParams.character.modDepthPercent = rawParam (ParamIDs::modDepth)->load();
    delayParams.character.degradePercent  = rawParam (ParamIDs::degrade)->load();

    // **The over-delivery policy.** dryBuffer.setSize grows when a host sends more samples than it
    // declared, which is a heap allocation on the audio thread. Chunking removes it: no span is
    // longer than the prepared size, so there is nothing left to grow.
    //
    // **THE BUS QUESTION, ASKED HERE RATHER THAN INHERITED FROM GATECRASHER.** Gatecrasher had to
    // move its getBusBuffer calls inside the loop, because asking once outside hands every span the
    // whole block's length and quietly undoes the chunking while every assertion still passes.
    // **That does not arise here, and for a stated reason rather than because nobody looked:** this
    // casting has plain stereo in and out with no second bus, so there is no getBusBuffer call
    // anywhere in processBlock. Its channel count comes from getTotalNumInput/OutputChannels — the
    // processor, not the buffer — which is span-invariant by construction.
    //
    // ScopedNoDenormals, the unused-channel clear and the parameter reads all stay OUTSIDE. The
    // guard is scoped; the clear operates on the whole buffer; the parameter reads are per block by
    // design, and moving them in would be a behaviour change smuggled in with a safety fix.
    nf::processInChunks (buffer, getBlockSize(), [&] (juce::AudioBuffer<float>& span)
    {
    const int numSamples = span.getNumSamples();

    dryBuffer.setSize (numChannels, numSamples, false, false, true);

    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom (ch, 0, span, ch, 0, numSamples);

    delayCore.process (span, delayParams);

    mixSmoothed.setTargetValue (mix01);
    trimSmoothed.setTargetValue (trimGain);

    std::array<float*, 2> out {};
    std::array<const float*, 2> dry {};

    for (int ch = 0; ch < numChannels; ++ch)
    {
        out[(size_t) ch] = span.getWritePointer (ch);
        dry[(size_t) ch] = dryBuffer.getReadPointer (ch);
    }

    for (int n = 0; n < numSamples; ++n)
    {
        // One call per sample, shared across channels - calling getNextValue per channel would
        // advance the smoother twice per sample.
        const float wet = mixSmoothed.getNextValue();
        const float gain = trimSmoothed.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            out[(size_t) ch][n] = (dry[(size_t) ch][n] * (1.0f - wet) + out[(size_t) ch][n] * wet) * gain;
    }

    // --- display state -------------------------------------------------------
    displayDelayMs.store (timing.delayMs, std::memory_order_relaxed);
    displayBpm.store (timing.bpm, std::memory_order_relaxed);
    displayTempoValid.store (timing.hostTempoValid, std::memory_order_relaxed);
    displayPerPassGain.store (delayCore.getPerPassGain(), std::memory_order_relaxed);

    // Meter ballistics derived from a real time constant and the actual block length, so they
    // behave identically whatever buffer size the host uses.
    constexpr float releaseSeconds = 0.15f;
    const float blockSeconds = (float) numSamples / (float) displaySampleRate;
    const float releaseCoeff = 1.0f - std::exp (-blockSeconds / releaseSeconds);

    const auto peakDb = [numSamples] (const juce::AudioBuffer<float>& b)
    {
        float peak = 0.0f;

        for (int ch = 0; ch < b.getNumChannels(); ++ch)
        {
            const auto* data = b.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
                peak = juce::jmax (peak, std::abs (data[i]));
        }

        return juce::Decibels::gainToDecibels (peak, -100.0f);
    };

    const auto smoothDb = [releaseCoeff] (std::atomic<float>& target, float measured)
    {
        const float current = target.load (std::memory_order_relaxed);
        target.store (measured > current ? measured : current + releaseCoeff * (measured - current),
                      std::memory_order_relaxed);
    };

    smoothDb (inputMeterDb, peakDb (dryBuffer));
    smoothDb (outputMeterDb, peakDb (span));
    });
}

//==============================================================================
juce::AudioProcessorEditor* FifthMemberAudioProcessor::createEditor()
{
    return new FifthMemberAudioProcessorEditor (*this);
}

//==============================================================================
void FifthMemberAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, programManager.getNumPrograms()))
        return;

    // The stale-replay guard, disarmed by this call whether or not it is honoured.
    if (userEdits.consumeRestore() && index == getCurrentProgram())
        return;

    programManager.requestProgramChange (ProgramManager::factoryIdAt (index));
}

void FifthMemberAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // The WHOLE APVTS, not just the active path. Session state and Program state are different
    // things: Cross-Feed, the inactive timing control and the two non-selected character modes all
    // have to survive a session reload even though no Program owns them.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());

    xml->setAttribute (LegacyMigration::stateSchemaVersionAttribute,
                       LegacyMigration::currentStateSchemaVersion);
    // **The bank, the identifier, and the full parameter state.** The values make the session
    // sound right; the identity only decides what the panel CALLS them.
    const auto id = programManager.getCurrentProgramId();
    xml->setAttribute (LegacyMigration::programBankAttribute, LegacyMigration::bankAttributeValue (id.bank));
    xml->setAttribute (LegacyMigration::programIdAttribute, id.id);
    xml->setAttribute (LegacyMigration::programNameAttribute, id.displayName);

    copyXmlToBinary (*xml, destData);
}

void FifthMemberAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    const int savedSchema = xml->getIntAttribute (LegacyMigration::stateSchemaVersionAttribute, 1);

    // Read, not merely written. Restoring an older layout otherwise leaves surviving IDs at their
    // saved values while new ones fall back to defaults - a silent hybrid nothing reports.
    // **Two branches, both pinned to literals.** Too old: the values cannot be interpreted. Too
    // new: written by a later build, and reading it with today's assumptions would give plausible
    // wrong values rather than an obvious fallback.
    //
    // This replaced `savedSchema != currentStateSchemaVersion`, which discarded schema-1 sessions
    // wholesale even though this plugin's own schema note says schema-1 Programs load fine by
    // design - the two were already inconsistent - and would have discarded schema-2 the same way.
    if (LegacyMigration::classifySchema (savedSchema) != LegacyMigration::SchemaVerdict::readable)
    {
        programManager.cancelPendingChange();
        programManager.requestProgramChange (ProgramManager::factoryIdAt (defaultFactoryProgramIndex));
        return;
    }

    // Essential: a program change requested just before the restore would otherwise be applied just
    // after it and overwrite everything that was restored.
    programManager.cancelPendingChange();

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    ProgramId restored;

    if (savedSchema >= LegacyMigration::identitySchemaVersion)
    {
        restored = programManager.resolve (
            LegacyMigration::bankFromAttribute (
                xml->getStringAttribute (LegacyMigration::programBankAttribute)),
            xml->getStringAttribute (LegacyMigration::programIdAttribute),
            xml->getStringAttribute (LegacyMigration::programNameAttribute));
    }
    else
    {
        // Older readable sessions stored a position. Map it through the CURRENT bank.
        const int savedIndex = xml->getIntAttribute (LegacyMigration::currentProgramIndexAttribute,
                                                      defaultFactoryProgramIndex);

        if (savedIndex == -1)
            restored = ProgramManager::initId();
        else if (juce::isPositiveAndBelow (savedIndex, kNumFactoryPrograms))
            restored = ProgramManager::factoryIdAt (savedIndex);
        else
            restored = ProgramManager::factoryIdAt (defaultFactoryProgramIndex);
    }

    programManager.setCurrentProgramWithoutApplying (restored);

    // **Armed AFTER replaceState**, or the restore's own writes would disarm it.
    userEdits.armRestore();
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FifthMemberAudioProcessor();
}
