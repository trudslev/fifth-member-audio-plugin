#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <nf/ParameterSnapshot.h>
#include <nf/UserProgramStore.h>

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
    /** The Factory bank's size - what the host is told, and it never changes. */
    int getNumPrograms() const noexcept { return kNumFactoryPrograms; }

    ProgramId getCurrentProgramId() const;
    static ProgramId factoryIdAt (int factoryPosition);
    static ProgramId initId();
    static int factoryPositionOf (const juce::String& slug);

    /** The Factory position of the current Program, or 0 when it is INIT, a User Program or
        unresolved - none of which the host's list contains. */
    int getCurrentFactoryPosition() const;

    ProgramId resolve (ProgramBank bank, const juce::String& id, const juce::String& displayName) const;
    std::vector<ProgramId> listPrograms() const;

    /** **What the LCD and the dropdown print - a label, not a key.** Only Factory Programs get the
        two-digit number, computed from their bank position at paint time. */
    juce::String displayLabelFor (const ProgramId& id) const;
    juce::String getProgramName (int index) const;

    /** INIT sits outside both banks at index -1, so it is neither factory nor user. */

    /** What the LCD and the dropdown show: a two-digit 1-based index, a space, then the name.
        getProgramName stays raw - that is what the HOST's program list wants, since a host renders
        its own numbering and would print "01" twice. INIT is unnumbered in both. */



    /** Safe from any thread; defers the apply to the message thread. */
    void requestProgramChange (const ProgramId& id);

    /** Drops a deferred change. setStateInformation MUST call this before restoring, or a request
        that arrived just beforehand lands afterwards and overwrites the restored session. */
    void cancelPendingChange();

    /** Applies a deferred change immediately. Only the tests need this - the console app they run
        in has no message loop to deliver the async callback. */
    void flushPendingChange() { handleUpdateNowIfNeeded(); }

    void setCurrentProgramWithoutApplying (const ProgramId& id);

    //==============================================================================
    void saveNewUserProgram (const juce::String& requestedName);
    void deleteUserProgram (const ProgramId& id);

    /** True once any ACTIVE-PATH parameter differs from the loaded Program. Moving a parameter no
        Program stores - Cross-Feed, the inactive timing control, another mode's dials - is
        deliberately not a modification. */
    bool isModifiedFromLoadedProgram() const;

    std::function<void()> onProgramListChanged;

    juce::File getUserProgramDirectory() const;
    static juce::File getDefaultUserProgramDirectory();
    static juce::String getProgramFileExtension() { return ".fifthmemberprogram"; }

    /** **26, and it does NOT change - which is the point.**

        Section 6.2 measures the 335.83px name cell at 19px Share Tech Mono / .12em = 26 characters.
        Dropping the "NN " prefix from user names would have allowed 24 = 26 - the dirty marker, and
        that was considered and rejected: **a cap must never contract.** Anyone who has already
        named a Program at 26 would find it truncated.

        The worst case is therefore 26 + 2 = 28, which the existing font step-down at
        ProgramHeader's lcdTextSizeGuard absorbs inside its 31-character 16px budget - and it now
        steps down LESS often than before, since the three characters of prefix are gone. */
    static constexpr int maxProgramNameLength = 26;

private:
    void handleAsyncUpdate() override;

    void applyProgram (const ProgramId& id);
    void setCurrentId (const ProgramId& id);
    void applyFactoryProgram (const FactoryProgram& program);

    /** **INIT only, and the ONE place the active-path filter is deliberately bypassed.**

        Every other Program writes only the parameters on its own path, so the ones it does not own
        keep the positions the player left them in. INIT cannot work that way: a canvas that is only
        blank along the path you happen to be on is not blank, and switching to Digital afterwards
        would land on whatever the last Digital Program left behind. */
    void applyInitProgram();
    void captureCleanSnapshot();

    /** The active-path IDs for the CURRENT parameter state. */
    juce::StringArray currentActivePath() const;

    juce::AudioProcessorValueTreeState& apvts;

    /** The User bank on disk. Scanning, naming, saving and deleting are core's - what a Program
        CONTAINS stays here, because Fifth Member's active-path serialisation is the one thing in
        this class no sibling shares. */
    nf::UserProgramStore store;

    mutable juce::SpinLock currentIdLock;
    ProgramId currentId;
    juce::SpinLock pendingLock;
    bool hasPendingProgram = false;
    ProgramId pendingProgram;

    /** The baseline the dirty flag compares against. Keyed by parameter ID inside - see
        nf/ParameterSnapshot.h for why that is not an index. Message-thread only. */
    nf::ParameterSnapshot cleanSnapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManager)
};
