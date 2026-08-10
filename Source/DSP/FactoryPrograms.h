#pragma once

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
struct FactoryProgram
{
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
    { "YOU TOO?",           true,   1,    0.0f, 35.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.6f, 20.0f,  0.0f, 80.0f, 6000.0f, 15.0f, 35.0f,  0.0f },
    { "SKY WIDE",           true,   1,    0.0f, 45.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.5f, 15.0f,  0.0f, 90.0f, 8000.0f, 10.0f, 40.0f,  0.0f },
    { "NEW YEAR'S",         true,   2,    0.0f, 40.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.7f, 25.0f,  0.0f, 80.0f, 7000.0f, 18.0f, 38.0f,  0.0f },
    { "GREAT GIG",         false,  -1,  550.0f, 55.0f, 1, 0,  30.0f, 20.0f, 40.0f, 0.0f,  0.0f,  0.0f,  0.0f, 4500.0f, 30.0f, 45.0f,  0.0f },
    { "DARK ECHOES",       false,  -1,  480.0f, 60.0f, 1, 0,  35.0f, 25.0f, 50.0f, 0.0f,  0.0f,  0.0f,  0.0f, 3500.0f, 35.0f, 40.0f,  0.0f },
    { "LONG LOOP",         false,  -1,  900.0f, 70.0f, 1, 0,  20.0f, 12.0f, 35.0f, 0.0f,  0.0f,  0.0f,  0.0f, 5000.0f, 20.0f, 50.0f,  0.0f },
    { "SLOW BUILD",        false,  -1,  700.0f, 65.0f, 1, 0,  22.0f, 14.0f, 38.0f, 0.0f,  0.0f,  0.0f,  0.0f, 5500.0f, 22.0f, 42.0f,  0.0f },
    { "SLAP HAPPY",         true,   4,    0.0f, 15.0f, 0, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 10.0f,  0.0f, 10000.0f, 8.0f, 25.0f,  0.0f },
    { "DOUBLED UP",         true,   4,    0.0f, 10.0f, 1, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f,  5.0f,  0.0f, 9000.0f,  5.0f, 20.0f,  0.0f },
    { "SIXTEENTH SENSE",    true,   4,    0.0f, 50.0f, 2, 1,   0.0f, 0.0f, 0.0f,  1.2f, 30.0f,  0.0f, 75.0f, 6500.0f, 15.0f, 40.0f,  0.0f },
    // Feedback above 100 % is deliberate - this one self-oscillates. DelayCore's always-on ceiling
    // is what keeps that a howl rather than an overflow. Cross is near the top so the oscillation
    // keeps circulating between the two lines instead of building up inside one of them.
    { "HOWL",               true,   2,    0.0f, 105.0f, 2, 2,  0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 60.0f, 95.0f, 5000.0f, 50.0f, 55.0f, -3.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

/** Loaded on instantiation and whenever no saved session state exists. */
inline constexpr int defaultFactoryProgramIndex = 0;
