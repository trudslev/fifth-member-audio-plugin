#pragma once

#include "DSP/FactoryPrograms.h"

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
    this set and nothing else, the dirty check compares exactly this set, and the factory-bank tests
    assert the untagged fields are zero.

    **Cross-Feed used to be excluded outright**, on the reasoning that it is a knob the player rides
    rather than part of a patch. That could not survive its own interaction with Stereo Mode. Stereo
    Mode IS stored, so recalling a Program recalls Ping-Pong - the one mode in which Cross-Feed is
    audible - without recalling the value that defines the bounce, and at cross 0 Ping-Pong leaves
    the right line silent. A Program could therefore name a ping-pong effect it had no way to
    reproduce. It is now stored on exactly the mode that uses it, which is the same conditional
    shape as Time and the character groups rather than a special case.
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

    /** Cross-Feed, and only in Ping-Pong.

        DelayCore reads it in that branch alone: Mono collapses to a single line, and Stereo feeds
        each line from its own input with `fb * dL` / `fb * dR`, never touching the cross term. So
        in the other two modes the value is genuinely inaudible and storing it would make every Mono
        and Stereo Program carry a figure that changes nothing - the same argument that keeps a
        Tape Program from storing Mod Rate.

        The panel already says this: the Cross-Feed LED is lit only in Ping-Pong. Storage now agrees
        with both the lamp and the DSP. */
    inline std::vector<const char*> crossFeedFor (int stereoMode)
    {
        if ((StereoMode) stereoMode == StereoMode::pingPong)
            return { ParamIDs::crossFeed };

        return {};
    }

    /** The complete set a Program stores for the given selector state. */
    inline std::vector<const char*> forState (bool sync, int character, int stereoMode)
    {
        auto ids = always();
        ids.push_back (timingFor (sync));

        for (const auto* id : characterFor (character))
            ids.push_back (id);

        for (const auto* id : crossFeedFor (stereoMode))
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

    /** 2: Ping-Pong Programs carry Cross-Feed. A schema-1 User Program simply has no crossFeed
        attribute, and the apply loop walks the attributes the file actually has - so an old file
        loads with Cross-Feed left where the player had it, exactly the schema-1 behaviour, and
        needs no migration step. The bump records that a Program's meaning changed. */
    inline constexpr int currentStateSchemaVersion = 3;

    /** The schema at which the session stopped storing a positional index and started storing bank
        + identifier. */
    inline constexpr int identitySchemaVersion = 3;

    /** **The oldest schema whose values can still be interpreted, pinned to a literal.** Every bump
        so far has been additive - schema 2 added crossFeed, and a schema-1 Program simply has no
        such attribute, which the apply loop already handles by walking the attributes the file
        actually has. So v1 remains fully readable.

        A literal on purpose. The gate read `savedSchema != currentStateSchemaVersion`, which was
        correct exactly once: it discarded schema-1 sessions wholesale even though this file's own
        note says schema-1 Programs load fine by design - the two were already inconsistent - and
        this bump would have discarded schema-2 sessions the same way. */
    inline constexpr int oldestReadableSchemaVersion = 1;

    /** **The identity attributes, and they are a contract.** `...ProgramName` is DISPLAY ONLY. */
    inline constexpr auto programBankAttribute = "fifthMemberProgramBank";
    inline constexpr auto programIdAttribute   = "fifthMemberProgramId";
    inline constexpr auto programNameAttribute = "fifthMemberProgramName";

    inline juce::String bankAttributeValue (ProgramBank bank)
    {
        switch (bank)
        {
            case ProgramBank::init:       return "init";
            case ProgramBank::factory:    return "factory";
            case ProgramBank::user:       return "user";
            case ProgramBank::unresolved: return "unresolved";
        }

        return "factory";
    }

    inline ProgramBank bankFromAttribute (const juce::String& value)
    {
        if (value == "init")       return ProgramBank::init;
        if (value == "user")       return ProgramBank::user;
        if (value == "unresolved") return ProgramBank::unresolved;

        return ProgramBank::factory;
    }

    /** Three outcomes, deliberately distinct: too old to interpret, too new to know about, usable. */
    enum class SchemaVerdict { tooOld, tooNew, readable };

    inline SchemaVerdict classifySchema (int savedSchema) noexcept
    {
        if (savedSchema < oldestReadableSchemaVersion) return SchemaVerdict::tooOld;
        if (savedSchema > currentStateSchemaVersion)   return SchemaVerdict::tooNew;

        return SchemaVerdict::readable;
    }

    inline constexpr auto currentProgramIndexAttribute = "fifthMemberCurrentProgramIndex";
}

//==============================================================================
/** **Mod Rate's range, named because TWO things need it and they must not diverge.**

    The parameter uses it, and dial 1's outer Hz ring is legended against it - that ring's numerals
    are printed at angles derived from this taper, so a ring built from a different one points at
    values the pointer never reaches. BRAND.md makes the printed scale a correctness requirement,
    and this is the shape of getting it wrong.

    It used to be built inline here and the ring carried the resulting skew as the literal
    `0.4090339496f`, transcribed. That number is `setSkewForCentre(1.0f)` on 0.1..5.0 evaluated by
    hand: correct, unexplained, and silently wrong the moment the range moved. `Tests/`
    now asserts the ring against this range via nf::printedScaleDefects, so the two cannot part
    company without a build failing.

    **1 Hz sits at 12 o'clock**, which is what the skew centre buys and why it is 1.0 rather than
    the arithmetic midpoint of 2.55.
*/
inline juce::NormalisableRange<float> modRateRange()
{
    juce::NormalisableRange<float> range { 0.1f, 5.0f, 0.01f };
    range.setSkewForCentre (1.0f);
    return range;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createFifthMemberParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    using Attributes = juce::AudioParameterFloatAttributes;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // withLabel() alone only feeds getLabel(); it does NOT change getText(), so a parameter left at
    // JUCE's default renders through juce::String(float) at full precision - "64.0" where the panel
    // and the host should both read "64". The LCD's live readout (section 6.3) joins the name, this
    // text and the label, and a host's automation lane shows the same string.
    //
    // The unit stays in the label rather than the text so the two are not doubled up when JUCE's own
    // generic editor appends it - except for Hz, where the unit is value-dependent.
    const auto addFloat = [&params] (const char* id, const juce::String& name,
                                     Range range, float defaultValue, const juce::String& unit)
    {
        auto attrs = Attributes().withLabel (unit);

        if (unit == "%")
            attrs = attrs.withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)); });
        else if (unit == "dB")
            attrs = attrs.withStringFromValueFunction ([] (float v, int)
                                                        { return (v >= 0.0f ? "+" : "") + juce::String (v, 1); });
        else if (unit == "ms")
            // One decimal below 10 ms, whole milliseconds above: with the interval now continuous,
            // the short end is where sub-millisecond resolution is both reachable and audible.
            attrs = attrs.withStringFromValueFunction ([] (float v, int)
                                                        { return v < 10.0f ? juce::String (v, 1)
                                                                           : juce::String (juce::roundToInt (v)); });

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, range, defaultValue, attrs));
    };

    // --- TIMING --------------------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::sync, 1 }, "SYNC", ParamDefaults::sync));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::noteDivision, 1 }, "NOTE DIVISION",
        juce::StringArray { "1/4", "1/8.", "1/8", "1/8T", "1/16" }, ParamDefaults::noteDivision));

    {
        // Skewed so the short end - where a few ms genuinely matter - gets most of the travel.
        // 375 ms lands near the middle of the knob rather than a fifth of the way up.
        // Interval 0, not 1 ms. Measured against this skew, one millisecond is 11.68 degrees of
        // rotation at 1 ms and 0.056 degrees at 2000: quantising to whole milliseconds makes 1.5 or
        // 2.5 ms unreachable exactly where short-delay comb effects need the resolution, while at
        // the long end the step is far finer than the knob can be aimed and buys nothing. The LCD
        // formats one decimal below 10 ms and whole milliseconds above.
        Range range { Timing::minDelayMs, Timing::maxDelayMs, 0.0f };
        range.setSkewForCentre (375.0f);
        addFloat (ParamIDs::timeMs, "TIME", range, ParamDefaults::timeMs, "ms");
    }

    // --- REPEATS -------------------------------------------------------------
    // Above 100% is deliberate: it is what makes the delay self-oscillate. DelayCore carries an
    // always-on soft ceiling so that stays musical rather than numeric.
    addFloat (ParamIDs::feedback, "FEEDBACK", Range { 0.0f, 110.0f, 0.1f }, ParamDefaults::feedback, "%");

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::stereoMode, 1 }, "STEREO MODE",
        juce::StringArray { "Mono", "Stereo", "Ping-Pong" }, ParamDefaults::stereoMode));

    addFloat (ParamIDs::crossFeed, "CROSS-FEED", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::crossFeed, "%");

    // --- DELAY CHARACTER -----------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::character, 1 }, "DELAY CHARACTER",
        juce::StringArray { "Tape", "BBD", "Digital" }, ParamDefaults::character));

    addFloat (ParamIDs::wow,      "WOW",             Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::wow,      "%");
    addFloat (ParamIDs::flutter,  "FLUTTER",         Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::flutter,  "%");
    addFloat (ParamIDs::genLoss,  "GENERATION LOSS", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::genLoss,  "%");

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::modRate, 1 }, "MOD RATE", modRateRange(), ParamDefaults::modRate,
        Attributes().withLabel ("Hz")
                    .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 2); })));

    addFloat (ParamIDs::modDepth, "MOD DEPTH",      Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::modDepth, "%");
    addFloat (ParamIDs::degrade,  "REPEAT DEGRADE", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::degrade,  "%");

    // --- CHARACTER, ALL MODES ------------------------------------------------
    {
        // A cutoff frequency, not a unitless amount: design/README.md's state table says 0-100 %,
        // but the brief and the whole factory bank are written in kHz, and a damping control on a
        // delay is a filter.
        Range range { 1000.0f, 16000.0f, 1.0f };
        range.setSkewForCentre (4000.0f);
        // Reads in kHz across its whole 1-16 kHz range, so the unit is in the text and the label is
        // left empty - "6.0 kHz", never "6000 Hz", which is what the panel prints beside it.
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::damping, 1 }, "DAMPING", range, ParamDefaults::dampingHz,
            Attributes().withStringFromValueFunction ([] (float v, int)
                                                       { return juce::String (v / 1000.0f, 1) + " kHz"; })));
    }

    addFloat (ParamIDs::saturation, "SATURATION", Range { 0.0f, 100.0f, 0.1f }, ParamDefaults::saturation, "%");

    // --- OUTPUT --------------------------------------------------------------
    addFloat (ParamIDs::mix,        "MIX",         Range { 0.0f, 100.0f, 0.1f },   ParamDefaults::mix,        "%");
    addFloat (ParamIDs::outputTrim, "OUTPUT TRIM", Range { -24.0f, 24.0f, 0.1f },  ParamDefaults::outputTrim, "dB");

    // New parameters are APPENDED below this line, never inserted above it - saved Programs and
    // host automation lanes are keyed by position as well as by ID.

    return { params.begin(), params.end() };
}
