# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

Fifth Member is its own independent repo and does not depend on `../taperot/`, `../gatecrasher/`,
`../chorus-60/` or `../reflect-84/` at runtime or at build time — it is a sibling casting under the
shared [Neon Foundry](../BRAND.md) umbrella, and those repos are read purely as structural
reference. Read `../BRAND.md` first for the cross-plugin design system (naming, "Program" not
"Preset", the one-accent-colour rule, component grammar), then this file.

`design/BUILD-HANDOFF.md` and `design/GUI-SPEC.md` are the authoritative spec, and
`design/Fifth Member.dc.html` is the live prototype — **authoritative for pixel geometry, where the
spec documents are authoritative for values.** `design/README.md` is the earlier revision and is
superseded wherever they disagree. **Errors in those documents that this build corrects are listed
below; do not "fix" the code back to match them.**

## RESUME POINT — the header has a measured baseline, and it is what a panel move fails against

**Verified `ed3688a` on 2026-08-17: this casting's header draws exactly where its own constants say.**
Not read — captured from the Release standalone and measured off the pixels.

Why it is written here rather than left in a session report: the value of a baseline is entirely in
being read by whoever moves the panel next, and a figure that lives somewhere the mover does not
open is worth nothing.

**What was measured**, band at y 53, height 34, canvas 1240 x 931 — **the pre-rewrite panel**,
kept as the baseline it was taken as. The window is 1444 x 1012 on a 1340 frame and the band is
the shared part's at y 61 as of 2026-08-21; see the canvas section below:

| Element | Constants | Measured |
|---|---|---|
| LCD | `lcdX` 368, `lcdW` 449 -> 368..817 | 368.5 .. 815.5 |
| SAVE | `saveX` 827, `saveW` 68 -> 827..895 | 827.5 .. 847.0 (glyph-broken) |
| DELETE | `deleteX` 902 -> 902..970 | 902.0 .. 976.5 |
| meter wells | — | 1003.5 .. 1078.0, 1090.5 .. 1164.0 |

**What this is NOT.** This casting references `nf::HeaderGeometry` **nowhere**, so it is on its own
canvas and its own layout, and none of the figures above is expected to match the shared part. The
baseline says *internally consistent*, not *conformant*.

**The defect it exists to catch** was found in Chorus-60 on 2026-08-17: that casting's header pass
aliased its LCD to the shared part and left SAVE, DELETE and both meter wells as literals from the
previous canvas — **29 px right and 29 px down** — and nothing could see it, because the plate baked
those faces and the only symptom was text centred inside a box nobody drew. It surfaced the moment
the material had to be painted from those rects.

**So when this casting moves: alias every band figure in one edit, then re-measure against the table
above.** A rect that moves and a rect that does not are indistinguishable in a diff and obvious in a
measurement. And note that **a literal which happens to agree with core is indistinguishable from an
alias by reading** — Reflect-84 held four such literals, one of them 2 px off §4's shared descriptor
anchor, in the casting whose editor had been declared conformant.

---

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

| Always stored | Conditionally stored |
|---|---|
| Sync, Feedback, Stereo Mode, Delay Character, Damping, Saturation, Mix, Output Trim | Note Division *(iff Sync on)* · Time *(iff Sync off)* · Wow+Flutter+Generation Loss *(iff Tape)* · Mod Rate+Mod Depth *(iff BBD)* · Repeat Degrade *(iff Digital)* · **Cross-Feed** *(iff Ping-Pong)* |

**Nothing is in a "never stored" column any more, and the reason it was emptied is worth keeping.**
Cross-Feed sat there on the reasoning that it is a knob the player rides rather than part of a patch.
That could not survive its own interaction with Stereo Mode: Stereo Mode *is* always stored, so
recalling a Program recalls Ping-Pong — the one mode whose `DelayCore` branch reads the cross term —
without recalling the value that defines the bounce, and at cross 0 Ping-Pong leaves the right line
silent. A factory Program could therefore name a ping-pong effect it had no way to reproduce,
depending on where the user's knob happened to be sitting.

The general test that settles this class of question: **a parameter may be left out of a Program only
when the Program cannot recall a state in which that parameter is audible.** The conditional
exclusions pass it because their discriminators (Sync, Character, Stereo Mode) are themselves always
stored, so what a Program omits is exactly what is inaudible in the mode it recalls. Cross-Feed
failed it, and now passes by being conditional on the mode that uses it.

If the concern is only that a control shouldn't *light SAVE*, the lighter tool is Chorus-60's:
exclude it from the dirty check while still storing it, so Programs stay reproducible. Excluding it
from storage is the heavier one and needs the audibility test above.

`ActivePath` in `Source/Parameters.h` is the single definition. Everything downstream derives from
it, and four consequences are easy to break:

1. **All 17 parameters exist in the APVTS at all times.** Hosts need stable automation lanes. Only
   *which subset a Program writes* varies.
2. **Applying a Program writes only its subset.** For User Programs that rules out
   `apvts.replaceState()` — the way Gatecrasher does it — because that would clobber every
   persisting parameter. They serialise the filtered subset and apply it attribute by attribute.
3. **The dirty check compares only the active path.** Otherwise moving the inactive timing control
   or another mode's dial lights SAVE and saving produces a Program indistinguishable from the
   loaded one. The path is recomputed from the live Sync/Character/Stereo Mode state each time,
   because flipping any of them changes what is being compared — Cross-Feed dirties a Ping-Pong
   Program and not a Stereo one.
4. **`getStateInformation` still persists the whole APVTS.** Session state and Program state are
   different things: the persisting knobs must survive a reload even though no Program owns them.

**Schema 2** is this change. A schema-1 User Program simply has no `crossFeed` attribute, and the
apply loop walks the attributes the file actually has, so old files load with Cross-Feed left where
the player had it — the schema-1 behaviour exactly, and no migration step to write.

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

### THE CANVAS MOVED 2026-08-21 — 1240 × 931 → a **1444 × 1012 window on a 1340 frame**

**Read this before any figure below it.** Fifth Member was one of two castings never rebuilt to the
harmonisation spec; the shipping panel was the old design, not a stale binary. Everything in the GUI
section that follows was written against 1240 × 931 and its absolute coordinates have moved.

**This is the one casting whose WINDOW is wider than the shared frame, and §1 states call 1 as the
FRAME for exactly that reason.** Full-height rack ears flank the fascia — 52 px each side, outside
the frame — and they carry three of the panel's five identity marks: the **RACK 4 · MON WORLD**
stencil, the **DLY 4** cable tape and the **HALDEN HALL · LOAD-IN 06** tape. A 1308-wide block at
x 16 inside a 1340 window leaves zero for ears.

The alternative was 16 px bezels, which would have removed those three marks to protect a window
dimension — inverting the round's scope, where the header is the part and body layout is the
casting's. **Named cost: this window is 104 px wider than the other five.** Right at 1444 against
1340; it would not have been at 960.

| | |
|---|---|
| Window | **1444 × 1012** — `Layout::canvasWidth/canvasHeight` |
| Frame | **1340** at x **52** — `Layout::frameWidth` is `nf::HeaderGeometry::canvasWidth`, aliased |
| Ears | 52 each side, full height, **drawn** — `PanelBackground::paintEar` |
| Header block | frame-local 16, 16, 1308 × 104 — the shared part |

**Every absolute coordinate in `Layout` has `frameX` folded in, once.** `nf::HeaderGeometry` speaks
frame-local, so anything taken from it lands 52 px right here. A frame-local number added later
lands on top of the left ear — the bug the pre-existing version of that comment already warned
about, and which a wider frame does not change.

#### The plate is the FRAME, not the window — and it stopped carrying the ears

`design/plate/fifth-member-plate-3x.png` is **4020 × 3036 = 1340 × 1012**, blitted at (`frameX`, 0).
The 2× cut it replaces was 2480 × 1862 = 1240 × 931 and its own manifest read *"the fascia and its
wear, **the rack ears and screws**, the nameplate…"* — the re-cut drops the ears, because a plate
covering the window would have to be 1444 wide and would then disagree with every other casting's
1340.

**Verified against the prototype before anything was built on it**, by scanning the plate for its
own edges rather than trusting the coordinates: the scope well steps at frame-local **160.00** and
**258.00**, its readout rule at **183.00**, and the control-band top at **274.00** — every one the
prototype's figure to the pixel. That check is what made deriving the layout from the prototype safe.

#### The nameplate stack answered its row of an open ask

`design-asks/header-nameplate-offsets.md` recorded this casting's published §4 stack as
`30 + 34 + 9 = 73`, **five short** of the shared descriptor anchor at 78. The delivered prototype
draws the tape's bounding box at 268.8 × 45.6 rotated −1.2°, which is a **268.2 × 40** strip —
and **30 + 40 + 8 = 78 exactly**. The published height was measured on something other than the
tape. Pinned by `static_assert`; §4's row wants 34 → 40.

TapeRot's row answered itself the same way on the same day, from its wordmark cut. Two of the three
open rows closed by measuring the delivered artwork rather than by a ruling.

#### The Program legends were never moved onto the shared part, and `check_contrast` had been saying so

Two roles had been failing since whenever they drifted — `legendLit` stated 13.98 against a measured
**12.52**, `legendDark` stated 3.48-3.66 against **3.12-3.37**. Neither was a floor breach, which is
why nobody noticed: the panel looked right and only the stated figure was wrong.

**The cause was not drift.** 13.98 is `#f2ece0` against the header **block** `#211f1c` — a real
measurement, taken against the wrong ground, with the wrong ground named in the annotation beside
it. The legend sits on the button face, not on the block.

§6 gives this role as `#f4f8fa` on cap `#23282c` at **13.93** and idle `#9aa1a6` at **5.68** — the
shared part's inks, which Gatecrasher already carries. Both are now those, and SAVE and DELETE stop
having *different* faces from each other (`#2a2823` against `#242219`), a distinction the shared
part does not make and which made two buttons in one row read as two components.

`legendDark` had been sitting at **3.12 against a 3.0 floor**. It is 5.68 now.

#### What this pass did NOT re-derive

The band interiors were re-derived from the prototype — section boxes, knob pivots, button rows,
LEDs, the foot row — and verified by capture. **Individual figures inside `PanelControls` that the
plate bakes were not re-measured against the plate**, because §11's flag list says the plate carries
them and the build does not draw them. If something printed looks wrong, check whether the build is
drawing it at all before moving a constant: on this casting **redrawing something the plate carries
double-prints it at a one-pixel offset**, which is visible, and that is the failure mode a plate
keeps.

**The panel cannot be shown at 100 % on a 1440 × 932 display** — 1012 plus window chrome exceeds it,
so a capture at full size is silently clipped by the screen rather than by the code. Capture at
1155 × 810 (0.8×) to see the foot row. That cost one wrong diagnosis before it was noticed.

---

### GUI (`Source/GUI/`)

**Every absolute coordinate in this section predates 2026-08-21 and has moved.** The section above
is the current geometry; this remains for its reasoning, which is unaffected.

Entirely vector, with the static layers baked once into a `juce::Image` at construction. The
prototype is 100 % CSS with zero images, zero SVG and no assets at all — unlike Gatecrasher and
CHORUS-60, whose designers supplied panel plates and 128-frame knob filmstrips — so there was
nothing to pre-render from but the CSS itself.

`PanelBackground` bakes at **2×**, not 1:1. The panel's two defining textures — the fascia's 3 px
brush and the rack ears' 2 px one — sit at the resolution limit, so a 1:1 bake blitted to a Retina
display resolves them to a flat wash and the metal stops reading as metal. ~17 MB, and it holds up
across the 0.5×–2× resize range.

`FifthMemberTheme.h` holds every colour, coordinate and typographic constant. **Every coordinate in
`Layout` is absolute against the 1240 × 932 canvas**, and every one of them was measured by
instrumenting the prototype's own DOM rather than summed from the CSS. The CSS they came from is nested — the fascia
is a flex child after the 52 px left rack ear, and its own 18 px padding starts the content box — so
the extracted figures were fascia-relative and have had `earWidth` folded in once. A fascia-relative
number added later lands 52 px left of where it belongs, on top of the rack ear.

**Corrections this build carries, all load-bearing:**

1. **The fascia is 1100 px, not 1084.** That figure subtracts one rack ear twice
   (1240 − 52 − 52 − 52). The unit is 52 + 1136 + 52, and the fascia's content box is 1136 − 36 px
   padding = 1100 at origin x = 18.
2. **The panel is 1240 × 932**, not the build handoff's stated 996 and not the old 848. Measured by
   instrumenting the prototype's own DOM: the chassis renders at 1240 × 931.45. Every other figure
   the handoff states checks out against that render — ears 52, LCD 449 × 34, the text-driven bank
   cell at 75.17 against a stated 75.2, TIMING 296, OUTPUT 322, the 232 and 144 dial wrappers, the
   536 dial row — so only the height disagrees, and section 4.5's own absolute Ys are off by two
   *different* amounts (774 vs 704.45, 895 vs 802.45), which says they come from a different layout
   rather than a taller one. Raised with the designers. **Re-measure rather than trusting either
   number** if a corrected prototype arrives.
3. **Output Trim is −24…+24 dB**, and handoff section 4.4 prints a −24…+12 ring. On the real range
   that ring puts 0 dB at +45° where the pointer reaches it at 0. The parameter is right; the table
   is recut in `Layout::trimMarks` and flagged.
4. **Permanent Marker and Special Elite are Apache 2.0, not OFL.** Both are vendored with
   `LICENSE.txt` accordingly.

Two absolutes:

- **No knob ever dims, darkens, greys out or goes inert** — under any mode, switch state or Program.
  The body renders identically at all times; only LEDs change. CHORUS-60's `dimFactor` is
  deliberately not ported. (The prototype agrees: it computes an `op: 0.32` for "off" knobs and
  never binds it. Dead code.)
- **No variable text in the scope's plot zone.** Every readout lives in the 23 px strip at the top
  of the dark box, in Share Tech Mono — the same face as the PROGRAM LCD. That combination of
  placement and typeface is what makes changing text read as a screen rather than a printed label.

**Printed scales, and the two things about them that are easy to break.** Every knob carries a
`ScaleMark` table and a tick is drawn at every printed numeral and nowhere else. What that replaced
was a fixed-pitch decorative ring — `roundToInt(360/27)` = 13 marks at 27° from 12 o'clock — which
put marks below the horizontal at both ends of the arc, printed one at +135° with no twin at −135°,
and left a 36° seam under the pointer at centre value.

  - **Angles are not stored.** They are computed from the bound parameter's own `NormalisableRange`
    at draw time, the same call the pointer uses, so a ring cannot drift from the taper it legends.
    `Tests/PrintedScaleTests.cpp` asserts every mark against the handoff's published angle and that
    no mark falls outside its ring's range.
  - **A knob is sized to its WRAPPER, not its tick tips.** Numerals live outside the ring and a
    component clips its own paint, so bounds taken from the tick annulus cut them off — and the
    failure is silent, because the ring still draws and the numerals simply vanish, which reads as
    the designer having omitted them. Section 4.2's 2R + 68, or the 232 box for dial 1.

**The fixed three-dial mechanic.** Three positions, always present, never appearing, disappearing or
moving. Each carries every label it could ever have, permanently printed and stacked, each with its
own 6 px LED.

**A dial with no label in the current mode drives NOTHING.** Dial 2 in Digital and Dial 3 in
BBD/Digital used to fall through to `flutter` and `genLoss`, so a Digital Program silently carried
tape values nothing on the panel named. They now bind to no parameter at all: the knob still turns —
a real control cannot lock, and section 2.2 forbids dimming it — but it turns a value stored
nowhere, and re-binding on the next mode change pulls it to that parameter's real value, which the
slew then animates.

**Dial 1 carries two concentric rings**, a percentage and a frequency, because it binds to Wow in
Tape, Mod Rate in BBD and Repeat Degrade in Digital and those cannot share numerals — 1 Hz sits at
0° where the percent ring prints 50. Both are permanently printed at different radii and each lights
or dims with the mode, exactly as the stacked labels do. **Each ring carries its own lo/hi/skew**:
only one matches the bound parameter at a time, and asking the Slider to map both put every percent
mark through the Hz range, clamping 25/50/75/100 onto a single stacked pile at +135°.

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

**`neon-foundry-core` is pinned at `v1.0.0`, declared AFTER `FetchContent_MakeAvailable(JUCE)`.**
That ordering is load-bearing: core links `juce::juce_core` and refuses to fetch its own when
consumed, and two JUCE trees in one build link two `juce_core` builds into one binary. It is linked
into **both** `FifthMember` and `FifthMemberTests`, because `ProgramManager.cpp` is compiled into
both and now calls into it.

What came from core, and what deliberately did not:

| From core | Stayed here |
|---|---|
| `nf::userProgramDirectory` — the per-OS path and the `Application Support` segment | The `.fifthmemberprogram` extension and the 26-character cap |
| `nf::UserProgramStore` — scanning, sorting, naming (`TAKE n`), collision, save, delete | **What a Program contains** — the active-path filter and the schema attribute |
| `nf::ParameterSnapshot` — the dirty baseline, keyed by parameter ID | **Which parameters it compares** — `currentActivePath()`, recomputed per call |
| `nf::ProgramId` / `nf::ProgramBank` / `programDisplayLabel` | The Factory bank, and resolving a slug to its position |
| `nf::UserEditGate` — the stale-replay guard, and `connectUserEdit`'s drag-guarded disarm | Which controls announce, and that the announcement names the parameter |

The split is the point: core owns files and names, this repo owns meaning. It is what lets Fifth
Member's filtered serialisation and the other five castings' `replaceState` model share one store.

**The clean snapshot now captures every parameter, not just the active path**, and the comparison is
what narrows. The answer is identical — the path is a function of Sync, Character and Stereo Mode,
all three of which are always compared, so a parameter can only be on the path when its
discriminator is where the Program left it — but a complete baseline cannot be misread as a missing
one. All 34 test suites pass **unedited** against the extracted model, which was the acceptance bar:
a test that needed changing would have meant the extraction changed behaviour.

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

### The Program list's group caption

**Sized from its own type plus padding, never derived from the row height.** The construction is
`nf::captionHeight (font, topPadding, bottomPadding)` — 3px above and 4px below, the suite's adopted
default — and it comes out **19px** here, from Share Tech Mono at 11px through `withPointHeight`.

**The construction is the rule, not the number.** Writing 19 as a literal would break silently at
the first change of font, size or font construction, which is a change nobody would think to check a
caption against. It is also how this caption came to inherit JUCE's `rowHeight + rowHeight / 2` in
the first place — a caption half again *taller* than a row, which is a menu convention rather than a
panel one.

`Tests/MenuMetricsTests.cpp` asserts the construction rather than the number, and logs the line
box rather than pinning it — the castings do not all build fonts the same way, so asserting one
ratio here would fail on a casting whose caption is correctly a different size.

### Case belongs at the source

`nf::ReadoutFormat::ValueCase` is deleted from core (2026-08-13). **A panel label reads in caps
because it is authored in caps in `Parameters.h`**, not because the readout upper-cased it on the
way out — the panel and the host's automation lane read the same parameter, so any re-casing in
between makes one of them lie about the other. Every parameter name here is authored in caps for
that reason, and `Tests/ReadoutConformanceTests.cpp` asserts it off `getName()`.

The rule is in `../BRAND.md` beside the unit rule; the suite-wide record is in the root
`../CLAUDE.md`. **The choice strings are deliberately NOT all caps** — this casting prints its
values as authored, and the rule is that case is decided at the source, not that everything is caps.

## Status

- **DSP**: complete, no stubs. Delay tables, filter cutoffs and character ranges are a
  structurally-reasoned first pass, not a by-ear one. Build, load, listen, adjust.
- **Programs**: 11 factory Programs with authored values. Program browsing, naming, saving and
  deleting follow TapeRot's paradigm, with two deliberate divergences where Fifth Member is the
  better implementation: `getNonexistentSibling()` so a name collision never overwrites, and
  cancel-on-focus-loss. The dropdown is additionally suppressed while naming — TapeRot opens it
  regardless, which applies a Program underneath a half-typed name.

  **The dropdown's pinning originated here and was ported back to the whole suite** — it hangs off
  the LCD's bottom edge at the LCD's width, never moves, and never outgrows the panel. The mechanism
  and the four `PopupMenu` traps behind it are in the root `CLAUDE.md` under "The Program dropdown";
  the local pieces are `menuHost` in `FifthMemberEditorContent` and `menuAnchorY`/`menuHostTop` in
  `ProgramHeader`. Verified with a 61-Program bank, which is what exposed the problem in the first
  place: as a free desktop window the list came out ~995 px tall over a 932 px panel.
- **Cross-Feed is Program state, but only in Ping-Pong** (schema 2). It is the one parameter whose
  storage is keyed on Stereo Mode rather than on Sync or Character, because that is the only mode
  whose `DelayCore` branch reads the cross term. See the data-model section above for the rule this
  settled — a parameter may be left out of a Program only when the Program cannot recall a state in
  which that parameter is audible.
- **GUI**: conformant to the revision-2 handoff. Every coordinate was re-measured by instrumenting
  the prototype's DOM rather than adjusted by eye. Printed scales replace the value tooltip as the
  at-rest reference; live values appear in the PROGRAM LCD while a control is moved and nowhere
  else. `auval` and `pluginval --strictness-level 8` pass on AU and VST3.

  **Register a lamp against the caption's printed ink, not against a label's layout box.** The
  captions are baked now, so the box the code once used to place them means nothing. Both live lamps
  were wrong on both axes because of it. Two habits that caught it: measure the caption in an
  x-window that stops at the caption rather than running into the next control's numerals — the
  window 660..900 looks like it isolates CROSS-FEED and also catches the "0" of the knob to its
  right, which sits ~9 px lower and drags the apparent centre — and sanity-check with **cap height**,
  because every caption here measures exactly 7.5 px and a "caption" reading 16.5 px tall is two
  things in one window. For horizontal spacing the panel's own reference is the character label
  stack: 7.5 px from lamp edge to first ink.
- **The Standalone needs `MICROPHONE_PERMISSION_ENABLED`**, which it now has. Without it macOS never
  grants capture access — no prompt, no error, an input stream of zeros — and the IN/OUT readouts sit
  at their floor looking like dead metering when they are simply being handed silence. Every `-99.0`
  in a screenshot from before that fix is this and not a meter bug.
- **The composite diff is against the plate now, which is the better target** — it removes the state
  problem entirely, since the plate has no live elements to differ on. The build paints **3.4 %** of
  the panel, and the regions are the knob bodies, the dials and the header text, i.e. section 1.2's
  runtime list.

  **Capture at exactly 1240 wide or the numbers lie**, and the standalone will not give you that by
  default: it restores its last window size — often ~870 wide — and macOS resamples the whole GUI to
  fit, so every edge differs slightly and a spanning "region" appears that is really the entire
  panel. That resampling was also the true source of the "1239 vs 1240 scale drift" this note used
  to describe; the drift was real, but its cause was the restored window size rather than anything
  inherent.

  So: set the size explicitly, **then confirm it took before measuring anything**. The window
  manager does not always honour the request — repeated resizes here have come back 1202 x 960 and
  1240 x 988 rather than the 1240 x 960 asked for, and a rerun under those is worthless. The check
  that settles it is correlating the capture against the plate: **residual 0.000 means true 1:1**,
  and anything above ~1 means it is scaled and no measurement off it can be trusted. Guard every
  capture on the app being frontmost too — an unguarded one silently grabs whatever window is in
  front, which is usually the editor you launched it from.
- **The canvas is 1 px shorter as of the 2026-08-11 bundle, and the code now matches.** The plate
  is **1240 × 931** at 1× — 2480 × 1862 and 3720 × 2793, all three exact multiples, so a deliberate
  cut rather than an export rounding — and `Layout::canvasHeight` is 931. It matters because
  `PanelBackground` blits `stretchToFit` into the canvas, so a 931-tall plate in a 932 canvas
  resampled the whole panel: a 0.107 % vertical stretch that blurs every baked edge and puts every
  coordinate progressively out toward the bottom. Nothing looks broken; it simply stops being 1:1.

  Safe to move because the plate's **content did not move**: the LCD well measures 54..86 in both
  the old and the new plate, and only the bottom row was trimmed (last ink 931.5 → 930.5). That was
  checked rather than assumed — the alternative was a re-layout, which would have invalidated every
  absolute Y in `Layout` at once.
- **The half-pixel meter offset is fixed, and the drift was never in the design.** `meterBoxY` was
  52.72 against the LCD's and the buttons' 53.22 — the audit's §E finding for this casting. The
  plate puts the meter well at exactly the same 54..86 as the LCD, so the artwork had the row
  aligned all along and only the code disagreed. All five parts now alias one `headerRowY` /
  `headerRowH` pair rather than each carrying its own literal, because they are one decision.

  **53, not 53.22, and it is measured.** The baked recess runs 54..86, which is the 32 px content
  box, so the border-box is 53..87. The designers reached the same place from the other side and
  named the cause: *"a 5px gap put the meters 0.4px high"*, now replaced by the PROGRAM caption's
  own `margin-bottom: 6px` construction so both column types resolve to one baseline.
- **`GUI-SPEC.md` §2 says the header row "sits on 32px" and BUILD-HANDOFF says 34px. Both are
  right** — 32 is the content box, 34 the border box, and 34 is the figure the suite holds constant.
  Read §2 alone and you build the band a border short.
- **The SYNC caption is drawn live and is NOT in the plate, as of the 2026-08-12 re-cut.** It was
  in both for one revision, and the two renderings landed a sub-pixel apart, so `SYNC ON` came out
  as `SYNC OON` and was unreadable. GUI-SPEC §8.2 lists the role as `SYNC ON / OFF` — state
  dependent, so runtime is right and the plate was wrong to carry it; BUILD-HANDOFF §1.2's own
  `data-plate="off"` attribute is the mechanism that should have suppressed it in the cut.

  The re-cut re-rendered the whole plate, not just that slot, so it was checked rather than trusted:
  the LCD and IN well edges are pixel-identical, and the substantive differences cluster in the
  scribble strip at y 880..920 — Permanent Marker's jitter is procedural, so it re-rolls on every
  render. Geometry unmoved, caption gone.

- **The plate is delivered and integrated.** `design/plate/fifth-member-plate-2x.png`, 2480 x 1862.
  `PanelBackground` is a blit now; the ~500 lines that rasterised the fascia, wear, ears, screws,
  tape elements, section frames and silkscreen are gone with it. Every ring except dial 1's two is
  `bakedInPlate`, verified by measuring ink in each tick annulus rather than read off the manifest -
  handoff section 1 lists seven knobs, but the plate also bakes dials 2 and 3, and only dial 1's two
  measure zero. **If something static looks wrong, it is wrong in the plate and gets re-cut, not
  patched in code.**
- **Icon**: concept 1b "repeat train" in its readout-free "delay only" form, wired to `ICON_BIG`
  (1024) and `ICON_SMALL` (256) from `design/icons/`. Verified out of the shipped bundle, not just
  off disk.
- **Not done**: registration in `../manifest/suite.toml`, deliberately held until all six suite
  plugins exist (Elmer, a bus glue plugin, is unbuilt). It will also need a tagged release and a
  freshly generated `windows_appid` GUID.
