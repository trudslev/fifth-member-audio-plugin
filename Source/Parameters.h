#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>
#include <vector>

/**
    Single source of truth for Fifth Member's parameter IDs, APVTS layout, the note-division table,
    and - critically - the definition of a Program's "active path".

    Parameters carry physical values behind a NormalisableRange (ms, Hz, %, dB), matching TapeRot,
    Gatecrasher and CHORUS-60. Reflect-84 stores normalised 0-1 instead because its own design doc
    specified that state model; this one's does not, and the factory bank is written in physical
    units throughout.
*/
namespace ParamIDs
{
    // TIMING
    inline constexpr auto sync         = "sync";
    inline constexpr auto noteDivision = "noteDivision";
    inline constexpr auto timeMs       = "timeMs";

    // REPEATS
    inline constexpr auto feedback     = "feedback";
    inline constexpr auto stereoMode   = "stereoMode";
    inline constexpr auto crossFeed    = "crossFeed";

    // DELAY CHARACTER
    inline constexpr auto character    = "character";
    inline constexpr auto wow          = "wow";
    inline constexpr auto flutter      = "flutter";
    inline constexpr auto genLoss      = "genLoss";
    inline constexpr auto modRate      = "modRate";
    inline constexpr auto modDepth     = "modDepth";
    inline constexpr auto degrade      = "degrade";

    // CHARACTER - ALL MODES
    inline constexpr auto damping      = "damping";
    inline constexpr auto saturation   = "saturation";

    // OUTPUT
    inline constexpr auto mix          = "mix";
    inline constexpr auto outputTrim   = "outputTrim";
}

//==============================================================================
enum class NoteDivision { quarter = 0, dottedEighth, eighth, eighthTriplet, sixteenth };
enum class StereoMode   { mono = 0, stereo, pingPong };
enum class DelayCharacter { tape = 0, bbd, digital };

inline constexpr int numNoteDivisions = 5;
inline constexpr int numStereoModes = 3;
inline constexpr int numDelayCharacters = 3;

namespace Timing
{
    /** Multiplier against a quarter note. design/README.md's division table, and the prototype's
        `DIVS` array, agree exactly on these. */
    inline constexpr float divisionMultiplier (int division) noexcept
    {
        switch ((NoteDivision) division)
        {
            case NoteDivision::quarter:        return 1.0f;
            case NoteDivision::dottedEighth:   return 0.75f;
            case NoteDivision::eighth:         return 0.5f;
            case NoteDivision::eighthTriplet:  return 1.0f / 3.0f;
            case NoteDivision::sixteenth:      return 0.25f;
        }

        return 0.5f;
    }

    inline const char* divisionLabel (int division) noexcept
    {
        switch ((NoteDivision) division)
        {
            case NoteDivision::quarter:       return "1/4";
            case NoteDivision::dottedEighth:  return "1/8.";
            case NoteDivision::eighth:        return "1/8";
            case NoteDivision::eighthTriplet: return "1/8T";
            case NoteDivision::sixteenth:     return "1/16";
        }

        return "1/8";
    }

    inline constexpr float minDelayMs = 1.0f;
    inline constexpr float maxDelayMs = 2000.0f;

    /** Headroom above maxDelayMs for character modulation and for a synced division at a very low
        BPM. A quarter note at 30 BPM is 2000 ms, which is already the ceiling, so the extra is
        purely modulation and interpolation margin. */
    inline constexpr float delayBufferMs = 2500.0f;

    inline constexpr double fallbackBpm = 120.0;
}

//==============================================================================
/**
    Which parameters a Program stores, given the state of its mutually exclusive selectors.

    This is the whole of Correction 1 expressed once, in one place. A Program stores only the
    parameters on its active path; where an exclusive selector chooses between alternatives, the
    non-selected alternatives are not Program state at all - they persist independently across
    Program changes, the way a physical knob keeps its position regardless of which patch is
    recalled.

    Deliberately unlike CHORUS-60, whose engine I and II are independent toggles combinable in
    place, so every combination had to be pre-baked into every Program. Fifth Member's selectors are
    exclusive switches - closer to Gatecrasher's Algorithm knob. Switching is "now doing something
    else", not "adding another simultaneous state".

    Everything downstream depends on this being the only definition: ProgramManager applies exactly
    this set and nothing else, the dirty check compares exactly this set (so moving Cross-Feed must
    not light SAVE), and the factory-bank tests assert the untagged fields are zero.
*/
namespace ActivePath
{
    /** Stored by every Program regardless of selector state. */
    inline std::vector<const char*> always()
    {
        return { ParamIDs::sync,      ParamIDs::feedback,   ParamIDs::stereoMode,
                 ParamIDs::character, ParamIDs::damping,    ParamIDs::saturation,
                 ParamIDs::mix,       ParamIDs::outputTrim };
    }

    /** The timing control the Sync switch makes active. The other one persists independently. */
    inline const char* timingFor (bool sync) noexcept
    {
        return sync ? ParamIDs::noteDivision : ParamIDs::timeMs;
    }

    /** The selected character mode's own parameters. The other two modes' persist independently. */
    inline std::vector<const char*> characterFor (int character)
    {
        switch ((DelayCharacter) character)
        {
            case DelayCharacter::tape:    return { ParamIDs::wow, ParamIDs::flutter, ParamIDs::genLoss };
            case DelayCharacter::bbd:     return { ParamIDs::modRate, ParamIDs::modDepth };
            case DelayCharacter::digital: return { ParamIDs::degrade };
        }

        return {};
    }

    /** The complete set a Program stores for the given selector state.

        Note what is absent: Cross-Feed is never here. It is audible only in Ping-Pong and its LED
        is conditional, but it is never Program state - it persists across every Program change. */
    inline std::vector<const char*> forState (bool sync, int character)
    {
        auto ids = always();
        ids.push_back (timingFor (sync));

        for (const auto* id : characterFor (character))
            ids.push_back (id);

        return ids;
    }
}

//==============================================================================
namespace ParamDefaults
{
    inline constexpr bool  sync         = true;
    inline constexpr int   noteDivision = (int) NoteDivision::dottedEighth;
    inline constexpr float timeMs       = 375.0f;
    inline constexpr float feedback     = 35.0f;
    inline constexpr int   stereoMode   = (int) StereoMode::pingPong;
    inline constexpr float crossFeed    = 80.0f;
    inline constexpr int   character    = (int) DelayCharacter::tape;
    inline constexpr float wow          = 25.0f;
    inline constexpr float flutter      = 20.0f;
    inline constexpr float genLoss      = 30.0f;
    inline constexpr float modRate      = 0.6f;
    inline constexpr float modDepth     = 20.0f;
    inline constexpr float degrade      = 20.0f;
    inline constexpr float dampingHz    = 6000.0f;
    inline constexpr float saturation   = 15.0f;
    inline constexpr float mix          = 35.0f;
    inline constexpr float outputTrim   = 0.0f;
}

namespace LegacyMigration
{
    /** Written into the state XML root and checked on restore. Deliberately separate from
        juce::ParameterID's versionHint, which only affects the host's numeric automation-lane ID -
        APVTS's own XML is keyed by plain ID string regardless, so a schema change needs its own
        marker. */
    inline constexpr auto stateSchemaVersionAttribute = "fifthMemberStateSchemaVersion";
    inline constexpr int currentStateSchemaVersion = 1;

    inline constexpr auto currentProgramIndexAttribute = "fifthMemberCurrentProgramIndex";
}

//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createFifthMemberParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    using Attributes = juce::AudioParameterFloatAttributes;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const auto addFloat = [&params] (const char* id, const juce::String& name,
                                     Range range, float defaultValue, const juce::String& unit)
    {
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, defaultValue,
            Attributes().withLabel (unit)));
    };

    // --- TIMING --------------------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::sync, 1 }, "Sync", ParamDefaults::sync));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::noteDivision, 1 }, "Note Division",
        juce::StringArray { "1/4", "1/8.", "1/8", "1/8T", "1/16" }, ParamDefaults::noteDivision));

    {
        // Skewed so the short end - where a few ms genuinely matter - gets most of the travel.
        // 375 ms lands near the middle of the knob rather than a fifth of the way up.
        Range range { Timing::minDelayMs, Timing::maxDelayMs, 1.0f };
        range.setSkewForCentre (375.0f);
        addFloat (ParamIDs::timeMs, "Time", range, ParamDefaults::timeMs, "ms");
    }

    // --- REPEATS -------------------------------------------------------------
    // Above 100% is deliberate: it is what makes the delay self-oscillate. DelayCore carries an
    // always-on soft ceiling so that stays musical rather than numeric.
    addFloat (ParamIDs::feedback, "Feedback", Range { 0.0f, 110.0f, 0.1f }, ParamDefaults::feedback, "%");

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::stereoMode, 1 }, "Stereo Mode",
        juce::StringArray { "Mono", "Stereo", "Ping-Pong" }, ParamDefaults::stereoMode));

    addFloat (ParamIDs::crossFeed, "Cross-Feed", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::crossFeed, "%");

    // --- DELAY CHARACTER -----------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::character, 1 }, "Delay Character",
        juce::StringArray { "Tape", "BBD", "Digital" }, ParamDefaults::character));

    addFloat (ParamIDs::wow,      "Wow",             Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::wow,      "%");
    addFloat (ParamIDs::flutter,  "Flutter",         Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::flutter,  "%");
    addFloat (ParamIDs::genLoss,  "Generation Loss", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::genLoss,  "%");

    {
        Range range { 0.1f, 5.0f, 0.01f };
        range.setSkewForCentre (1.0f);
        addFloat (ParamIDs::modRate, "Mod Rate", range, ParamDefaults::modRate, "Hz");
    }

    addFloat (ParamIDs::modDepth, "Mod Depth",      Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::modDepth, "%");
    addFloat (ParamIDs::degrade,  "Repeat Degrade", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::degrade,  "%");

    // --- CHARACTER, ALL MODES ------------------------------------------------
    {
        // A cutoff frequency, not a unitless amount: design/README.md's state table says 0-100 %,
        // but the brief and the whole factory bank are written in kHz, and a damping control on a
        // delay is a filter.
        Range range { 1000.0f, 16000.0f, 1.0f };
        range.setSkewForCentre (4000.0f);
        addFloat (ParamIDs::damping, "Damping", range, ParamDefaults::dampingHz, "Hz");
    }

    addFloat (ParamIDs::saturation, "Saturation", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::saturation, "%");

    // --- OUTPUT --------------------------------------------------------------
    addFloat (ParamIDs::mix,        "Mix",         Range { 0.0f, 100.0f, 0.1f },   ParamDefaults::mix,        "%");
    addFloat (ParamIDs::outputTrim, "Output Trim", Range { -24.0f, 24.0f, 0.1f },  ParamDefaults::outputTrim, "dB");

    // New parameters are APPENDED below this line, never inserted above it - saved Programs and
    // host automation lanes are keyed by position as well as by ID.

    return { params.begin(), params.end() };
}
