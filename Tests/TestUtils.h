#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "../Source/Parameters.h"

/** A bare AudioProcessor that exists only to own a real APVTS built from Fifth Member's actual
    parameter layout, so anything manipulating parameters can be tested against the parameter set
    the plugin ships without dragging in the DSP or the GUI. */
class TestHostProcessor final : public juce::AudioProcessor
{
public:
    TestHostProcessor()
        : AudioProcessor (BusesProperties()
                              .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                              .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
          apvts (*this, nullptr, "PARAMETERS", createFifthMemberParameterLayout())
    {
    }

    juce::AudioProcessorValueTreeState apvts;

    float plain (const char* id) const
    {
        if (const auto* p = apvts.getParameter (id))
            return p->convertFrom0to1 (p->getValue());
        return 0.0f;
    }

    void setPlain (const char* id, float value)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    }

    const juce::String getName() const override { return "FifthMemberTestHost"; }
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
};

/** A scratch directory that deletes itself, so no test writes into the user's real Programs
    folder. */
struct ScopedTestDirectory
{
    explicit ScopedTestDirectory (const juce::String& name)
        : directory (juce::File::getSpecialLocation (juce::File::tempDirectory)
                         .getChildFile ("FifthMemberTests").getChildFile (name))
    {
        directory.deleteRecursively();
        directory.createDirectory();
    }

    ~ScopedTestDirectory() { directory.deleteRecursively(); }

    juce::File directory;
};
