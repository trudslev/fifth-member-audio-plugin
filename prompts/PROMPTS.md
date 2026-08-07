Convention: once a prompt below has been implemented, mark it SHIPPED with the date, e.g.
"PROMPT #1 - SHIPPED 2026-08-07".

PROMPT #1 - SHIPPED 2026-08-07

Scaffold Fifth Member, a tempo-synced stereo delay (model DL-88), as a new sibling plugin to
TapeRot, Gatecrasher, CHORUS-60 and Reflect-84 under neon-foundry/. Study TapeRot's Source/ layout,
DSP folder organization (one class per responsibility), Tests/ setup and BUILDING.md format, and
reuse Gatecrasher's Program storage architecture (Factory/User banks, save-always-creates-new,
FACT/USER in the PROGRAM LCD) rather than redesigning it. Determine the GUI rendering approach from
design/ itself rather than assuming which sibling it matches.

Two corrections to the brief were issued during planning and are the defining constraints of the
build. CORRECTION 1: a Program stores only the parameters on its active path; where a mutually
exclusive selector chooses between alternatives, the non-selected alternatives are not Program
state and persist independently across Program changes. This is deliberately unlike CHORUS-60's
every-combination model. CORRECTION 2: no knob ever dims, darkens, greys out or goes visually
inert, anywhere on the panel, under any state - only LEDs change.

Delivered: full project scaffold (CMakeLists.txt, .gitignore, README.md, BUILDING.md, CLAUDE.md,
release workflow); Parameters.h with all 17 parameters plus ActivePath as the single definition
Correction 1 rests on; the DSP chain (TimingEngine reading host BPM/PPQ with a free-running
fallback and transport-discontinuity detection - the first tempo-aware code in the suite; DelayCore
owning the feedback loop, Mono/Stereo/Ping-Pong routing, the Cross-Feed matrix and an always-on
safety ceiling; CharacterEngine as one double-buffered reconfigurable engine with three parameter
sets, sitting inside the loop so degradation compounds); ProgramManager + FactoryPrograms.h with
the 11 factory Programs and active-path filtering throughout; the GUI (FifthMemberTheme,
PanelBackground with its baked static layer, FifthMemberKnob with CHORUS-60's display-proportion
slew, RepeatTimelineScope, PanelControls carrying the fixed three-dial mechanic and the three
conditional LEDs, ProgramHeader); and a JUCE-UnitTest suite of 29 groups whose headline cases are
the Correction 1 contract and the compounding-degradation guard.

Three errors in design/README.md were found and corrected during implementation: the fascia is
1100px not 1084 (the doc subtracts one rack ear twice), the panel height is never stated and
resolves to 855, and Permanent Marker and Special Elite are Apache 2.0 rather than the OFL the type
table claims.

PROMPT #2

Do the side-by-side GUI fidelity pass. Open design/Fifth Member.dc.html in a browser next to the
built Standalone and compare region by region - there are no reference screenshots in this bundle,
so the prototype is the acceptance target. Two visual passes have already been done from
screenshots alone and each found real defects (readout text drawn inside the plot zone, LED glows
painting over adjacent labels, tick marks reading as spokes, knob faces reading flat), so treat
"it looks about right" as insufficient. Known candidates: gradient weighting on the knob faces,
corner-wear balance, nameplate marker weight, and text metrics throughout.

PROMPT #3 - SHIPPED 2026-08-07

Build the product icon. design/README.md Part 2 specifies it fully: concept 1b "repeat train", a
256x256 master with six bars on a baseline inside an LCD window, exact heights 112/74/48/30/18/10,
and a separate simplified master for 16/24/32 that drops the readout and the phosphor bloom.
IconPulse.dc.html is the primary artifact. Wire the result to ICON_BIG/ICON_SMALL.

Shipped: 1024 and 256 masters in design/icons/, wired to ICON_BIG/ICON_SMALL. AppIcon.icns carries
ic10 (1024) and ic08 (256), verified by reading it back out of the built .app.

The doc's separate simplified <=32px master was settled by dropping the readout from BOTH masters
instead - JUCE exposes only two icon slots, so there was nowhere to put a third. 24/32/48 resolve
cleanly; 16px is a wedge and stays one.

PROMPT #4

Tune the DSP and the factory bank by ear. Delay tables, filter cutoffs and character ranges are a
structurally-reasoned first pass; the 11 Programs carry deliberate values but nothing has been
listened to. Keep Tests/DelayCoreTests.cpp's compounding-degradation case passing throughout - it
is the guard on the premise that Tape, BBD and Digital each degrade progressively rather than
applying a fixed dose.
