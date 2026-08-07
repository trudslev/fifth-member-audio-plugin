# Fifth Member

A tempo-synced stereo delay, model **DL-88**, built with JUCE 8 for macOS (AU, VST3, Standalone),
Windows (VST3, Standalone) and Linux (VST3, Standalone). The fifth casting under the
[Neon Foundry](../BRAND.md) umbrella, sibling to [TapeRot](../taperot),
[Gatecrasher](../gatecrasher), [CHORUS-60](../chorus-60) and [Reflect-84](../reflect-84), and
dependent on none of them.

The panel is road-worn touring gear: a black rack unit that has lived on a truck. Brushed aluminium
rack ears, corner wear rubbed through to bare metal, a gaffer-tape nameplate hand-lettered in
marker, cable-tape labels, stencil stamps.

Three delay characters share one reconfigurable engine — a tape echo whose Wow, Flutter and
Generation Loss compound with every pass through the feedback loop, a fixed-character BBD
bucket-brigade, and a digital line whose quantisation accretes. Because every character stage sits
*inside* the recirculation path, repeat six is measurably more degraded than repeat one rather than
carrying the same fixed dose.

## Parameters

| Parameter | Range | Default |
|---|---|---|
| Sync | on / off | on |
| Note Division | 1/4 · 1/8. · 1/8 · 1/8T · 1/16 | Dotted 1/8 |
| Time | 1 – 2000 ms | 375 ms |
| Feedback | 0 – 110 % | 35 % |
| Stereo Mode | Mono / Stereo / Ping-Pong | Ping-Pong |
| Cross-Feed | 0 – 100 % | 80 % |
| Delay Character | Tape / BBD / Digital | Tape |
| Wow · Flutter · Generation Loss | 0 – 100 % | 25 / 20 / 30 % |
| Mod Rate · Mod Depth | 0.1 – 5 Hz · 0 – 100 % | 0.6 Hz · 20 % |
| Repeat Degrade | 0 – 100 % | 20 % |
| Damping | 1 – 16 kHz | 6 kHz |
| Saturation | 0 – 100 % | 15 % |
| Mix | 0 – 100 % | 35 % |
| Output Trim | ±24 dB | 0 dB |

Feedback above 100 % is deliberate — it self-oscillates, and an always-on soft ceiling keeps that a
howl rather than an overflow.

## Programs

Eleven factory Programs: You too?, Sky Wide, New Year's, Great Gig, Dark Echoes, Long Loop, Slow
Build, Slap Happy, Doubled Up, Sixteenth Sense, Howl. Click the LCD for the menu. SAVE is disabled
until something on the Program's own path moves, DELETE is disabled for factory Programs, and
saving always creates a new Program rather than overwriting one — including under a reused name.

**A Program stores only what is on its active path.** Cross-Feed, the timing control Sync is not
using, and the two Delay Character modes you are not in all persist independently across Program
changes, the way a physical knob keeps its position regardless of which patch is recalled.

They are called Programs, never Presets, per [BRAND.md](../BRAND.md).

## Building

See [BUILDING.md](BUILDING.md). In short:

```sh
cmake -B build -G Xcode          # macOS; -A x64 on Windows; -DCMAKE_BUILD_TYPE=Release on Linux
cmake --build build --config Release
./build/Tests/FifthMemberTests_artefacts/Release/FifthMemberTests
```

JUCE is fetched automatically.

## Project layout

```
Source/
  Parameters.h            Parameter IDs, APVTS layout, and ActivePath - the definition the
                          Program data model rests on
  PluginProcessor.*       Signal chain, metering, state
  PluginEditor.*          Fixed-canvas scaling shell
  DSP/
    TimingEngine.*        Host BPM/PPQ, free-run fallback, transport-discontinuity detection
    DelayCore.*           The feedback loop, routing, cross-feed, safety ceiling
    CharacterEngine.*     One double-buffered engine, three parameter sets
    ProgramManager.*      Factory and user banks, active-path filtering
    FactoryPrograms.h     The eleven Programs
  GUI/
    FifthMemberTheme.h    Every colour, coordinate and typographic constant
    PanelBackground.*     The baked static layer
    FifthMemberKnob.*     Knob body and the display-proportion animation hook
    RepeatTimelineScope.* The readout strip and the pulse plot
    PanelControls.*       Switch, buttons, LEDs, and the fixed three-dial label stacks
    ProgramHeader.*       LCD, bank tag, menu, SAVE/DELETE, meters
Tests/                    JUCE UnitTest console app
design/                   The approved spec, the live prototype, and the embedded fonts
prompts/PROMPTS.md        Numbered work packages
```

## Status

See [CLAUDE.md](CLAUDE.md). Everything is implemented; the GUI has not had a side-by-side pass
against the prototype, and neither the DSP tuning nor the Program bank has been listened to.
