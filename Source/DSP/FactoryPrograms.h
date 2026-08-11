#pragma once

#include <juce_core/juce_core.h>

#include <array>

/**
    Fifth Member's factory bank.

    A flat POD of plain ints and floats rather than DSP-layer types, so this header stays decoupled
    and can be compiled into the test console app on its own.

    **The zero-fill is the point.** Per Correction 1, a Program stores only the parameters on its
    active path. The fields belonging to a non-selected alternative are therefore not part of that
    Program at all, and are written as exact zeros (or -1 for an unused division index) rather than
    given plausible-looking values. Tests/FactoryProgramsTests.cpp asserts this per Program, which
    makes the rule structurally checkable instead of a convention someone will eventually "fix" by
    filling in the blanks.

    Concretely, for every row: exactly one of `division` / `timeMs` carries data, exactly one of the
    three character groups does, and `crossFeedPercent` carries data iff `stereoMode` is Ping-Pong.

    **Cross-Feed is a third conditional axis, not a global.** DelayCore uses it in the Ping-Pong
    branch only, so a Mono or Stereo Program zero-fills it like any other parameter it does not own.
    The five Ping-Pong rows carry real values because at cross 0 that mode leaves the right line
    silent - a ping-pong Program that did not store its own cross would recall as a one-sided delay
    whenever the knob happened to be down.

    This is deliberately unlike CHORUS-60, whose bank had to carry all three engine configurations
    in every Program because its I and II latches are independent toggles combinable in place.
    Fifth Member's selectors are exclusive switches: switching is "now doing something else", not
    "adding another simultaneous state".
*/
/** Which list a Program belongs to. INIT is its own bank rather than a magic index; `unresolved`
    is a stored identifier that no longer names anything. */
enum class ProgramBank
{
    init,
    factory,
    user,
    unresolved
};

/** **How a Program is identified everywhere except the host adapter.** Not a position - positions
    change when the bank is reordered or extended.

    `displayName` is carried because a factory slug is not presentable: "sky-wide?" in the LCD would
    read as a rendering fault. It is display only and never resolves anything. */
struct ProgramId
{
    ProgramBank bank = ProgramBank::factory;
    juce::String id;
    juce::String displayName;

    bool operator== (const ProgramId& o) const noexcept { return bank == o.bank && id == o.id; }
    bool operator!= (const ProgramId& o) const noexcept { return ! operator== (o); }
};

struct FactoryProgram
{
    /** **The permanent identity, fixed at creation and never changed again.** `name` is a label the
        designers may revise; `slug` may not be, because it is what a saved session stores. */
    const char* slug;

    const char* name;

    bool  sync;
    int   division;             // iff sync;   -1 otherwise
    float timeMs;               // iff !sync;   0 otherwise

    float feedbackPercent;
    int   stereoMode;           // 0 Mono, 1 Stereo, 2 Ping-Pong
    int   character;            // 0 Tape, 1 BBD, 2 Digital

    float wow, flutter, genLoss;    // iff character == Tape;     all 0 otherwise
    float modRateHz, modDepth;      // iff character == BBD;      both 0 otherwise
    float degrade;                  // iff character == Digital;  0 otherwise

    float crossFeedPercent;         // iff stereoMode == Ping-Pong;  0 otherwise

    float dampingHz, saturationPercent, mixPercent, trimDb;
};

// The Ping-Pong rows' cross values are set from the mechanism, not by ear: fbL = fb*((1-x)*dL +
// x*dR) and its mirror, so x = 1 makes each line feed only the other (maximum alternation), x = 0.5
// feeds both lines the same mix and collapses the bounce to the centre, and x = 0 leaves the right
// line silent. Useful ping-pong therefore lives in roughly 0.75-1.0, and these sit in it. Like the
// rest of the bank they are structurally reasoned and still awaiting a by-ear pass.
inline constexpr std::array<FactoryProgram, 11> kFactoryPrograms { {
    //  name                sync  div  timeMs    FB  st  ch     wow  flut  gen    rate  depth  degr  cross   damp    sat   mix   trim
    { "you-too", "YOU TOO?",           true,   1,    0.0f, 35.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.6f, 20.0f,  0.0f, 80.0f, 6000.0f, 15.0f, 35.0f,  0.0f },
    { "sky-wide", "SKY WIDE",           true,   1,    0.0f, 45.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.5f, 15.0f,  0.0f, 90.0f, 8000.0f, 10.0f, 40.0f,  0.0f },
    { "new-years", "NEW YEAR'S",         true,   2,    0.0f, 40.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.7f, 25.0f,  0.0f, 80.0f, 7000.0f, 18.0f, 38.0f,  0.0f },
    { "great-gig", "GREAT GIG",         false,  -1,  550.0f, 55.0f, 1, 0,  30.0f, 20.0f, 40.0f, 0.0f,  0.0f,  0.0f,  0.0f, 4500.0f, 30.0f, 45.0f,  0.0f },
    { "dark-echoes", "DARK ECHOES",       false,  -1,  480.0f, 60.0f, 1, 0,  35.0f, 25.0f, 50.0f, 0.0f,  0.0f,  0.0f,  0.0f, 3500.0f, 35.0f, 40.0f,  0.0f },
    { "long-loop", "LONG LOOP",         false,  -1,  900.0f, 70.0f, 1, 0,  20.0f, 12.0f, 35.0f, 0.0f,  0.0f,  0.0f,  0.0f, 5000.0f, 20.0f, 50.0f,  0.0f },
    { "slow-build", "SLOW BUILD",        false,  -1,  700.0f, 65.0f, 1, 0,  22.0f, 14.0f, 38.0f, 0.0f,  0.0f,  0.0f,  0.0f, 5500.0f, 22.0f, 42.0f,  0.0f },
    { "slap-happy", "SLAP HAPPY",         true,   4,    0.0f, 15.0f, 0, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 10.0f,  0.0f, 10000.0f, 8.0f, 25.0f,  0.0f },
    { "doubled-up", "DOUBLED UP",         true,   4,    0.0f, 10.0f, 1, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f,  5.0f,  0.0f, 9000.0f,  5.0f, 20.0f,  0.0f },
    { "sixteenth-sense", "SIXTEENTH SENSE",    true,   4,    0.0f, 50.0f, 2, 1,   0.0f, 0.0f, 0.0f,  1.2f, 30.0f,  0.0f, 75.0f, 6500.0f, 15.0f, 40.0f,  0.0f },
    // Feedback above 100 % is deliberate - this one self-oscillates. DelayCore's always-on ceiling
    // is what keeps that a howl rather than an overflow. Cross is near the top so the oscillation
    // keeps circulating between the two lines instead of building up inside one of them.
    { "howl", "HOWL",               true,   2,    0.0f, 105.0f, 2, 2,  0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 60.0f, 95.0f, 5000.0f, 50.0f, 55.0f, -3.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

/** INIT's index. **-1, deliberately outside the bank rather than position 0 within it.**

    INIT is the blank canvas you start from, not an authored sound competing with the eleven, so
    numbering it would push every Program down one and imply a running order it is not part of.
    Keeping it outside also means it never renumbers anything, so no saved session needs migrating.

    -1 is therefore a meaningful index here, and every "no index" sentinel in this casting has to be
    something else - see ProgramManager's pending-apply sentinel, which is -2. */
inline constexpr int initProgramIndex = -1;

/** The blank canvas: the delay present and audible in its plainest form, with everything that gives
    Fifth Member its character at zero.

    Three rules decide every value, and they are not the same rule:
      - **Character and amount go to zero** - Feedback, Wow, Flutter, Generation Loss, Mod Depth,
        Repeat Degrade, Saturation. Raise any one and you immediately hear what that one does.
      - **Structure goes to a usable middle, never zero** - Time 375 ms and Mod Rate 1.0 Hz. Both
        are *exactly* the centre of their knobs, and not by coincidence: `Parameters.h` sets each
        range's skew with `setSkewForCentre(375.0f)` and `setSkewForCentre(1.0f)` respectively. A
        delay at its 1 ms minimum is not neutral, it is a comb filter.
      - **Anything meaning "not acting" takes whatever value that is** - Damping at its 16 kHz
        ceiling, so the filter is open and repeats do not darken; Trim 0 dB.

    **This is the one Program that carries a value in every field, including the inactive paths.**
    Every other Program zero-fills what it does not own, and `Tests/FactoryProgramsTests.cpp`
    asserts exactly that - a value in an inactive slot is a bug there, because the panel's knobs
    persist across Program changes the way physical knobs do.

    INIT is the deliberate exception, and the reason is what it is FOR. Loading it with the
    active-path filter would leave Repeat Degrade at 80 % because that is where the last Digital
    Program left it, so switching to Digital from a "blank canvas" would land on someone else's
    sound. A canvas that is only blank along the path you happen to be on is not blank. So INIT
    applies all seventeen parameters unconditionally - see ProgramManager::applyInitProgram, which
    exists for this and nothing else.

    **Mix is 50 %.** Fifth Member is a wet/dry effect, and the midpoint reads as "nothing decided
    yet" where a value like 35 % would look like a judgement someone made. The two serial castings,
    TapeRot and Elmer, sit at 100 % for the opposite reason. */
inline constexpr FactoryProgram kInitProgram
    //          sync  div   time    fb     st ch   wow    flut   gen    rate   depth  degr   cross  damp      sat    mix    trim
    { "init", "INIT",   true, 2,    375.0f, 0.0f,  1, 0,   0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  50.0f, 16000.0f, 0.0f,  50.0f, 0.0f };

/** Loaded on instantiation and whenever no saved session state exists. */
inline constexpr int defaultFactoryProgramIndex = 0;
