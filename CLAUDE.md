# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

Fifth Member is its own independent repo and does not depend on `../taperot/`, `../gatecrasher/`,
`../chorus-60/` or `../reflect-84/` at runtime or at build time — it is a sibling casting under the
shared [Neon Foundry](../BRAND.md) umbrella, and those repos are read purely as structural
reference. Read `../BRAND.md` first for the cross-plugin design system (naming, "Program" not
"Preset", the one-accent-colour rule, component grammar), then this file.

`design/README.md` is the authoritative GUI spec and `design/Fifth Member.dc.html` is the live
prototype — per the doc, "the authority wherever this document is ambiguous". Both were written
before implementation and are meant to be implemented as-is, not redesigned. **Three errors in that
document are corrected in this build and documented below; do not "fix" the code back to match
them.**

## Commands

Fifth Member builds on macOS (AU + VST3 + Standalone), Windows (VST3 + Standalone) and Linux
(VST3 + Standalone) — AU is Apple-only. JUCE 8.0.14 is fetched automatically via CMake
`FetchContent`, no local checkout needed.

Configure once — macOS: `cmake -B build -G Xcode`. Windows: `cmake -B build -A x64`. Linux
(single-config generator, so the build type must be set here): `cmake -B build
-DCMAKE_BUILD_TYPE=Release`. Re-run configure whenever `CMakeLists.txt` changes.

Build: `cmake --build build --config Release`. Tests:
`./build/Tests/FifthMemberTests_artefacts/Release/FifthMemberTests`.

See [BUILDING.md](BUILDING.md) for per-platform requirements and `auval`/pluginval commands.

## Prompts log

`prompts/PROMPTS.md` holds numbered work-package prompts. Once a prompt is fully implemented, mark
it `SHIPPED` with the date, e.g. `PROMPT #1 - SHIPPED 2026-08-07`.

## Architecture

### Signal chain (fixed order, all in `PluginProcessor::processBlock`)

```
in --+------------------------------- dry ----------------------------+
     |                                                                 |
     |  TimingEngine -> delayMs                                        |
     |         |                                                       |
     +-> DelayCore  [ read -> CharacterEngine -> Damping -> Sat ] -----+-> wet
              ^                                              |
              +--- feedback, routed per Stereo Mode ---------+
                                                              |
                            Mix ------------------------------+-> Trim -> out
```

**Every character stage lives inside `DelayCore`'s feedback loop.** That is what makes degradation
compound: repeat six has been through the filter, the saturator and the character engine six times.
Move any of them onto the output and every repeat gets an identical fixed dose instead —
`Tests/DelayCoreTests.cpp`'s "Tape degradation COMPOUNDS" case is the guard on exactly that, and it
should not be weakened to make a change pass.

### The Program data model — the thing most likely to be got wrong

**A Program stores only the parameters on its active path.** Where a mutually exclusive selector
chooses between alternatives, the non-selected alternatives are not Program state at all — they
persist independently across Program changes, the way a physical knob keeps its position regardless
of which patch is recalled.

| Always stored | Conditionally stored | Never stored |
|---|---|---|
| Sync, Feedback, Stereo Mode, Delay Character, Damping, Saturation, Mix, Output Trim | Note Division *(iff Sync on)* · Time *(iff Sync off)* · Wow+Flutter+Generation Loss *(iff Tape)* · Mod Rate+Mod Depth *(iff BBD)* · Repeat Degrade *(iff Digital)* | Cross-Feed |

`ActivePath` in `Source/Parameters.h` is the single definition. Everything downstream derives from
it, and four consequences are easy to break:

1. **All 17 parameters exist in the APVTS at all times.** Hosts need stable automation lanes. Only
   *which subset a Program writes* varies.
2. **Applying a Program writes only its subset.** For User Programs that rules out
   `apvts.replaceState()` — the way Gatecrasher does it — because that would clobber every
   persisting parameter. They serialise the filtered subset and apply it attribute by attribute.
3. **The dirty check compares only the active path.** Otherwise moving Cross-Feed lights SAVE and
   saving produces a Program indistinguishable from the loaded one. The path is recomputed from the
   live Sync/Character state each time, because flipping either changes what is being compared.
4. **`getStateInformation` still persists the whole APVTS.** Session state and Program state are
   different things: the persisting knobs must survive a reload even though no Program owns them.

`FactoryPrograms.h` zero-fills every field a Program does not own, and
`Tests/FactoryProgramsTests.cpp` asserts it per Program — so the rule is structurally checkable
rather than a convention someone will later "complete" by filling in the blanks.

**This is deliberately unlike CHORUS-60**, whose engine I and II are independent toggles combinable
in place, so every combination had to be pre-baked into every Program. Fifth Member's selectors are
exclusive switches, closer to Gatecrasher's Algorithm knob: switching is "now doing something else",
not "adding another simultaneous state". Do not carry CHORUS-60's every-combination invariant here.

### DSP (`Source/DSP/`)

One class per responsibility, each `prepare(ProcessSpec)` / `reset()` / `process(...)` taking plain
scalars. No DSP class reads the APVTS; the processor converts once and passes values down.
`ProgramManager` is the one exception, because APVTS manipulation is its whole job.

- **`TimingEngine`** — the first tempo-aware code in the suite. No sibling reads a play head for
  anything but play/stop, so there is no house pattern to copy beyond TapeRot's null-guard idiom.
  Free-running fallback holds the last known good BPM (initially 120) when there is no play head or
  the host reports no tempo. A transport discontinuity is **backwards PPQ motion of any size**, or a
  forward skip of several blocks — not a single tolerance band, which misses short loops entirely
  because a two-bar loop wrapping moves PPQ back by less than one block's worth of beats. On a jump
  the *scope* re-anchors; the delay line is deliberately **not** flushed, because hard-locking
  repeats to the grid would cut the tail dead on every loop wrap.
- **`DelayCore`** — the loop, the routing and the safety ceiling. Ping-Pong injects the input into
  the left line only; Cross-Feed sets how much of each line's output crosses rather than self-feeds.
  At cross 0 the right line is **silent** — it is not equivalent to Stereo, which feeds each line
  from its own input channel.
- **`CharacterEngine`** — one reconfigurable engine with three parameter sets, not three parallel
  engines behind a selector. It is **double-buffered**: a mode change cross-fades over ~40 ms with
  the outgoing configuration running on its own state, because running both against one set of
  filters would double-filter for the length of the ramp.

**A gain-staging trap worth knowing.** `tanh(x*d)/tanh(d)` is unity only at *full scale*; below it
the gain is `d/tanh(d)`, which at drive 7 is a factor of 7. Inside a feedback loop that silently
multiplies the feedback coefficient and a nominally 80 % patch runs away. Both saturation points use
`tanh(x*d)/d`, which is unity for small signals at any drive. Gatecrasher's `SlamSaturation` carries
a comment about the same trap.

Feedback reaches 110 % by design and Program 11 (`HOWL`) ships at 105 %, so `DelayCore` carries an
always-on soft ceiling **independent of the Saturation control**. Without it, self-oscillation at
Saturation 0 % has nothing bounding it.

### GUI (`Source/GUI/`)

Entirely vector, with the static layers baked once into a `juce::Image` at construction. The
prototype is 100 % CSS with zero images, zero SVG and no assets at all — unlike Gatecrasher and
CHORUS-60, whose designers supplied panel plates and 128-frame knob filmstrips — so there was
nothing to pre-render from but the CSS itself.

`PanelBackground` bakes at **2×**, not 1:1. The panel's two defining textures — the fascia's 3 px
brush and the rack ears' 2 px one — sit at the resolution limit, so a 1:1 bake blitted to a Retina
display resolves them to a flat wash and the metal stops reading as metal. ~17 MB, and it holds up
across the 0.5×–2× resize range.

`FifthMemberTheme.h` holds every colour, coordinate and typographic constant. **Every coordinate in
`Layout` is absolute against the 1240 × 848 canvas.** The CSS they came from is nested — the fascia
is a flex child after the 52 px left rack ear, and its own 18 px padding starts the content box — so
the extracted figures were fascia-relative and have had `earWidth` folded in once. A fascia-relative
number added later lands 52 px left of where it belongs, on top of the rack ear.

**Three corrections to `design/README.md`, all load-bearing:**

1. **The fascia is 1100 px, not the doc's 1084.** That figure subtracts one rack ear twice
   (1240 − 52 − 52 − 52). The unit is 52 + 1136 + 52, and the fascia's content box is 1136 − 36 px
   padding = 1100 at origin x = 18.
2. **The panel is 1240 × 848.** The height is never stated in the doc. Summing the CSS gives ~854.5,
   but that over-counts every line box; measured off the rendered prototype it is 848.
3. **Permanent Marker and Special Elite are Apache 2.0, not OFL** as the doc's type table claims.
   Both are vendored with `LICENSE.txt` accordingly.

Two absolutes:

- **No knob ever dims, darkens, greys out or goes inert** — under any mode, switch state or Program.
  The body renders identically at all times; only LEDs change. CHORUS-60's `dimFactor` is
  deliberately not ported. (The prototype agrees: it computes an `op: 0.32` for "off" knobs and
  never binds it. Dead code.)
- **No variable text in the scope's plot zone.** Every readout lives in the 22 px strip at the top
  of the dark box, in Share Tech Mono — the same face as the PROGRAM LCD. That combination of
  placement and typeface is what makes changing text read as a screen rather than a printed label.

**The fixed three-dial mechanic.** Three positions, always present, never appearing, disappearing or
moving. Each carries every label it could ever have, permanently printed and stacked, each with its
own 6 px LED. A dial with no label in the current mode keeps editing its own parameter — Dial 3
always drives Generation Loss — rather than falling back to the current mode's first parameter,
which would make it a duplicate of Dial 1.

**Knob animation.** `SliderAttachment` still sets parameters instantly; a `setDisplayProportion()`
override is slewed by the editor's 60 Hz timer using `1 − 0.002^(dt/settle)`. Three traps CHORUS-60
paid for: slew in **rotation** proportion (not parameter value — they differ for skewed parameters
like Time, Damping and Mod Rate); **create knob components once and re-attach**, never recreate per
mode, or the animation silently degrades to a snap; and a dragged knob tracks the pointer 1:1.

On a Delay Character change the dials are held at minimum for ~40 ms and released, so all three
visibly sweep up — the design calls this "the panel physically re-setting itself".

**CSS-to-JUCE traps this panel has already paid for.** Every one of these was caught by rendering
`design/Fifth Member.dc.html` in headless Chrome and diffing region by region — do that before
adjusting anything here by eye:

```
"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" --headless --disable-gpu \
  --force-device-scale-factor=2 --window-size=1320,1300 --virtual-time-budget=8000 \
  --screenshot=proto.png "file://$PWD/design/Fifth Member.dc.html"
```

- **`font-size` is an em size.** Use JUCE 8's `FontOptions::withPointHeight()`, never `withHeight()`.
- **`juce::String`'s `const char*` constructor decodes Latin-1, not UTF-8.** Build `·` from its
  codepoint (`Text::middleDot()`).
- **`width` on a bordered, padded box is the CONTENT width** — the prototype has no `box-sizing`
  reset. This bit twice: the fascia (1136 border box, 1100 content) and DELAY CHARACTER's right
  column (`width:250px` + 26 px padding + 1 px border = 277 border box). The second one walked a
  divider 27 px right and, because the middle column is `flex:1`, stretched the three dials onto a
  pitch 9 px too wide.
- **CSS interpolates translucent gradient stops in premultiplied space; JUCE does not.** A single
  JUCE gradient from `rgba(255,255,255,.022)` to `rgba(0,0,0,.18)` carries a mid grey at partial
  alpha and lightens the middle of the box by six levels. Draw light-fading-out and dark-fading-in
  as two passes.
- **A gradient is clipped by the element that carries it.** The corner wears are 120×70-ish boxes;
  filled across the full fascia column instead they hazed the whole panel several levels lighter.
- **Gradient stop percentages are of the gradient's own radius, not the box's.** The knob tick masks
  quote 64 % and 76 % of the ring box's *farthest-corner* radius (`ringHalf × √2`) — read as
  fractions of the box radius the marks come out a third short and read as spokes on the body.
- **A `linear-gradient` angle is a direction, not a diagonal.** `96deg` on a 52 × 848 rack ear runs
  across the 52 px width; drawn corner to corner it is almost vertical and every band of the brushed
  figure disappears. `92deg` on the brush texture leans the marks 2° off vertical, which over 848 px
  walks them 30 px sideways — that near-miss against the pixel grid is the shimmer.
- **box-shadow blur radii are not gradient radii** — drawn as discs of colour they wash over adjacent
  labels, which is what happened to REPEATS LIVE, NOTE DIVISION, CROSS-FEED and TIME.

Judge a texture by local mean and local contrast, not by per-pixel difference: a 2 px brush that is
one pixel out of phase scores terribly and looks identical.

**The icon, and the one slot JUCE does not give us.** `ICON_BIG`/`ICON_SMALL` are the only two inputs
and every platform icon is derived from them: the macOS `.icns` takes one entry per file at the
largest exact size of 16/32/64/128/256/512/1024 that fits (so a non-power-of-two source silently
downgrades — 1000 px lands as 512), and the Windows `.ico` takes 16/32/48/256, each from the smallest
supplied image still big enough, which means all four come from the 256.

`design/README.md`'s "Small sizes" section asks for a separate simplified master for 16/24/32 that
drops the readout line, because downscaled it turns into a muddy band across the bottom. JUCE has no
third icon slot, so that was settled the other way: **both masters are the readout-free "delay only"
art**, six decaying bars on a baseline and nothing else. 24 / 32 / 48 all resolve cleanly now. 16 px
still collapses to a wedge — six bars and five gaps cannot survive eleven usable pixels at any
weight — and Fifth Member is the softest of the five suite icons at that size. That is accepted, not
outstanding; it shows up only in Windows Explorer's small-icon view and the taskbar.

**Re-run configure after touching the artwork.** JUCE builds the `.icns` and `.ico` at *configure*
time and the PNGs are not configure dependencies, so `cmake --build` alone ships the previous icon
silently. See BUILDING.md for the check that reads back what the bundle actually carries.

### Build system

JUCE pinned to `8.0.14`, matching all four siblings. `PLUGIN_MANUFACTURER_CODE` (`Nfdy`),
`PLUGIN_CODE` (`Fm88`), `BUNDLE_ID` (`com.neonfoundry.fifthmember`) and `COMPANY_NAME` are settled —
changing them breaks saved projects in both AU and VST3, since JUCE derives the VST3 class ID from
the manufacturer and plugin codes together.

The CMake target is `FifthMember` (targets cannot contain a space) while the product is
`Fifth Member`. Two separate name axes; anything deriving a path from the wrong one looks in a
directory that does not exist.

`Tests/` compiles the DSP `.cpp` files directly — **a new DSP `.cpp` goes in both target_sources
lists**. `Tests/TestMain.cpp` creates a `ScopedJuceInitialiser_GUI`: without a MessageManager,
`AsyncUpdater::triggerAsyncUpdate()` silently clears its own pending flag and every Program test
passes while proving nothing.

## Status

- **DSP**: complete, no stubs. Delay tables, filter cutoffs and character ranges are a
  structurally-reasoned first pass, not a by-ear one. Build, load, listen, adjust.
- **Programs**: 11 factory Programs with authored values; no by-ear pass.
- **GUI**: complete, and measured against the prototype rendered in headless Chrome rather than
  judged by eye. Whole-panel tone difference is ~3.4 levels out of 255; every panel-box edge, both
  DELAY CHARACTER dividers and the scope's inner box land on the reference to within a pixel.
- **Icon**: concept 1b "repeat train" in its readout-free "delay only" form, wired to `ICON_BIG`
  (1024) and `ICON_SMALL` (256) from `design/icons/`. Verified out of the shipped bundle, not just
  off disk.
- **Not done**: registration in `../manifest/suite.toml`, deliberately held until all six suite
  plugins exist (Elmer, a bus glue plugin, is unbuilt). It will also need a tagged release and a
  freshly generated `windows_appid` GUID.
