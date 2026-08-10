#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FactoryPrograms.h"

#include <functional>
#include <vector>

/**
    Program storage: a read-only factory bank plus a user bank on disk.

    The architecture is Gatecrasher's, reused directly - Factory/User banks, one XML file per User
    Program, Save always creates new and never overwrites, Delete gated to User Programs, an
    AsyncUpdater apply path because a host can call setCurrentProgram from the audio thread while
    applying requires the message thread. Terminology is Programs, never Presets, per BRAND.md.

    What is NOT Gatecrasher's is the data model. Per Correction 1, a Program stores only the
    parameters on its **active path**, defined once in ActivePath (Parameters.h). Everything else -
    the inactive timing control, the two non-selected character modes, and Cross-Feed outside
    Ping-Pong - is left completely untouched by a Program change and persists independently, the way
    a physical knob keeps its position regardless of which patch is recalled.

    There is no parameter that is *never* Program state. Cross-Feed was one until it turned out to
    be reachable: Stereo Mode is always stored, so a Program recalls Ping-Pong - the only mode whose
    DelayCore branch reads the cross term - and at cross 0 that mode leaves the right line silent.
    A Program could name a bounce it had no way to reproduce. It is conditionally stored now, on
    Stereo Mode, alongside the other two conditional axes.

    Three consequences that are easy to get wrong, and are each covered by a test:

    1. Applying a Program writes only its own subset. For User Programs that means it cannot use
       `apvts.replaceState()` the way Gatecrasher does - that would clobber every persisting
       parameter. User Programs serialise the same filtered subset and are applied attribute by
       attribute.

    2. The dirty check compares only the active path. Otherwise moving Cross-Feed lights SAVE, and
       saving then produces a Program indistinguishable from the one already loaded. The active path
       is recomputed from the live Sync/Character state each time, because flipping either changes
       which parameters are being compared.

    3. The session state (getStateInformation) still persists the whole APVTS. Session state and
       Program state are different things: the persisting knobs must survive a reload even though
       no Program owns them.
*/
class ProgramManager final : private juce::AsyncUpdater
{
public:
    explicit ProgramManager (juce::AudioProcessorValueTreeState& state,
                             juce::File userDirectoryOverride = {});
    ~ProgramManager() override;

    void initialise();

    //==============================================================================
    int getNumPrograms() const;
    juce::String getProgramName (int index) const;

    static bool isFactoryProgram (int index) noexcept
    {
        return index >= 0 && index < kNumFactoryPrograms;
    }

    int getCurrentProgram() const noexcept
    {
        return currentProgramIndex.load (std::memory_order_relaxed);
    }

    /** Safe from any thread; defers the apply to the message thread. */
    void requestProgramChange (int index);

    /** Drops a deferred change. setStateInformation MUST call this before restoring, or a request
        that arrived just beforehand lands afterwards and overwrites the restored session. */
    void cancelPendingChange();

    /** Applies a deferred change immediately. Only the tests need this - the console app they run
        in has no message loop to deliver the async callback. */
    void flushPendingChange() { handleUpdateNowIfNeeded(); }

    void setCurrentProgramIndexWithoutApplying (int index);

    //==============================================================================
    void saveNewUserProgram (const juce::String& requestedName);
    void deleteUserProgram (int index);

    /** True once any ACTIVE-PATH parameter differs from the loaded Program. Moving a parameter no
        Program stores - Cross-Feed, the inactive timing control, another mode's dials - is
        deliberately not a modification. */
    bool isModifiedFromLoadedProgram() const;

    std::function<void()> onProgramListChanged;

    juce::File getUserProgramDirectory() const;
    static juce::File getDefaultUserProgramDirectory();
    static juce::String getProgramFileExtension() { return ".fifthmemberprogram"; }

    static constexpr int maxProgramNameLength = 26;   // section 6.2's measured cell budget

private:
    void handleAsyncUpdate() override;

    void refreshUserProgramList();
    void applyProgramByIndex (int index);
    void applyFactoryProgram (const FactoryProgram& program);
    void captureCleanSnapshot();

    /** The active-path IDs for the CURRENT parameter state. */
    std::vector<const char*> currentActivePath() const;

    juce::AudioProcessorValueTreeState& apvts;

    const juce::File userDirectory;
    juce::Array<juce::File> userProgramFiles;

    std::atomic<int> currentProgramIndex { defaultFactoryProgramIndex };
    std::atomic<int> pendingProgramIndex { -1 };

    /** Normalised values keyed by parameter ID - not a positional vector, because the set being
        compared changes with the selectors. Message-thread only. */
    std::map<juce::String, float> cleanSnapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManager)
};
