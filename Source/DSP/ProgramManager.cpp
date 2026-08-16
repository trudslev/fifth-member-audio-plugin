#include "ProgramManager.h"

#include "../Parameters.h"

#include <nf/UserProgramDirectory.h>

#include <algorithm>
#include <cmath>

#if ! defined (NF_COMPANY_NAME) || ! defined (NF_PRODUCT_NAME)
 #error "NF_COMPANY_NAME and NF_PRODUCT_NAME must come from CMake. They cannot be read from \
JucePlugin_* here: those macros only exist in the plugin target's generated header, and this file \
is also compiled into the Tests console app."
#endif

namespace
{
    /** Sets a parameter by ID from a plain physical value, going through the host so automation and
        the GUI both see it. */
    void setPlain (juce::AudioProcessorValueTreeState& apvts, const char* id, float plainValue)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        else
            jassertfalse;
    }
}

//==============================================================================
ProgramManager::ProgramManager (juce::AudioProcessorValueTreeState& state,
                                juce::File userDirectoryOverride)
    : apvts (state),
      store (nf::userProgramDirectory (NF_COMPANY_NAME, NF_PRODUCT_NAME, userDirectoryOverride),
             getProgramFileExtension(),
             maxProgramNameLength)
{
    // The identity must be valid before initialise() runs, as the atomic index it replaced was
    // valid from its in-class initialiser.
    setCurrentId (factoryIdAt (defaultFactoryProgramIndex));

    store.refresh();
}

ProgramManager::~ProgramManager()
{
    cancelPendingUpdate();
}

void ProgramManager::initialise()
{
    applyProgram (factoryIdAt (defaultFactoryProgramIndex));
}

//==============================================================================
juce::File ProgramManager::getUserProgramDirectory() const
{
    return store.getDirectory();
}

juce::File ProgramManager::getDefaultUserProgramDirectory()
{
    // The per-OS resolution, the "Application Support" segment macOS alone needs, and the reason
    // ~/Library/Audio/Presets is the wrong answer are all in nf/UserProgramDirectory.h now. That
    // reasoning was carried in six near-identical comment blocks, and the one time it was wrong it
    // was wrong in all six at once.
    return nf::userProgramDirectory (NF_COMPANY_NAME, NF_PRODUCT_NAME);
}

//==============================================================================
juce::String ProgramManager::getProgramName (int factoryPosition) const
{
    // Raw, unnumbered - what the HOST's list wants, since a host renders its own numbering.
    return juce::isPositiveAndBelow (factoryPosition, kNumFactoryPrograms)
               ? juce::String (kFactoryPrograms[(size_t) factoryPosition].name)
               : juce::String();
}

//==============================================================================
juce::StringArray ProgramManager::currentActivePath() const
{
    const auto boolOf = [this] (const char* id)
    {
        const auto* raw = apvts.getRawParameterValue (id);
        return raw != nullptr && raw->load() > 0.5f;
    };

    const auto intOf = [this] (const char* id)
    {
        if (const auto* p = dynamic_cast<const juce::AudioParameterChoice*> (apvts.getParameter (id)))
            return p->getIndex();
        return 0;
    };

    // ActivePath::forState is the single definition and stays a vector of literals - this converts
    // once, at the boundary, because core's snapshot compares by juce::String key.
    juce::StringArray ids;

    for (const auto* id : ActivePath::forState (boolOf (ParamIDs::sync),
                                                intOf (ParamIDs::character),
                                                intOf (ParamIDs::stereoMode)))
        ids.add (id);

    return ids;
}

//==============================================================================
//==============================================================================
// Identity. Nothing below addresses a Program by position except the deliberate crossings.

ProgramId ProgramManager::factoryIdAt (int factoryPosition)
{
    const auto& p = kFactoryPrograms[(size_t) factoryPosition];
    return { ProgramBank::factory, p.slug, p.name };
}

ProgramId ProgramManager::initId()
{
    return { ProgramBank::init, kInitProgram.slug, kInitProgram.name };
}

int ProgramManager::factoryPositionOf (const juce::String& slug)
{
    for (size_t i = 0; i < kFactoryPrograms.size(); ++i)
        if (slug == kFactoryPrograms[i].slug)
            return (int) i;

    return -1;
}

ProgramId ProgramManager::getCurrentProgramId() const
{
    const juce::SpinLock::ScopedLockType lock (currentIdLock);
    return currentId;
}

void ProgramManager::setCurrentId (const ProgramId& id)
{
    const juce::SpinLock::ScopedLockType lock (currentIdLock);
    currentId = id;
}

int ProgramManager::getCurrentFactoryPosition() const
{
    const auto id = getCurrentProgramId();

    if (id.bank == ProgramBank::factory)
        if (const int pos = factoryPositionOf (id.id); pos >= 0)
            return pos;

    return 0;
}

ProgramId ProgramManager::resolve (ProgramBank bank, const juce::String& id,
                                    const juce::String& displayName) const
{
    if (bank == ProgramBank::init && id == kInitProgram.slug)
        return initId();

    if (bank == ProgramBank::factory)
        if (const int pos = factoryPositionOf (id); pos >= 0)
            return factoryIdAt (pos);

    if (bank == ProgramBank::user)
        if (store.fileFor (id) != juce::File())
            return { ProgramBank::user, id, id };

    // **Degrade honestly.** The restored values are correct and stay put; only the name is unknown.
    return { ProgramBank::unresolved, id, displayName.isNotEmpty() ? displayName : id };
}

std::vector<ProgramId> ProgramManager::listPrograms() const
{
    std::vector<ProgramId> out;
    out.reserve (1 + kFactoryPrograms.size() + (size_t) store.getFiles().size());

    out.push_back (initId());

    for (size_t i = 0; i < kFactoryPrograms.size(); ++i)
        out.push_back (factoryIdAt ((int) i));

    for (const auto& f : store.getFiles())
    {
        const auto stem = f.getFileNameWithoutExtension();
        out.push_back ({ ProgramBank::user, stem, stem });
    }

    return out;
}

juce::String ProgramManager::displayLabelFor (const ProgramId& id) const
{
    // The Factory position is resolved here because the Factory bank is this casting's own; core
    // never holds one. The two-digit number itself is presentation and is computed, never stored.
    return nf::programDisplayLabel (id, id.bank == ProgramBank::factory ? factoryPositionOf (id.id)
                                                                        : -1);
}

/*  **The critical section is a SWAP now, and it used to be two assignments.**

    A `juce::String` copy is a refcount increment and reads as safe. The ASSIGNMENT is the other
    half: it releases whatever the target held first, and a refcount reaching zero calls `free()`.
    So `pendingProgram = id` and `id = pendingProgram` each did heap work, and both were inside the
    lock — on a path VST3 can deliver **on the audio thread**, since a program change is an
    automatable parameter there.

    **Measured at 0.12 us worst case against a 10,667 us block budget**, so this was never a dropout
    risk and is not sold as one. It is negligible because a refcount release happens to be cheap,
    not because anything guarantees the path stays heap-free — and the next person to add a field to
    `ProgramId` has no reason to think about it.

    The copy and the destruction both move OUT of the lock: `exchangePendingProgram` takes its
    argument by value, so the caller's copy is made in the caller's frame, and returns the previous
    program by value, so its release happens in the caller's frame too. What is left between the
    lock and the unlock is a pointer exchange.

    **Named functions rather than inline blocks because that is what makes it testable.** An
    allocation sentinel is not lock-aware, so a probe around `requestProgramChange` sees the same
    total either way — the change is WHERE the work happens, not whether it happens. Arming the
    sentinel around a function that IS the critical section is the only honest way to assert it. */
ProgramId ProgramManager::exchangePendingProgram (ProgramId incoming)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);

    std::swap (pendingProgram, incoming);
    hasPendingProgram = true;

    return incoming;   // the PREVIOUS pending program; it is released in the caller's frame
}

bool ProgramManager::takePendingProgram (ProgramId& out)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);

    if (! hasPendingProgram)
        return false;

    // `out` is empty on entry, so this is a pointer exchange and nothing is released here.
    std::swap (out, pendingProgram);
    hasPendingProgram = false;

    return true;
}

void ProgramManager::requestProgramChange (const ProgramId& id)
{
    // The copy is made HERE, in this frame: copying a ProgramId is two refcount increments, and an
    // increment never frees. The previous pending program comes back and is released here too.
    const ProgramId previous = exchangePendingProgram (id);
    juce::ignoreUnused (previous);

    triggerAsyncUpdate();
}

void ProgramManager::cancelPendingChange()
{
    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        hasPendingProgram = false;
    }

    cancelPendingUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    ProgramId id;

    if (! takePendingProgram (id))
        return;

    applyProgram (id);
}

void ProgramManager::setCurrentProgramWithoutApplying (const ProgramId& id)
{
    // INIT is a valid remembered Program and is NOT isPositiveAndBelow, so it is admitted
    // explicitly rather than by widening the check. **No migration is needed**: INIT was ADDED at
    // -1 rather than inserted at 0, so not one existing Factory index moved and every session saved
    // before today still names the sound it was saved with.
    setCurrentId (id);
    captureCleanSnapshot();

    if (onProgramListChanged)
        onProgramListChanged();
}

//==============================================================================
void ProgramManager::applyProgram (const ProgramId& id)
{
    if (id.bank == ProgramBank::init)
    {
        // The slug is checked, not just the bank. An id claiming to be INIT with some other
        // identifier names nothing, and applying INIT anyway would be the same "land on whatever is
        // nearby" failure this whole model exists to prevent.
        if (id.id != kInitProgram.slug)
            return;

        applyInitProgram();
    }
    else if (id.bank == ProgramBank::factory)
    {
        const int pos = factoryPositionOf (id.id);

        if (pos < 0)
            return;

        applyFactoryProgram (kFactoryPrograms[(size_t) pos]);
    }
    else if (id.bank == ProgramBank::user)
    {
        const auto file = store.fileFor (id.id);

        if (file == juce::File())
            return;

        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));

        if (xml == nullptr)
            return;

        // Attribute by attribute, NOT apvts.replaceState(): a User Program stores only its own
        // active path, and replacing the whole state would wipe every parameter that is meant to
        // persist across a Program change.
        for (int i = 0; i < xml->getNumAttributes(); ++i)
        {
            const auto& id = xml->getAttributeName (i);

            if (apvts.getParameter (id) != nullptr)
                setPlain (apvts, id.toRawUTF8(), (float) xml->getDoubleAttribute (id));
        }
    }
    else
    {
        // Unresolved: the values are whatever the session restored and stay exactly as they are.
        setCurrentId (id);
        captureCleanSnapshot();

        if (onProgramListChanged)
            onProgramListChanged();

        return;
    }

    setCurrentId (id);
    captureCleanSnapshot();

    if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::applyInitProgram()
{
    const auto& p = kInitProgram;

    // **Every parameter, unconditionally - no active-path filter.** This is the one place that
    // filter is bypassed, and the reason is what INIT is FOR: a canvas that is only blank along the
    // path you happen to be on is not blank. Filtered, loading INIT from a Digital Program would
    // leave Repeat Degrade wherever that Program left it, so the first switch back to Digital would
    // land on someone else's sound.
    //
    // Written out rather than routed through applyFactoryProgram with a flag, because the two do
    // genuinely different things and a bool parameter would hide that at every call site.
    if (auto* sync = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (ParamIDs::sync)))
        *sync = p.sync;

    if (auto* div = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::noteDivision)))
        *div = juce::jlimit (0, numNoteDivisions - 1, p.division);

    if (auto* sm = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::stereoMode)))
        *sm = juce::jlimit (0, numStereoModes - 1, p.stereoMode);

    if (auto* ch = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::character)))
        *ch = juce::jlimit (0, numDelayCharacters - 1, p.character);

    setPlain (apvts, ParamIDs::timeMs,     p.timeMs);
    setPlain (apvts, ParamIDs::feedback,   p.feedbackPercent);
    setPlain (apvts, ParamIDs::wow,        p.wow);
    setPlain (apvts, ParamIDs::flutter,    p.flutter);
    setPlain (apvts, ParamIDs::genLoss,    p.genLoss);
    setPlain (apvts, ParamIDs::modRate,    p.modRateHz);
    setPlain (apvts, ParamIDs::modDepth,   p.modDepth);
    setPlain (apvts, ParamIDs::degrade,    p.degrade);
    setPlain (apvts, ParamIDs::crossFeed,  p.crossFeedPercent);
    setPlain (apvts, ParamIDs::damping,    p.dampingHz);
    setPlain (apvts, ParamIDs::saturation, p.saturationPercent);
    setPlain (apvts, ParamIDs::mix,        p.mixPercent);
    setPlain (apvts, ParamIDs::outputTrim, p.trimDb);
}

void ProgramManager::applyFactoryProgram (const FactoryProgram& program)
{
    // Always on the active path.
    if (auto* p = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (ParamIDs::sync)))
        *p = program.sync;

    setPlain (apvts, ParamIDs::feedback,   program.feedbackPercent);
    setPlain (apvts, ParamIDs::damping,    program.dampingHz);
    setPlain (apvts, ParamIDs::saturation, program.saturationPercent);
    setPlain (apvts, ParamIDs::mix,        program.mixPercent);
    setPlain (apvts, ParamIDs::outputTrim, program.trimDb);

    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::stereoMode)))
        *p = juce::jlimit (0, numStereoModes - 1, program.stereoMode);

    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::character)))
        *p = juce::jlimit (0, numDelayCharacters - 1, program.character);

    // Conditionally on the active path. The alternative is deliberately NOT written - that is the
    // whole of Correction 1, and the reason the untagged fields in the bank are exact zeros.
    if (program.sync)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::noteDivision)))
            *p = juce::jlimit (0, numNoteDivisions - 1, program.division);
    }
    else
    {
        setPlain (apvts, ParamIDs::timeMs, program.timeMs);
    }

    switch ((DelayCharacter) program.character)
    {
        case DelayCharacter::tape:
            setPlain (apvts, ParamIDs::wow,     program.wow);
            setPlain (apvts, ParamIDs::flutter, program.flutter);
            setPlain (apvts, ParamIDs::genLoss, program.genLoss);
            break;

        case DelayCharacter::bbd:
            setPlain (apvts, ParamIDs::modRate,  program.modRateHz);
            setPlain (apvts, ParamIDs::modDepth, program.modDepth);
            break;

        case DelayCharacter::digital:
            setPlain (apvts, ParamIDs::degrade, program.degrade);
            break;
    }

    // Cross-Feed, iff Ping-Pong - the only mode whose DelayCore branch reads it. In Mono and Stereo
    // it is left alone for the same reason the unselected character group is.
    if ((StereoMode) program.stereoMode == StereoMode::pingPong)
        setPlain (apvts, ParamIDs::crossFeed, program.crossFeedPercent);
}

//==============================================================================
void ProgramManager::captureCleanSnapshot()
{
    // **Every parameter, not just the active path - and the comparison is what narrows.** The old
    // code captured the path alone, so a parameter joining the path later had no baseline and was
    // skipped. Capturing everything gives it one, and since the path is a function of Sync,
    // Character and Stereo Mode - all three of which are always compared - a parameter can only be
    // on the path when its discriminator is where the Program left it. The answer is identical
    // either way; this one just cannot be read as "the baseline is incomplete".
    cleanSnapshot.capture (apvts.processor);
}

bool ProgramManager::isModifiedFromLoadedProgram() const
{
    // Restricted to the active path, recomputed each call: flipping Sync or Character changes which
    // parameters are being compared, so moving the inactive timing control or another mode's dial
    // is deliberately not a modification.
    return cleanSnapshot.differsFrom (apvts.processor, currentActivePath());
}

//==============================================================================
void ProgramManager::saveNewUserProgram (const juce::String& requestedName)
{
    // **Only the active path is serialised, and that stays here** - it is the one thing in this
    // class no sibling shares. Core owns naming, collision and the write; what a Program CONTAINS
    // is the casting's own business, which is exactly the split that lets Fifth Member's filtered
    // model and the other five castings' whole-state model use one store.
    juce::XmlElement xml ("FifthMemberProgram");
    xml.setAttribute (LegacyMigration::stateSchemaVersionAttribute,
                      LegacyMigration::currentStateSchemaVersion);

    for (const auto& id : currentActivePath())
        if (const auto* p = apvts.getParameter (id))
            xml.setAttribute (id, (double) p->convertFrom0to1 (p->getValue()));

    const auto file = store.save (requestedName, xml);

    if (file == juce::File())
        return;   // the write failed; the panel keeps naming the Program it was already on

    // **The stem comes off the file core returned, not off the requested name.** A collision takes
    // the next free sibling, so taking it from the request would point the panel at the first file
    // while the values came from the second.
    const auto stem = file.getFileNameWithoutExtension();
    setCurrentId ({ ProgramBank::user, stem, stem });
    captureCleanSnapshot();

    if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::deleteUserProgram (const ProgramId& id)
{
    // Gated on the BANK, which is stronger than the old index range: an id from any other bank
    // simply cannot address a file.
    if (id.bank != ProgramBank::user)
        return;

    const bool wasCurrent = getCurrentProgramId() == id;

    if (! store.remove (id.id))
        return;

    // Deliberately NOT the unresolved state: deleting from the panel is unambiguous intent.
    if (wasCurrent)
        requestProgramChange (factoryIdAt (defaultFactoryProgramIndex));
    else if (onProgramListChanged)
        onProgramListChanged();
}
