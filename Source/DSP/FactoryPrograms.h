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

    Concretely, for every row: exactly one of `division` / `timeMs` carries data, and exactly one of
    the three character groups does. Cross-Feed appears nowhere - it is never Program state.

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

    float dampingHz, saturationPercent, mixPercent, trimDb;
};

inline constexpr std::array<FactoryProgram, 11> kFactoryPrograms { {
    //  name                sync  div  timeMs    FB  st  ch     wow  flut  gen    rate  depth  degr   damp    sat   mix   trim
    { "YOU TOO?",           true,   1,    0.0f, 35.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.6f, 20.0f,  0.0f, 6000.0f, 15.0f, 35.0f,  0.0f },
    { "SKY WIDE",           true,   1,    0.0f, 45.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.5f, 15.0f,  0.0f, 8000.0f, 10.0f, 40.0f,  0.0f },
    { "NEW YEAR'S",         true,   2,    0.0f, 40.0f, 2, 1,   0.0f, 0.0f, 0.0f,  0.7f, 25.0f,  0.0f, 7000.0f, 18.0f, 38.0f,  0.0f },
    { "GREAT GIG",         false,  -1,  550.0f, 55.0f, 1, 0,  30.0f, 20.0f, 40.0f, 0.0f,  0.0f,  0.0f, 4500.0f, 30.0f, 45.0f,  0.0f },
    { "DARK ECHOES",       false,  -1,  480.0f, 60.0f, 1, 0,  35.0f, 25.0f, 50.0f, 0.0f,  0.0f,  0.0f, 3500.0f, 35.0f, 40.0f,  0.0f },
    { "LONG LOOP",         false,  -1,  900.0f, 70.0f, 1, 0,  20.0f, 12.0f, 35.0f, 0.0f,  0.0f,  0.0f, 5000.0f, 20.0f, 50.0f,  0.0f },
    { "SLOW BUILD",        false,  -1,  700.0f, 65.0f, 1, 0,  22.0f, 14.0f, 38.0f, 0.0f,  0.0f,  0.0f, 5500.0f, 22.0f, 42.0f,  0.0f },
    { "SLAP HAPPY",         true,   4,    0.0f, 15.0f, 0, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 10.0f, 10000.0f, 8.0f, 25.0f,  0.0f },
    { "DOUBLED UP",         true,   4,    0.0f, 10.0f, 1, 2,   0.0f, 0.0f, 0.0f,  0.0f,  0.0f,  5.0f, 9000.0f,  5.0f, 20.0f,  0.0f },
    { "SIXTEENTH SENSE",    true,   4,    0.0f, 50.0f, 2, 1,   0.0f, 0.0f, 0.0f,  1.2f, 30.0f,  0.0f, 6500.0f, 15.0f, 40.0f,  0.0f },
    // Feedback above 100 % is deliberate - this one self-oscillates. DelayCore's always-on ceiling
    // is what keeps that a howl rather than an overflow.
    { "HOWL",               true,   2,    0.0f, 105.0f, 2, 2,  0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 60.0f, 5000.0f, 50.0f, 55.0f, -3.0f },
} };

inline constexpr int kNumFactoryPrograms = (int) kFactoryPrograms.size();

/** Loaded on instantiation and whenever no saved session state exists. */
inline constexpr int defaultFactoryProgramIndex = 0;
