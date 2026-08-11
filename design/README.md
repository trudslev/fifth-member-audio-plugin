# Handoff: Fifth Member — Tempo-Synced Delay (plugin panel + product icon)

> **Build handoff — start at `BUILD-HANDOFF.md`.** It is authoritative for the build and supersedes any conflicting figure here or in `GUI-SPEC.md`: per-element asset format, LED and multi-label state tables, the three Delay Character pages, per-knob mark and tick tables, scope plot-vs-display bounds, the PROGRAM LCD character budget, the four tape elements, the full palette with measured ratios, bypass, and what stays unchanged.
>
> `GUI-SPEC.md` remains as the record of the conformance pass and its reasoning.

## Overview
**Fifth Member** is a tempo-synced stereo delay plugin, model **DL-88**, part of the Neon Foundry suite
(alongside Gatecrasher, Chorus-60, and Reflect-84). This bundle covers two deliverables:

1. **The plugin panel** — the full editor UI, 1240 px wide fixed-width fascia in a rack-mount chassis.
2. **The product icon** — concept 1b, the phosphor "repeat train", for DAW browsers, the plugin manager,
   the store, and installers.

The design concept is *road-worn touring gear*: a black rack unit that has lived on a truck. Brushed
aluminum rack ears, corner wear rubbed through to bare metal, gaffer-tape nameplate hand-lettered in
marker, cable-tape labels, stencil stamps. The premise is that this is crew-rigged equipment, not a
designer product — that intent should survive implementation. Do not "clean it up."

## About the Design Files
The files in this bundle are **design references created in HTML** — prototypes showing intended look
and behavior, not production code to copy directly. The task is to **recreate these designs in the
target environment** using its established patterns: for an audio plugin that typically means JUCE
(C++), iPlug2, or a web-view-based editor. If the codebase already has a component/skin system, build
these to it. If no environment exists yet, choose the framework appropriate to the target plugin
formats (VST3 / AU / AAX) and implement there.

The HTML uses CSS gradients, inset shadows, conic gradients, and a \`<canvas>\` for the scope. In a
native renderer these become drawn primitives or pre-rendered knob filmstrips — see *Implementation
Notes*.

## Fidelity
**High-fidelity.** All colors, sizes, spacing, typography, and interaction behavior below are final and
exact, and match the shipped prototype. Reproduce them faithfully; do not re-derive measurements by eye
from screenshots. The one deliberately loose element is the nameplate lettering (see *Nameplate*).

---

# PART 1 — THE PLUGIN PANEL

## Overall Layout
Single non-resizable window. Outer wrapper centers the unit on a \`#171716\` page with 36 px vertical /
24 px horizontal padding (that page background is the prototype's stage, not part of the plugin — in a
DAW the plugin is just the unit).

The **unit** is 1240 px wide, \`border-radius: 5px\`, \`overflow: hidden\`, background \`#0c0c0b\`,
drop shadow \`0 30px 70px rgba(0,0,0,.7)\` plus an inset top lip \`0 2px 0 rgba(255,255,255,.05)\`.
It is a 3-column flex row:

| Column | Width | Contents |
|---|---|---|
| Left rack ear | 52 px fixed | brushed metal, 2 screws, rotated stencil "RACK 4 · MON WORLD" |
| Fascia | flex: 1 (1084 px) | all controls, padding \`16px 18px 12px\` |
| Right rack ear | 52 px fixed | brushed metal, 2 screws, cable tape "DLY 4", stencil "HALDEN HALL · LOAD-IN 06" |

Fascia contents stack vertically:
1. **Header** — nameplate + PROGRAM LCD + IN/OUT meters. Bottom border \`1px solid rgba(255,255,255,.07)\`, 14 px bottom padding.
2. **Repeat Timeline scope** — margin-top 14 px.
3. **Control row A** — TIMING / REPEATS / OUTPUT, margin-top 22 px, 16 px gap.
4. **Control row B** — DELAY CHARACTER, margin-top 22 px.
5. **Foot strip** — recessed label window + spec text, margin-top 12 px, top border \`1px solid rgba(255,255,255,.06)\`, 10 px top padding.

### Fascia surface treatment (four stacked, all \`pointer-events: none\`)
- Base: linear gradient 180° \`#1b1a18\` 0% → \`#131211\` 42% → \`#0e0e0c\` 100%.
- Brushed texture: \`repeating-linear-gradient(90deg, rgba(255,255,255,.022) 0 1px, rgba(0,0,0,.05) 1px 3px)\`.
- Ambient light: \`radial-gradient(90% 120% at 12% -10%, rgba(255,255,255,.07), transparent 55%)\` plus \`radial-gradient(70% 90% at 88% 110%, rgba(120,110,95,.10), transparent 60%)\`.
- **Corner wear** — four radial gradients in \`rgba(196,190,178, …)\` rubbing through to bare metal, each anchored to a corner with a different size and opacity so the wear is asymmetric:
  - top-left 120×70, peak .55; top-right 140×80, peak .48; bottom-right 150×90, peak .42; bottom-left 100×64, peak .35.
- **Scuffs** — two thin horizontal streaks: 210×5 at (left 300, top 8) rotated −0.6°, \`rgba(205,199,186,.30)\` fading to transparent at both ends; and 260×3 at (right 180, bottom 34) rotated +0.5°, \`rgba(190,184,172,.22)\`.

The asymmetry is the point. If the implementation bakes these into a background bitmap, bake them
exactly as specified rather than tiling a generic noise texture.

### Rack ears
Left ear gradient (96°): \`#8e8a82\` 0%, \`#c2beb4\` 18%, \`#9d998f\` 40%, \`#75726b\` 62%, \`#a9a49a\` 84%, \`#807c75\` 100%.
Right ear gradient (96°): \`#7d7a73\` 0%, \`#aca79d\` 20%, \`#8c8880\` 44%, \`#6f6c66\` 66%, \`#b0aba0\` 88%, \`#7a7770\` 100%.
Both get a brush overlay \`repeating-linear-gradient(92deg, rgba(255,255,255,.10) 0 1px, rgba(0,0,0,.09) 1px 2px)\`
plus radial specular highlights at top and bottom, and (left ear only) a soft dark radial at 70%/46%.

**Screws**: 24 × 24 circles at (left 14, top 22) and (left 14, bottom 22) on both ears.
Fill \`radial-gradient(circle at 36% 30%, #4a4741, #14130f 70%)\`, ring \`inset 0 0 0 3px rgba(255,255,255,.16)\`,
drop \`0 2px 4px rgba(0,0,0,.6)\`. Each has a 12 × 3 slot in \`#0b0a08\` at (left 20, ±32) rotated to a
**different** angle per screw: left ear +28° / −14°, right ear +52° / +9°. Keep the four angles distinct —
identical screws read as CG.

**Rotated text on ears**: 11 px Barlow Condensed 600, letter-spacing .34em, \`rgba(30,28,25,.62)\`, rotated −90°.
Right ear cable-tape label: 44 px wide, rotated +1.4°, gradient \`#e8e3d3\` → \`#cdc7b6\`, shadow \`0 2px 4px rgba(0,0,0,.45)\`,
text "DLY 4" in Permanent Marker 12 px \`#20201c\`. Right ear stencil: Special Elite 11 px, letter-spacing .22em, \`rgba(38,35,31,.55)\`.

## Header

### Nameplate (326 px fixed)
A strip of gaffer tape with the product name hand-written on it.
- Container rotated **−1.2°**, padding \`9px 20px 11px\`.
- Fill: linear gradient 180° \`#d9d4c6\` → \`#bdb7a7\` 60% → \`#a8a293\`.
- Shadow \`0 3px 7px rgba(0,0,0,.55)\`, inset top highlight \`0 1px 0 rgba(255,255,255,.5)\`.
- **Torn edges** via \`clip-path: polygon(1% 6%, 99% 0%, 100% 92%, 60% 100%, 12% 96%, 0% 88%)\`.
- Tape weave overlay: \`repeating-linear-gradient(90deg, rgba(0,0,0,.055) 0 1px, rgba(255,255,255,.05) 1px 4px)\`.
- Grime: \`radial-gradient(60% 100% at 80% 20%, rgba(90,80,66,.30), transparent 70%)\`.
- Lettering: **Permanent Marker** 33 px, line-height .98, \`#151310\`, letter-spacing .005em, text-shadow \`0 1px 0 rgba(255,255,255,.25)\`. Text: **FIFTH MEMBER**.

**Nameplate is deliberately freestyle.** It must read as marker on tape written by a crew member in a
hurry, not as set type. If the target renderer can't do Permanent Marker convincingly, replace it with a
hand-lettered vector asset rather than substituting a neat font. This is the single most important
brand element on the panel.

Below the tape, 12 px gap, 3 px stack:
- \`TEMPO-SYNCED DELAY\` — Barlow Condensed 600, 12 px, letter-spacing .30em, \`#8e887d\`.
- \`MODEL DL-88 · STEREO\` — Barlow Condensed 500, 12 px, letter-spacing .30em, \`#6d685f\`.

### PROGRAM LCD (flex: 1)
Label \`PROGRAM\` above: Barlow Condensed 600, 10 px, letter-spacing .28em, \`#7e786e\`, 6 px below.

The LCD bar: height **44 px**, radius 3 px, fill \`linear-gradient(180deg, #071009, #040806)\`,
border \`1px solid #2a2823\`, inset \`0 2px 10px rgba(0,0,0,.9)\` + \`0 1px 0 rgba(255,255,255,.05)\`.
Three regions in a row:
- **Bank tag** — left, padding \`0 13px\`, right divider \`1px solid rgba(255,255,255,.09)\`, Share Tech Mono 14 px, letter-spacing .06em, \`#9fb2a2\`. Shows a **single dynamic value**, \`FACT\` or \`USER\` — never both with one greyed.
- **Program name** — center, flex: 1, Share Tech Mono **19 px**, letter-spacing .14em, \`#cfd8cb\`, glow \`text-shadow: 0 0 9px rgba(180,210,185,.35)\`. Format: two-digit zero-padded index, space, name (e.g. \`01 YOU TOO?\`). Cursor pointer; click opens the program list.
- **Caret** — \`▼\`, padding \`0 12px\`, Share Tech Mono 12 px, \`#6f7a70\`, also opens the list.

**Program list** (when open): absolutely positioned, \`top: 46px\`, z-index 40, max-height 210 px, scrolls.
Background \`#060d09\`, border \`1px solid #2c2b26\`, shadow \`0 18px 32px rgba(0,0,0,.75)\`.
Rows: padding \`8px 14px\`, Share Tech Mono 15 px, letter-spacing .1em, bottom border \`1px solid rgba(255,255,255,.05)\`.
Row label left, bank tag right in 11 px \`#5d675e\`. Selected row \`#d7e2d6\`, unselected \`#7d8a7e\`.

**Naming input** (when saving): overlays the name region, \`left: 60px; right: 30px; top: 3px; bottom: 3px\`,
background \`#0a1410\`, border \`1px solid rgba(190,220,195,.35)\`, Share Tech Mono 18 px, letter-spacing .12em,
centered, uppercase, placeholder \`NAME PROGRAM\`.

**SAVE** button: 66 × 32 content (**34 border-box**, the suite header height), radius 3,
\`linear-gradient(180deg,#2a2823,#161512)\`, border \`1px solid #0a0908\`, inset \`0 1px 0 rgba(255,255,255,.14)\`,
drop \`0 2px 4px rgba(0,0,0,.5)\`. Hover: \`linear-gradient(180deg,#35322b,#1c1a16)\`.

**DELETE** button: 74 × 32 content (**34 border-box**), same construction but darker fill
\`linear-gradient(180deg,#242219,#141310)\` and inset highlight \`.10\`.

**Both buttons carry two stacked legends and never change their face** — SAVE over STORE, DELETE over
CANCEL, Barlow Condensed 600 **10 px**, letter-spacing .14em, \`line-height: 1\`, 2 px gap, centred in the
34 px box. Each legend is individually **backlit**: lit \`#f2ece0\` with \`0 0 7px rgba(242,236,224,.55),
0 0 15px rgba(242,236,224,.25)\`, dark \`#77736a\` with no glow. Weight and size never change between the
two states. There is **no disabled face** — when a button has nothing to do, both its legends sit dark
but stay readable (3.48:1 on SAVE's face, 3.66:1 on DELETE's). BUILD-HANDOFF §1.3.1 has the full
state table; in short, SAVE lights on an edited Program, DELETE on a User Program, and STORE and
CANCEL while a Program is being named.

### IN / OUT meters
Two stacks, 10 px gap. Each: caption (Barlow Condensed 600, 11 px, letter-spacing .26em, \`#a8a294\`) with
\`margin-bottom: 6px\` — the PROGRAM caption's construction, so the meters sit on the header band's shared
baseline — then a readout box **74 × 32** (34 border-box), radius 2, background \`#05080a\`, border \`1px solid #2a2823\`,
inset \`0 2px 8px rgba(0,0,0,.9)\`, Share Tech Mono 16 px, \`#b9c3c8\`, centered.
Prototype shows static \`-12.4\` and \`-14.1\`; **in production these are live dBFS readouts** from the
host's input and output buses, ballistics at the studio's standard peak/RMS behavior.

## Repeat Timeline Scope

Caption row above, 8 px below:
- **Lamp** — 13 px circle, fill = accent color, glow \`0 0 10px <accent>, 0 0 22px rgba(255,157,60,.45), inset 0 1px 1px rgba(255,255,255,.5)\`. Pulses via keyframes \`fmLamp\`: opacity 1 → .72 → 1, **1.6 s, ease-in-out, infinite**.
- \`REPEATS LIVE\` — Barlow Condensed 600, 12 px, letter-spacing .26em, \`#c3bcae\`.
- Right-aligned \`REPEAT TIMELINE\` — Barlow Condensed 600, 11 px, letter-spacing .24em, \`#79746b\`.

The scope box: height **96 px**, radius 2, background \`#04060a\`, border \`1px solid #23221e\`,
inset \`0 3px 14px rgba(0,0,0,.9)\`. It splits into two zones:

### Readout strip — top 22 px
Bottom border \`1px solid rgba(150,175,155,.13)\`, padding \`0 9px\`, 16 px gap, \`white-space: nowrap\`,
\`overflow: hidden\`. **Share Tech Mono 11 px, letter-spacing .12em** — the same segment-display face as
the PROGRAM LCD. This is a deliberate decision: *all* variable readout text lives here in LCD type, so
the plot area stays clean. Contents left to right:
1. \`RPT ENV\` — \`#7d8d7e\`.
2. Mode descriptor line, \`#a9bda9\` — \`"<CHARACTER> · <STEREO MODE> · FB <n>% · <n> ms / DIV"\`, e.g. \`TAPE ECHO · 3-HEAD · STEREO · FB 62% · 375 ms / DIV\`. Character strings: \`TAPE ECHO · 3-HEAD\`, \`BBD BUCKET-BRIGADE\`, \`DIGITAL · 16-BIT\`.
3. Flex spacer.
4. Timing info, \`#7d8d7e\` — \`"<division> · <bpm> BPM · <n> ms"\` when synced, \`"FREE · <n> ms"\` when not.
5. \`0 dB\` — \`#7d8d7e\`.

### Plot area — bottom 74 px
A \`<canvas>\`, full width, DPR-aware (resize backing store to \`rect × devicePixelRatio\`, then \`setTransform(dpr,0,0,dpr,0,0)\`).
Redrawn every \`requestAnimationFrame\`. Only \`RPT ENV\` and \`0 dB\` corner labels sit near it — **no other
text belongs inside the plot**.

Draw order per frame:
1. **Grid** — \`rgba(150,175,155,.10)\`, 1 px: seven vertical lines at eighths of the width, one horizontal at mid-height.
2. **Pulse model** — a pulse spawns every \`delayMs\`; each new pulse's amplitude = previous × feedback × modeLoss. When amplitude drops below 0.03 it resets to 1 (a fresh dry hit). Pulses live in a **2600 ms window** and scroll right-to-left: \`x = W − ((now − t) / 2600) × W\`.
3. **Height mapping** — baseline sits at \`H − 9\`; usable span is \`baseline − 8\`; \`height = span × clamp(amp^0.45, 0.04, 1)\`. The 0.45 exponent is what makes late repeats stay visible; don't linearize it.
4. **Ghost tails** — \`rgba(190,200,190,.07)\` filled rects, width \`max(8, W × 0.014)\`, drawn behind everything.
5. **Baseline** — \`rgba(160,180,165,.22)\`, 1 px.
6. **Pulses** — accent-colored vertical strokes from baseline up to height, with a filled cap circle. Dry hit: line width 3, cap radius 3.2, alpha 1. Repeat: line width 2.2, cap radius 2.2, alpha .92. Glow: \`shadowColor = accent\`, \`shadowBlur = max(4, 16 − age × 6)\` where \`age = (now − t) / 260\`.
7. **Ping-pong marker** — when stereo mode is PING-PONG only: a dashed line (\`[3,4]\`) in \`rgba(200,210,200,.16)\` at half the span above the baseline.

Mode loss factors: TAPE \`1 − generationLoss/260\`, BBD \`1 − modDepth/420\`, DIGITAL \`1 − degrade/300\`.

## Control Row A

Three panels, 16 px gap: **TIMING** 296 px fixed, **REPEATS** flex: 1, **OUTPUT** 300 px fixed.

### Shared panel chrome
Border \`1px solid rgba(255,255,255,.075)\`, radius 3, background
\`linear-gradient(180deg, rgba(255,255,255,.022), rgba(0,0,0,.18))\`. Each has a **notched legend**:
absolutely positioned at \`top: -7px; left: 12px\`, padding \`0 7px\`, background \`#131211\` (matching the
fascia so it punches a hole in the border), Barlow Condensed 600, 10 px, letter-spacing .28em, \`#8d877c\`.

### TIMING
- **SYNC switch** — a 52 × 24 track, radius 2, \`#0b0a09\`, border \`1px solid #2b2924\`, inset \`0 2px 5px rgba(0,0,0,.8)\`. Thumb 23 px wide, inset 2 px top/bottom, radius 1, \`linear-gradient(180deg,#cfc9bb,#8d887d)\`, shadow \`0 1px 3px rgba(0,0,0,.7)\`. Position \`left: 2px\` (off) / \`27px\` (on), **transition \`left 180ms ease\`**. Caption \`SYNC ON\`/\`SYNC OFF\` in Barlow Condensed 600, 11 px, letter-spacing .22em, \`#a49d92\`.
- **NOTE DIVISION** — LED + label row (7 px LED, 10 px Barlow Condensed 600, letter-spacing .24em, \`#a49d92\`), then five equal buttons in a 5 px-gap row, height 30, radius 3. Divisions and their beat multipliers: \`1/4\` ×1, \`1/8.\` ×0.75, \`1/8\` ×0.5, \`1/8T\` ×0.333…, \`1/16\` ×0.25. Delay time = \`(60000 / bpm) × mult\`.
- **TIME knob** — 66 px, centered, with an LED + \`TIME\` label beneath. Displays ms.

### REPEATS
- **FEEDBACK** — 84 px knob (the large size), range **0–110 %**, label \`FEEDBACK\` in \`#a9a297\`. Above 100 % is intentional: self-oscillation.
- **CROSS-FEED** — 62 px knob, range 0–100 %, offset \`margin-top: 23px\` so its label baseline aligns with FEEDBACK's. Its LED is lit only in PING-PONG, but **the knob stays fully adjustable in every mode**.
- **STEREO MODE** — label (10 px, .24em, \`#79746b\`, margins \`16px 0 7px\`) then three equal buttons, height 32, 5 px gap: \`MONO\`, \`STEREO\`, \`PING-PONG\`.

### OUTPUT
Vertically centered, two knobs, 38 px gap: **MIX** 82 px (0–100 %) and **OUTPUT TRIM** 62 px
(−24 to +12 dB, formatted with an explicit \`+\` on positives, one decimal), the smaller offset
\`margin-top: 21px\` for label alignment.

## Control Row B — DELAY CHARACTER

One full-width panel, padding \`26px 20px 22px\`, legend \`DELAY CHARACTER\`. Three columns, 26 px gap:

### Column 1 — mode selector (168 px fixed)
Three stacked buttons, 12 px gap, each **54 px tall**, padding \`0 14px\`, 12 px gap between a **10 px LED**
and the label (Barlow Condensed 600, **13 px**, letter-spacing .24em). Modes: \`TAPE\`, \`BBD\`, \`DIGITAL\`.
Selected LED glow is the larger variant: \`0 0 10px rgba(240,236,220,.75), 0 0 22px rgba(230,224,205,.35)\`.

### Column 2 — the three character dials (flex: 1)
Left border \`1px solid rgba(255,255,255,.07)\`, padding-left 26. Inner padding \`26px 6px 0\`, dials spread
\`space-around\`, each in a 150 px column.

**This is the central design decision of the panel.** There are always **exactly three physical dials**.
They never appear, disappear, move, or grey out. Each dial has a **stack of silk-screened labels
underneath it — one per mode — each with its own tiny 6 px LED**. Switching character mode does two
things and nothing else:
1. Relights which label's LED is on.
2. Rotates the dial to that mode's stored value.

The label stacks (top to bottom = TAPE, BBD, DIGITAL):

| Dial | TAPE | BBD | DIGITAL |
|---|---|---|---|
| 1 | WOW (0–100 %) | MOD RATE (0.05–8.00 Hz) | REPEAT DEGRADE (0–100 %) |
| 2 | FLUTTER (0–100 %) | MOD DEPTH (0–100 %) | *(no label — dial 2 is unused in DIGITAL)* |
| 3 | GENERATION LOSS (0–100 %) | *(none)* | *(none)* |

Labels are Barlow Condensed 600, 10 px, letter-spacing .18em, left-aligned, 5 px vertical gap.
Lit: text \`#e7e1d4\`, LED \`radial-gradient(circle at 35% 30%, #ffffff, #efe9d6 45%, #b9b09a 100%)\`,
glow \`0 0 7px rgba(240,236,220,.8), 0 0 15px rgba(230,224,205,.35)\`.
Unlit: text \`#615c54\`, LED \`radial-gradient(circle at 35% 30%, #34322d, #171614)\`, inset \`0 1px 2px rgba(0,0,0,.8)\`.

When a dial has no parameter in the current mode, it falls back to controlling its first available
parameter — it stays live and turnable, just with no lit label. **Never dim or disable the dial itself.**

Dial size here is **76 px** (between the 62 small and 84 large), with the mode-change rotation using the
slower easing (see *Knobs*).

### Column 3 — CHARACTER · ALL MODES (250 px fixed)
Left border \`1px solid rgba(255,255,255,.07)\`, padding-left 26. Heading \`CHARACTER · ALL MODES\` in
Barlow Condensed 600, 10 px, letter-spacing .24em, \`#6d685f\`, nudged \`top: -10px\`. Two 66 px knobs:
**DAMPING** and **SATURATION**, both 0–100 %. These apply in every mode, which is why they sit apart
behind their own rule.

## Foot strip
- **Recessed label window** — padding \`4px 12px 5px\`, background \`linear-gradient(180deg,#0a0a09,#141312)\`, border \`1px solid #33312b\`, inset \`0 2px 6px rgba(0,0,0,.8)\` + \`0 1px 0 rgba(255,255,255,.06)\`. Text in **Special Elite** 11 px, letter-spacing .10em, \`#a09883\`: \`CH 4 — GTR / STAGE LEFT\`.
- \`DL-88 · TOURING SPEC · 5U\` — Barlow Condensed 600, 10 px, letter-spacing .26em, \`#57534c\`.
- Right-aligned \`SN 0417 · v1.0\` — same style. Serial number, static; **not** a state readout. Fifth Member has no bypass control, so no state word is printed here.

---

## Component Specs

### Knobs
Four sizes are in use: **62** (secondary), **66** (standard), **76** (character dials), **82/84** (primary).
All share the same construction; the numbers below scale with size.

**Body** — circle, \`cursor: ns-resize\`, border \`1px solid #090908\`.
- 62/66 px: fill \`radial-gradient(circle at 38% 28%, #4c4942 0%, #262420 55%, #121110 100%)\`, shadow \`inset 0 1px 1px rgba(255,255,255,.16), inset 0 -6px 12px rgba(0,0,0,.65), 0 6px 12px rgba(0,0,0,.5)\`.
- 76 px: fill \`… #514e46 0%, #282621 55%, #121110 100%\`, shadow \`inset 0 1px 1px rgba(255,255,255,.17), inset 0 -7px 14px rgba(0,0,0,.68), 0 7px 14px rgba(0,0,0,.52)\`.
- 82/84 px: fill \`… #55524a 0%, #2a2823 55%, #131210 100%\`, shadow \`inset 0 1px 1px rgba(255,255,255,.18), inset 0 -8px 16px rgba(0,0,0,.7), 0 8px 16px rgba(0,0,0,.55)\`.

**Tick ring** — a ring of radial ticks *outside* the body, made with a repeating conic gradient masked to
an annulus. Small knobs: \`inset: -10px\`, \`repeating-conic-gradient(from 180deg, #7d786e 0deg 1.4deg, transparent 1.4deg 27deg)\`, mask \`radial-gradient(circle, transparent 60%, #000 62%, #000 74%, transparent 76%)\`.
76 px: \`inset: -11px\`, color \`#827d72\`, \`1.3deg / 24deg\`, mask stops 62/64/76/78%.
82/84 px: \`inset: -11px\`, color \`#85806f\`, \`1.2deg / 22.5deg\`, mask stops 62/64/76/78%.
(Include the \`-webkit-mask\` prefix in web contexts.)

**Pointer** — a rounded bar from the top of the knob, \`linear-gradient(#f4efe4, #b6afa1)\` (large knobs \`#f6f1e6 → #b8b1a2\`, 76 px \`#f5f0e5 → #b7b0a1\`), rotated about a \`transform-origin\` at the knob's center:

| Knob | Pointer w × h | Top offset | transform-origin Y |
|---|---|---|---|
| 62 px | 2 × 24 | 6 | 25 px |
| 66 px | 2 × 26 | 6 | 27 px |
| 76 px | 2.5 × 30 | 7 | 31 px |
| 82 px | 3 × 32 | 7 | 34 px |
| 84 px | 3 × 33 | 7 | 35 px |

**Rotation** — sweep is **270°**, from **−135°** (minimum) to **+135°** (maximum):
\`angle = −135 + 270 × normalized\`.
Transition \`transform 620ms cubic-bezier(.18, .9, .2, 1)\` on standard knobs; the character dials use the
slower \`660ms cubic-bezier(.16, .86, .24, 1)\` so a mode change reads as the panel physically re-setting
itself.

**Drag** — pointer-down captures; vertical drag, **190 px of travel spans the full parameter range**;
up increases. Value clamps to range, and to \`step\` where one is defined (TIME steps by 1 ms).
Listeners go on the window and release on pointer-up.

**Value tooltip** — appears only while that knob is being dragged. Absolutely positioned \`top: -24px\`,
centered, padding \`2px 8px\`, radius 2, background \`#0b0a09\`, border \`1px solid #35322b\`,
shadow \`0 4px 10px rgba(0,0,0,.65)\`, \`pointer-events: none\`, z-index 5, **Share Tech Mono 12 px**, \`#ece6d8\`.

**Knob caption** — Barlow Condensed 600, 10 px, letter-spacing .20em, \`#c3bcae\` (or \`#a9a297\` on the two
primary knobs), 14–15 px below the knob.

### LEDs
Three sizes: **6 px** (character dial labels), **7 px** (knob captions, division/stereo buttons), **10 px**
(character mode buttons), plus the 13 px scope lamp.

| State | Fill | Shadow |
|---|---|---|
| On | \`radial-gradient(circle at 35% 30%, #ffffff, #efe9d6 45%, #b9b09a 100%)\` | \`0 0 8px rgba(240,236,220,.75), 0 0 18px rgba(230,224,205,.35)\` |
| On (10 px) | same | \`0 0 10px rgba(240,236,220,.75), 0 0 22px rgba(230,224,205,.35)\` |
| On (6 px) | same | \`0 0 7px rgba(240,236,220,.8), 0 0 15px rgba(230,224,205,.35)\` |
| Off | \`radial-gradient(circle at 35% 30%, #3a3833, #191816)\` | \`inset 0 1px 2px rgba(0,0,0,.8)\` |
| Off (6 px) | \`radial-gradient(circle at 35% 30%, #34322d, #171614)\` | \`inset 0 1px 2px rgba(0,0,0,.8)\` |

LEDs are warm white, not colored. The only colored light on the panel is the accent-colored scope lamp.

### Buttons (division, stereo, character mode)
Fill \`linear-gradient(180deg, #2b2924, #171613)\`, border \`1px solid #0a0a08\`, radius 3, \`cursor: pointer\`.
- **Raised (unselected)**: \`inset 0 1px 0 rgba(255,255,255,.13), 0 3px 6px rgba(0,0,0,.5)\`; label \`#857f75\`.
- **Pressed (selected)**: \`inset 0 2px 6px rgba(0,0,0,.75), 0 1px 0 rgba(255,255,255,.10)\`; label \`#f0eade\`.
- **Hover**: fill \`linear-gradient(180deg, #332f29, #1b1a16)\`.

Selection is communicated by the pressed shadow, the label brightening, and the LED lighting — three
signals, no color change. That redundancy is deliberate; it survives on a dim stage.

## Interactions & Behavior

| Control | Interaction | Result |
|---|---|---|
| All knobs | vertical drag (190 px = full range) | value updates live, tooltip shown during drag |
| SYNC switch | click | toggles; thumb slides 180 ms; TIME knob's LED goes out but the knob stays turnable |
| Note division | click | sets division; delay time recomputes from BPM; scope respaces immediately |
| Stereo mode | click | sets mode; PING-PONG adds the dashed scope marker; CROSS-FEED LED lights |
| Character mode | click | see below |
| PROGRAM name / caret | click | opens/closes program list |
| Program row | click | loads that program, closes list |
| SAVE | click | opens the inline naming field; STORE and CANCEL light, SAVE and DELETE go dark |
| Naming field | Enter | creates a **new User program** (never overwrites); empty name defaults to \`TAKE <n>\` |
| Naming field | Escape | cancels |
| DELETE | click | removes the current User program; no-op on Factory |

**Character mode change animation.** On mode change the panel briefly clears \`armed\`, which snaps every
knob angle to −135°, then restores it **40 ms later**. Combined with the 660 ms dial easing, the result
is that all three dials visibly sweep from zero to their new values — the panel physically re-setting
itself, the way real recall works. Keep this; it is the moment that sells the hardware premise.

**No disabling anywhere else.** Knobs, buttons, and labels stay visually present and fully adjustable at
all times. LEDs indicate only what is *in circuit and active*. Nothing dims, greys, or vanishes. DELETE
is the single exception, and it is a destructive action on a nonexistent target, not a parameter.

## State

| Key | Type / range | Default | Notes |
|---|---|---|---|
| \`sync\` | bool | \`true\` | tempo sync on |
| \`div\` | int 0–4 | \`2\` | index into division table (\`1/8\`) |
| \`timeMs\` | 1–2000, step 1 | \`375\` | free-run delay time |
| \`feedback\` | 0–110 | \`62\` | % |
| \`stereo\` | int 0–2 | \`1\` | MONO / STEREO / PING-PONG |
| \`cross\` | 0–100 | \`40\` | cross-feed % |
| \`mode\` | int 0–2 | \`0\` | TAPE / BBD / DIGITAL |
| \`armed\` | bool | \`true\` | false for 40 ms during a mode change |
| \`wow\` | 0–100 | \`24\` | TAPE dial 1 |
| \`flutter\` | 0–100 | \`16\` | TAPE dial 2 |
| \`genLoss\` | 0–100 | \`32\` | TAPE dial 3 |
| \`modRate\` | 0.05–8 | \`0.55\` | BBD dial 1, Hz, 2 decimals |
| \`modDepth\` | 0–100 | \`22\` | BBD dial 2 |
| \`degrade\` | 0–100 | \`15\` | DIGITAL dial 1 |
| \`damping\` | 0–100 | \`46\` | all modes |
| \`saturation\` | 0–100 | \`24\` | all modes |
| \`mix\` | 0–100 | \`38\` | % |
| \`trim\` | −24 to +12 | \`0\` | dB |
| \`bank\` | \`'FACT'\` \| \`'USER'\` | \`'FACT'\` | |
| \`factIdx\`, \`userIdx\` | int | \`0\`, \`0\` | |
| \`user\` | string[] | \`[]\` | user program names |
| \`listOpen\`, \`naming\` | bool | \`false\` | UI only |
| \`draft\` | string | \`''\` | naming field buffer |
| \`active\` | string \| null | \`null\` | key of the knob being dragged |

**Host-supplied:** \`bpm\` (60–200, default 120) comes from the DAW transport in production — in the
prototype it is a tweakable prop. \`accentColor\` (default \`#ff9d3c\`) is a skin option; alternates offered
are \`#ff5f3c\`, \`#48e08a\`, \`#4fc3ff\`.

**Factory programs** (six): \`YOU TOO?\`, \`SLAPBACK 66\`, \`CANYON WALL\`, \`HALF-STEP\`, \`GHOST NOTE\`, \`BUS TIRE DUB\`.
These names carry the product's voice — road slang, not descriptors. Keep them.

**Persistence:** the prototype keeps user programs in memory only. Production must persist them to the
plugin's preset store and serialize full parameter state into the host session.

---

# PART 2 — THE PRODUCT ICON

Chosen direction: **concept 1b, "repeat train."** Three alternates were explored and rejected — gaffer
tape (1a, most ownable but illegible small), character dial (1c, generic), rack-ear stencil (1d, strong
but reads as a different product family). All four are in this bundle for context.

## Canvas
Master **256 × 256** design units; all values scale linearly. Outer corner radius **52** (20.3 %).
Nothing bleeds past the rounded rect.

## Layers
1. **Chassis** — \`linear-gradient(165deg, #1a1917 0%, #0f0f0d 50%, #080907 100%)\`, inset top lip \`0 2px 0 rgba(255,255,255,.06)\`.
2. **LCD window** — inset 22 from every edge (212 × 212), radius 30, fill \`linear-gradient(180deg, #04100a, #020806)\`, border 1 unit \`#2b2a25\`, recess \`inset 0 4px 18px rgba(0,0,0,.9)\`. Phosphor bloom on top: radial 70 % × 70 % at (50 %, 60 %), \`rgba(90,220,140,.10)\` → transparent at 70 %, clipped to the same radius.
3. **Baseline** — inside the window, inset 26 left/right, 64 up from its bottom; height 2, \`rgba(140,200,160,.28)\`. Window-local: x 26→186, y 146.
4. **Repeat train** — six bars sitting on the baseline, growing up. First bar at window-local x = 26; each **11 wide** with a **15 gap** (pitch 26, total run 141). Radius 2.

| # | Height | Fill (vertical) | Glow |
|---|---|---|---|
| 1 | 112 | \`#a9ffcd\` → \`#3ee089\` | \`0 0 16px rgba(70,230,140,.90)\` |
| 2 | 74 | \`#8ff2bb\` → \`#31c876\` | \`0 0 13px rgba(70,230,140,.72)\` |
| 3 | 48 | \`#74d9a3\` → \`#26a660\` | \`0 0 11px rgba(70,230,140,.55)\` |
| 4 | 30 | \`#5cb888\` → \`#1e8a50\` | \`0 0 9px rgba(70,230,140,.42)\` |
| 5 | 18 | solid \`#187a45\` | \`0 0 7px rgba(70,230,140,.30)\` |
| 6 | 10 | solid \`#125c34\` | none |

The heights are a feedback decay (≈ 0.66× each, floored). Keep the exact values — a clean geometric
series looks mechanical at large sizes.

5. **Readout** — Share Tech Mono 19, letter-spacing .16em, 26 up from the window's bottom, 28 in from its
left/right edges. Left \`1/8D\` in \`#7fbf96\`; right \`5M\` in \`#4e7d5e\`. **Outline this type on export** — never
ship the icon with a live font dependency.

## Small sizes
Checked at 64, 40, and 24. At **24 and below**: drop the readout (layer 5) and the phosphor bloom, and
raise the bar glows to compensate. Produce a **separate simplified master** for 16/24/32 rather than
downscaling the full artwork.

## Deliverables
- \`fifth-member.svg\` — 256 × 256 viewBox master, type outlined, plus one simplified sibling for ≤ 32 px.
- PNG at 16, 24, 32, 48, 64, 128, 256, 512, 1024 (@1x and @2x where the platform wants both).
- macOS \`.icns\`, Windows \`.ico\`.
- If a host requires a non-square plugin-browser thumbnail, **crop to the LCD window outward — never letterbox the full chassis.**

---

# Design Tokens

### Chassis & fascia
| Token | Value |
|---|---|
| page-bg | \`#171716\` |
| unit-bg | \`#0c0c0b\` |
| fascia-top / mid / bottom | \`#1b1a18\` / \`#131211\` / \`#0e0e0c\` |
| panel-border | \`rgba(255,255,255,.075)\` |
| panel-fill | \`linear-gradient(180deg, rgba(255,255,255,.022), rgba(0,0,0,.18))\` |
| divider | \`rgba(255,255,255,.07)\` |
| wear-metal | \`rgba(196,190,178, .35–.55)\` |

### Metal (rack ears, screws, tape)
| Token | Value |
|---|---|
| ear-light / mid / dark | \`#c2beb4\` / \`#9d998f\` / \`#75726b\` |
| screw-fill | \`radial-gradient(circle at 36% 30%, #4a4741, #14130f 70%)\` |
| screw-slot | \`#0b0a08\` |
| gaffer-light / mid / dark | \`#d9d4c6\` / \`#bdb7a7\` / \`#a8a293\` |
| cable-tape | \`#e8e3d3\` → \`#cdc7b6\` |
| marker-ink | \`#151310\` (nameplate), \`#20201c\` (cable tape) |

### LCD & phosphor
| Token | Value |
|---|---|
| lcd-fill | \`linear-gradient(180deg, #071009, #040806)\` |
| lcd-border | \`#2a2823\` |
| lcd-text | \`#cfd8cb\` (glow \`0 0 9px rgba(180,210,185,.35)\`) |
| lcd-text-dim | \`#9fb2a2\` |
| scope-bg | \`#04060a\` |
| scope-grid | \`rgba(150,175,155,.10)\` |
| scope-baseline | \`rgba(160,180,165,.22)\` |
| scope-readout / -readout-hi | \`#7d8d7e\` / \`#a9bda9\` |
| list-bg | \`#060d09\` |
| meter-bg / meter-text | \`#05080a\` / \`#b9c3c8\` |

### Type on metal / fascia
| Token | Value |
|---|---|
| label-bright | \`#f0eade\` |
| label | \`#c3bcae\` |
| label-mid | \`#a49d92\` / \`#a9a297\` |
| label-dim | \`#857f75\` / \`#8d877c\` / \`#7e786e\` |
| label-faint | \`#6d685f\` / \`#615c54\` / \`#57534c\` |
| disabled | \`#4d4941\` |

### Controls
| Token | Value |
|---|---|
| accent (default) | \`#ff9d3c\` (alts \`#ff5f3c\`, \`#48e08a\`, \`#4fc3ff\`) |
| led-on | \`radial-gradient(circle at 35% 30%, #ffffff, #efe9d6 45%, #b9b09a 100%)\` |
| led-off | \`radial-gradient(circle at 35% 30%, #3a3833, #191816)\` |
| btn-fill / btn-hover | \`#2b2924 → #171613\` / \`#332f29 → #1b1a16\` |
| shadow-raised | \`inset 0 1px 0 rgba(255,255,255,.13), 0 3px 6px rgba(0,0,0,.5)\` |
| shadow-pressed | \`inset 0 2px 6px rgba(0,0,0,.75), 0 1px 0 rgba(255,255,255,.10)\` |
| tooltip-bg / -border / -text | \`#0b0a09\` / \`#35322b\` / \`#ece6d8\` |

### Scale
Spacing steps in use: 3, 5, 7, 10, 12, 14, 16, 22, 26, 38.
Radii: 2 (LCD/scope/meters), 3 (buttons, panels), 5 (unit), 50 % (knobs, LEDs).
Knob diameters: 62, 66, 76, 82, 84. LED diameters: 6, 7, 10, 13.

### Typography
| Face | Source | Used for |
|---|---|---|
| **Barlow Condensed** 400/500/600/700 | Google Fonts, OFL | all panel labels, captions, legends. Sizes 10–13, letter-spacing .18em–.34em |
| **Share Tech Mono** | Google Fonts, OFL | every LCD, meter, scope readout, and value tooltip. Sizes 11–19 |
| **Permanent Marker** | Google Fonts, OFL | nameplate, cable-tape label |
| **Special Elite** | Google Fonts, OFL | stencil stamps, recessed foot label |

The typographic rule is simple and should be preserved: **anything that displays a variable value is in
Share Tech Mono; anything silk-screened onto the panel is in Barlow Condensed; anything applied by a
human with a marker or a stencil is in Permanent Marker or Special Elite.** Three voices, no exceptions.

---

# Implementation Notes

- **Knobs**: in a native plugin, pre-render each knob size as a filmstrip (≈ 128 frames over the 270°
  sweep) rather than compositing gradients at runtime. Keep the pointer as a separate rotating layer if
  the framework allows — the pointer highlight should not rotate with the body's specular.
- **Tick rings** are static per knob size; bake them into the background, not the filmstrip.
- **The scope** needs a real animation timer, not a paint-on-parameter-change. Budget one redraw per
  display refresh; the pulse list is tiny (a 2.6 s window at typical delay times holds well under 100
  entries) so this is cheap.
- **The scope's pulse model is a visualization, not the audio path.** In production, drive it from the
  actual delay line's tap amplitudes rather than re-simulating decay in the UI.
- **Fonts**: license-check and embed all four; do not rely on system fallbacks. A missing Permanent
  Marker turns the nameplate into a generic cursive and loses the entire concept.
- **Meter values** are placeholders in the prototype — wire to real bus levels.
- **BPM** is a prop in the prototype; take it from the host transport, and handle hosts that report no
  tempo by falling back to free-run.
- **Accessibility**: the panel is dark and low-contrast by design. Ensure every control is keyboard
  reachable with arrow-key increments, exposes its value to the host's automation and to screen readers,
  and that a host "high contrast" hint (where available) raises label colors — the label token scale
  above is ordered so this is a straightforward substitution.

## Known issues fixed in this bundle
Two defects were found while writing this document and are corrected in the included files:
1. The scope readout's \`ms / DIV\` figure was hardcoded to \`325 ms\` and did not track the actual delay time. It now derives from the computed delay.
2. A malformed closing tag in the OUTPUT panel markup.

---

# Files in this bundle

| File | What it is |
|---|---|
| \`Fifth Member.dc.html\` | **The plugin panel. Primary spec artifact.** Fully interactive. |
| \`IconPulse.dc.html\` | **The chosen icon (1b) at 256 × 256. Primary icon spec artifact.** |
| \`Fifth Member Icon.dc.html\` | Four-concept icon gallery, including 1b rendered at 64 / 40 / 24 for small-size review. |
| \`IconTape.dc.html\` | Rejected icon concept 1a (gaffer tape). Context only. |
| \`IconDial.dc.html\` | Rejected icon concept 1c (character dial). Context only. |
| \`IconStencil.dc.html\` | Rejected icon concept 1d (rack-ear stencil). Context only. |
| \`support.js\` | Runtime required to open any \`.dc.html\` in a browser. |

Open any \`.dc.html\` file directly in a browser — no build step, no server. Drag any knob, click any
button; the prototype is fully live and is the authority wherever this document is ambiguous.


## Icon assets (rendered)

Concept 1b — Repeat Train.

| File | Size | Projucer slot |
|---|---|---|
| `icons/fifth-member-icon-1024.png` | 1024×1024 | Large Icon |
| `icons/fifth-member-icon-256.png` | 256×256 | Small Icon |

PNG with alpha; the rounded-square corners are transparent, so JUCE/macOS masking will not double-round it. Source of truth is `IconPulse.dc.html` (256px design) — re-render at any multiple from there. The readout text has been removed; the decay train is vertically centred, so the mark reads cleanly at every size with no simplification variant needed.
