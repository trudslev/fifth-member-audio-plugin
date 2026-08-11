#include "ProgramManager.h"

#include "../Parameters.h"

#include <algorithm>
#include <cmath>

#if ! defined (NF_COMPANY_NAME) || ! defined (NF_PRODUCT_NAME)
 #error "NF_COMPANY_NAME and NF_PRODUCT_NAME must come from CMake. They cannot be read from \
JucePlugin_* here: those macros only exist in the plugin target's generated header, and this file \
is also compiled into the Tests console app."
#endif

namespace
{
    constexpr float modifiedEpsilon = 1.0e-4f;

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
      userDirectory (userDirectoryOverride == juce::File() ? getDefaultUserProgramDirectory()
                                                           : userDirectoryOverride)
{
    refreshUserProgramList();
}

ProgramManager::~ProgramManager()
{
    cancelPendingUpdate();
}

void ProgramManager::initialise()
{
    applyProgramByIndex (defaultFactoryProgramIndex);
}

//==============================================================================
juce::File ProgramManager::getUserProgramDirectory() const
{
    return userDirectory;
}

juce::File ProgramManager::getDefaultUserProgramDirectory()
{
    // **Application data on every platform - no macOS special case.** This used to branch, putting
    // macOS Programs under ~/Library/Audio/Presets. That is Apple's location for the AU PRESET
    // FORMAT: .aupreset files the AU system itself scans, reads and writes. Our user Programs are
    // not those - they are application-owned data in our own XML format.
    //
    // **macOS needs the "Application Support" segment added by hand, and only macOS.** JUCE's
    // userApplicationDataDirectory is `~/Library` there - NOT `~/Library/Application Support` -
    // while it is `%APPDATA%` on Windows and `~/.config` on Linux, both of which are already the
    // right root. JUCE's own PropertiesFile appends the segment the same way, for the same reason.
    //
    // This was got wrong once in exactly the plausible direction: the note here used to claim JUCE
    // resolved the segment for us, and that hard-coding it would be wrong on two platforms out of
    // three. The first half was false, and the second half only argues for the `#if` - it is one
    // platform's extra segment, not a shared literal path. Programs landed directly in
    // `~/Library/<Company>/` for a while, which is not where application data goes on macOS and is
    // not a folder anything else writes into.
    //
    // No migration from the old location - nothing has shipped, so nothing is there to migrate.
    // See Elmer's ProgramManager for why that is a decision rather than an oversight.
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

   #if JUCE_MAC
    dir = dir.getChildFile ("Application Support");
   #endif

    return dir
               .getChildFile (NF_COMPANY_NAME).getChildFile (NF_PRODUCT_NAME).getChildFile ("Programs");
}

void ProgramManager::refreshUserProgramList()
{
    userProgramFiles.clear();

    if (userDirectory.isDirectory())
        for (const auto& entry : juce::RangedDirectoryIterator (userDirectory, false,
                                                                "*" + getProgramFileExtension()))
            userProgramFiles.add (entry.getFile());

    // Alphabetical by filename, deliberately not by modification time: the menu's order has to be
    // the same on every launch.
    std::sort (userProgramFiles.begin(), userProgramFiles.end(),
               [] (const juce::File& a, const juce::File& b)
               { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
}

//==============================================================================
int ProgramManager::getNumPrograms() const
{
    return kNumFactoryPrograms + userProgramFiles.size();
}

juce::String ProgramManager::getProgramName (int index) const
{
    if (isInitProgram (index))
        return kInitProgram.name;

    if (isFactoryProgram (index))
        return kFactoryPrograms[(size_t) index].name;

    const int userIndex = index - kNumFactoryPrograms;

    if (juce::isPositiveAndBelow (userIndex, userProgramFiles.size()))
        return userProgramFiles.getReference (userIndex).getFileNameWithoutExtension();

    return {};
}

//==============================================================================
std::vector<const char*> ProgramManager::currentActivePath() const
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

    return ActivePath::forState (boolOf (ParamIDs::sync),
                                 intOf (ParamIDs::character),
                                 intOf (ParamIDs::stereoMode));
}

//==============================================================================
void ProgramManager::requestProgramChange (int index)
{
    // INIT is a legal target and is NOT isPositiveAndBelow, so it is admitted explicitly rather
    // than by widening the check - which would also admit every other negative index.
    if (! isInitProgram (index) && ! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    pendingProgramIndex.store (index, std::memory_order_relaxed);
    triggerAsyncUpdate();
}

void ProgramManager::cancelPendingChange()
{
    pendingProgramIndex.store (noPendingProgram, std::memory_order_relaxed);
    cancelPendingUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    const int index = pendingProgramIndex.exchange (noPendingProgram, std::memory_order_relaxed);

    if (index != noPendingProgram)
        applyProgramByIndex (index);
}

void ProgramManager::setCurrentProgramIndexWithoutApplying (int index)
{
    // INIT is a valid remembered Program and is NOT isPositiveAndBelow, so it is admitted
    // explicitly rather than by widening the check. **No migration is needed**: INIT was ADDED at
    // -1 rather than inserted at 0, so not one existing Factory index moved and every session saved
    // before today still names the sound it was saved with.
    const bool valid = isInitProgram (index) || juce::isPositiveAndBelow (index, getNumPrograms());

    currentProgramIndex.store (valid ? index : defaultFactoryProgramIndex,
                               std::memory_order_relaxed);
    captureCleanSnapshot();

    if (onProgramListChanged)
        onProgramListChanged();
}

//==============================================================================
void ProgramManager::applyProgramByIndex (int index)
{
    if (isInitProgram (index))
    {
        applyInitProgram();
        currentProgramIndex.store (index, std::memory_order_relaxed);
        captureCleanSnapshot();

        if (onProgramListChanged)
            onProgramListChanged();

        return;
    }

    if (! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    if (isFactoryProgram (index))
    {
        applyFactoryProgram (kFactoryPrograms[(size_t) index]);
    }
    else
    {
        const int userIndex = index - kNumFactoryPrograms;
        std::unique_ptr<juce::XmlElement> xml (
            juce::XmlDocument::parse (userProgramFiles.getReference (userIndex)));

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

    currentProgramIndex.store (index, std::memory_order_relaxed);
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

juce::String ProgramManager::getProgramDisplayName (int index) const
{
    const auto name = getProgramName (index);

    if (isInitProgram (index) || name.isEmpty())
        return name;

    return juce::String (index + 1).paddedLeft ('0', 2) + " " + name;
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
    cleanSnapshot.clear();

    for (const auto* id : currentActivePath())
        if (const auto* p = apvts.getParameter (id))
            cleanSnapshot[juce::String (id)] = p->getValue();
}

bool ProgramManager::isModifiedFromLoadedProgram() const
{
    for (const auto* id : currentActivePath())
    {
        const auto key = juce::String (id);
        const auto it = cleanSnapshot.find (key);

        // A parameter that has just joined the active path - because Sync or Character was
        // flipped - has no baseline. The selector that moved is itself on the path and will have
        // registered the change, so this is not a missed modification.
        if (it == cleanSnapshot.end())
            continue;

        if (const auto* p = apvts.getParameter (id))
            if (std::abs (p->getValue() - it->second) > modifiedEpsilon)
                return true;
    }

    return false;
}

//==============================================================================
void ProgramManager::saveNewUserProgram (const juce::String& requestedName)
{
    juce::String name = requestedName.trim().toUpperCase();

    if (name.isEmpty())
        name = "TAKE " + juce::String (userProgramFiles.size() + 1);

    if (name.length() > maxProgramNameLength)
        name = name.substring (0, maxProgramNameLength);

    if (! userDirectory.isDirectory())
        userDirectory.createDirectory();

    // Only the active path is serialised, so a User Program behaves exactly like a Factory one:
    // recalling it leaves every persisting knob where the player left it.
    juce::XmlElement xml ("FifthMemberProgram");
    xml.setAttribute (LegacyMigration::stateSchemaVersionAttribute,
                      LegacyMigration::currentStateSchemaVersion);

    for (const auto* id : currentActivePath())
        if (const auto* p = apvts.getParameter (id))
            xml.setAttribute (juce::String (id), (double) p->convertFrom0to1 (p->getValue()));

    juce::File file = userDirectory.getChildFile (
        juce::File::createLegalFileName (name) + getProgramFileExtension());

    // Save always creates a NEW Program. Both older siblings write straight to this path, which
    // means reusing a name silently replaces that Program's contents - the one way their "never
    // overwrites" guarantee could actually be broken.
    if (file.existsAsFile())
        file = file.getNonexistentSibling();

    xml.writeTo (file);

    refreshUserProgramList();

    currentProgramIndex.store (kNumFactoryPrograms + userProgramFiles.indexOf (file),
                               std::memory_order_relaxed);
    captureCleanSnapshot();

    if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::deleteUserProgram (int index)
{
    // Gated here as well as at the button, so no code path can delete a Factory Program.
    if (isFactoryProgram (index))
        return;

    const int userIndex = index - kNumFactoryPrograms;

    if (! juce::isPositiveAndBelow (userIndex, userProgramFiles.size()))
        return;

    const bool wasCurrent = getCurrentProgram() == index;

    userProgramFiles.getReference (userIndex).deleteFile();
    refreshUserProgramList();

    if (wasCurrent)
        requestProgramChange (defaultFactoryProgramIndex);
    else if (onProgramListChanged)
        onProgramListChanged();
}
