# Fifth Member — GUI Spec (conformance pass)

Model DL-88 · tempo-synced delay · Neon Foundry casting 5

**Build from this file, with two exceptions:** the Program buttons (SAVE/STORE, DELETE/CANCEL — geometry, backlit legend states and the naming-mode state matrix) are specified in `BUILD-HANDOFF.md` §1.3 and appear nowhere below; the printed dial scales are in `BUILD-HANDOFF.md` §4.

> **Superseded in part.** The printed-scale tables in §5 were cut against proposed tapers and are wrong; `BUILD-HANDOFF.md` §4 carries the recut scales from the build inventory (TIME, MOD RATE and DAMPING are all skewed, DAMPING is a frequency, and the 0–10 legend is retired). Everything else here still stands.

Revision: conformance pass, 2026-08-09. All ratios below recomputed with the WCAG relative-luminance formula against the named substrate. Supersedes the pre-pass spec.

Identity is settled and **unchanged** by this pass. See *What is unchanged* at the end.

---

## 1. Contrast palette

All ratios measured against the fascia gradient mid tone **#131211** (relative luminance 0.00612) unless a different substrate is named. Substrates used:

| Substrate | Hex | Rel. luminance |
|---|---|---|
| Fascia (mid of #1b1a18 → #131211 → #0e0e0c) | `#131211` | 0.00612 |
| Button face (mid of #2b2924 → #171613) | `#211f1b` | 0.01383 |
| PROGRAM LCD | `#071009` | 0.00436 |
| Scope glass | `#04060a` | 0.00178 |
| Program dropdown | `#060d09` | 0.00347 |
| Foot label window (mid of #0a0a09 → #141312) | `#0f0f0e` | 0.00475 |
| Meter glass | `#05080a` | 0.00228 |

### Tokens

| Token | Hex | Where |
|---|---|---|
| `FN` | `#a8a294` | function line, model line, section labels, sub-headings, printed scale numerals, foot strip |
| `FN-BTN` | `#b0aa9c` | unselected button labels (sits on the lighter button face) |
| `FN-KNOB` | `#c3bcae` | knob names, REPEATS LIVE |
| `BTN-ON` | `#f0eade` | selected button labels |
| `LCD-DIM` | `#93a894` | scope readout base, dropdown rows |
| `LCD-HI` | `#a9bda9` | scope readout highlight, bank tag, chevron |
| `FOOT` | `#aca492` | typed channel strip in the recessed window |
| `UNLIT` | `#615c54` | unlit labels in multi-label stacks — **intentionally dim, unchanged** |

### Corrections — the roles you listed

| Role | Was | Ratio | Now | Ratio | Target |
|---|---|---|---|---|---|
| TEMPO-SYNCED DELAY | `#8e887d` | 5.32 | `#a8a294` | **7.36** | 7:1 |
| MODEL DL-88 · STEREO | `#6d685f` | 3.38 | `#a8a294` | **7.36** | 7:1 |
| PROGRAM label | `#7e786e` | 4.28 | `#a8a294` | **7.36** | 7:1 |
| Caret | `#6f7a70` | 4.18 | drawn chevron, stroke `#a9bda9` on LCD | **9.69** | 4.5 min |
| Scope readout | `#7d8d7e` | 5.33 | `#93a894` on scope glass | **7.99** | 7:1 |
| Scope readout, highlighted | `#a9bda9` | 9.39 | `#a9bda9` unchanged | **10.18** | passes |

Function and model line now share one colour and separate by **size and weight only** — 13px/600 and 12px/500. No opacity is used anywhere for hierarchy.

### Full audit — every remaining text role

| Role | Was | Ratio | Now | Ratio | Substrate |
|---|---|---|---|---|---|
| REPEATS LIVE | `#c3bcae` | 9.91 | unchanged | 9.91 | fascia |
| REPEAT TIMELINE | `#79746b` | 3.99 | `#a8a294` | **7.36** | fascia |
| Section labels (TIMING, REPEATS, OUTPUT, DELAY CHARACTER) | `#8d877c` | 5.24 | `#a8a294` | **7.36** | fascia (chip bg is #131211) |
| STEREO MODE heading | `#79746b` | 3.99 | `#a8a294` | **7.36** | fascia |
| CHARACTER · ALL MODES | `#6d685f` | 3.38 | `#a8a294` | **7.36** | fascia |
| NOTE DIVISION | `#a49d92` | 6.96 | `#a8a294` | **7.36** | fascia |
| SYNC ON / OFF | `#a49d92` | 6.96 | `#a8a294` | **7.36** | fascia |
| IN / OUT meter labels | `#7e786e` | 4.28 | `#a8a294` | **7.36** | fascia |
| Division button labels, unselected | `#857f75` | 4.74 | `#b0aa9c` | **7.11** | button face |
| Division button labels, selected | `#f0eade` | 13.73 | unchanged | 13.73 | button face |
| Stereo mode labels, unselected | `#857f75` | 4.74 | `#b0aa9c` | **7.11** | button face |
| Character mode labels, unselected | `#857f75` | 4.74 | `#b0aa9c` | **7.11** | button face |
| Knob names (FEEDBACK, MIX, TIME, …) | `#a9a297` / `#c3bcae` | 7.39 / 9.91 | `#c3bcae` throughout | **9.91** | fascia |
| Printed scale numerals | — (did not exist) | — | `#a8a294` | **7.36** | fascia |
| Bank tag (FACT / USER) | `#9fb2a2` | 8.61 | `#cfd8cb` | **13.19** | LCD |
| Program name | `#cfd8cb` | 13.19 | unchanged | 13.19 | LCD |
| Dropdown rows, unselected | `#7d8a7e` | 5.43 | `#93a894` | **7.73** | dropdown |
| Dropdown bank tag | `#5d675e` | 2.86 | `#93a894` | **7.73** | dropdown |
| Meter values | `#b9c3c8` | 11.19 | unchanged | 11.19 | meter glass |
| Foot channel strip | `#a09883` | 6.68 | `#aca492` | **7.74** | foot window |
| DL-88 · TOURING SPEC · 5U | `#57534c` | 2.45 | `#a8a294` | **7.36** | fascia |
| SN 0417 · v1.0 | `#57534c` | 2.45 | `#a8a294` | **7.36** | fascia |
| Multi-label stack, lit | `#e7e1d4` | 14.36 | unchanged | 14.36 | fascia |
| SAVE label | `#cec7ba` | 9.53 | `#d3ccbe` | **10.30** | button face |
| DELETE label, disabled | `#4d4941` | 1.99 | `#6f6a61` | 3.06 | button face — **intentional**, see note |
| Multi-label stack, unlit | `#615c54` | 2.82 | **unchanged — intentional** | 2.82 | fascia |
| Rack-ear stencil, RACK 4 · MON WORLD | `rgba(30,28,25,.62)` → composites `#494641` | 2.73 | `rgba(24,22,19,.92)` → composites `#211f1c` | **4.78** | left ear `#8e8a82` |
| Tape lettering, all four strips | — | — | `#23211c` | **11.3** | tape `#ded8c8` |

**The two rows below target are deliberate**, and neither is a legibility failure:

- *Multi-label stack, unlit* (2.82) — dimness is the state signal, as you specified; the lit entry and its LED carry the live identity.
- *DELETE, disabled* (3.06) — a disengaged control, the same case the brand's bypass rule exempts: the job is to read as unavailable. It was raised from 1.99 anyway, since at that value it read as absent rather than disabled.

### Size floor

Every functional string was raised to **11px minimum**, except printed scale numerals at **10px**, which sit exactly on the floor. Nothing functional is below 10px.

Sizes raised: section labels 10→11, sub-headings 10→11, knob names 10→11, multi-label stack entries 10→11, IN/OUT labels 10→11, PROGRAM label 10→11, foot spec lines 10→11, function line 12→13, stereo/division button labels 11→12.

---

## 2. Header geometry

The whole header row now sits on **32px**.

| Element | Was | Now |
|---|---|---|
| PROGRAM LCD bar | 44px tall | **32px** |
| SAVE | 66 × 44 | **66 × 32** |
| DELETE | 74 × 44 | **74 × 32** |
| IN / OUT readout boxes | 74 × 32 | 74 × 32 (unchanged — this was the reference height) |

Type inside the reduced boxes:

| Element | Was | Now | Note |
|---|---|---|---|
| Program name | Share Tech Mono 19px | **19px, unchanged** | fits 32px with 6.5px above/below |
| Bank tag | Share Tech Mono 14px | **19px** | matches the program name exactly — same face, size, tracking (.12em), colour and glow; pad 0 12px |
| SAVE / DELETE labels | Barlow Condensed 600 12px | **12px, unchanged**, tracking .18em → **.16em** | tracking reduced so DELETE keeps 6px side margin in a 74px box |
| Rename field | 18px, inset 5px | **17px, inset 3px** | |
| Dropdown offset | `top: 46px` | **`top: 34px`** | |

Nothing dropped below the 10px floor, so no further size changes were needed.

---

## 3. Chevron

Replaces the Share Tech Mono `▼` glyph. Drawn, not typographic — no font dependency, identical on every platform.

```
viewBox   0 0 14 8
path      M1 1.6 L7 6.4 L13 1.6
stroke    #a9bda9        (9.69:1 on the LCD substrate — the chevron keeps this dimmer
                         green; only the bank tag was promoted to the name treatment)
width     1.6
linecap   round
linejoin  round
fill      none
```

Rendered box **14 × 8 px**, vertically centred in the 32px LCD bar, with 11px horizontal padding either side. Included angle ≈ 77°; that is the wider, lighter chevron TapeRot and Gatecrasher use. In JUCE, build it as a `Path` with `PathStrokeType (1.6f, curved, rounded)` and scale to the 14 × 8 box; do not bake it into the LCD plate.

---

## 4. Where live values appear

Two displays, no overlap, no tooltip. **The value tooltip is removed entirely.**

**PROGRAM LCD — transient, per-control.**
While a control is being moved, the LCD shows `NAME: value` in Share Tech Mono 19px: `FEEDBACK: 62 %`, `MOD RATE: 0.45 Hz`, `TIME: 375 ms`, `OUTPUT TRIM: +2.5 dB`. It reverts to the program name **900 ms** after release. Only direct user manipulation triggers it — host automation must never drive it.

**Scope readout strip — standing, whole-effect.**
The 22px strip at the top of the Repeat Timeline holds only state that describes the effect as a whole, and never a single control's value:

- `RPT ENV` — strip identifier
- character mode and topology — `BBD BUCKET-BRIGADE`
- stereo mode — `PING-PONG`
- feedback percentage — `FB 62%` *(whole-effect state, and it is what the drawn envelope is a picture of)*
- scope division — `375 ms / DIV`
- sync state — `1/8. · 120 BPM · 375 ms` or `FREE · 375 ms`
- `0 dB` reference

**Rule for the build:** if a string answers *what is this delay doing*, it belongs in the scope strip. If it answers *what is this knob set to*, it belongs in the PROGRAM LCD and only while the knob is moving. `FB 62%` is the one value that appears in the strip, because the envelope drawn beneath it is a direct plot of it; it also appears in the LCD as `FEEDBACK: 62 %` while that knob is moved, which is the intended and only duplication.

Nothing outside those two displays carries dynamic text. No panel label rewrites itself.

---

## 5. Printed scales

**Finding: the knobs previously carried tick rings only — decorative rings at even 24°/27° intervals, with no numerals and no relationship to parameter values.** Both problems are fixed. Ticks are now placed at real values, computed by rotation fraction, and every knob carries numerals.

Common geometry, all knobs:

- Sweep **270°**, from **−135°** (minimum) to **+135°** (maximum), 0° = pointer up.
- Rotation fraction `f` → angle `−135 + 270f`.
- Both tick rings end on a common outer tip at radius **`R + 17`**, where `R` = knob radius.
- Major tick: 2 × 9px, `#a8a294`, spanning `R + 8` → `R + 17`.
- Minor tick: 1.5 × 5px, `#8a857a`, spanning `R + 12` → `R + 17`.
- Numerals: Barlow Condensed 600 10px, `#a8a294`, anchored **by the box edge facing the dial**, not by their centre — so a one-character and a three-character label clear their tick tip by the same amount. Effective radius `r = R + 21 + halfWidth·|sin θ| + 5·|cos θ|` (θ measured from vertical, half cap height 5px, ~2.7px per character at this size), which places the label box's inner edge at a constant `R + 21` — a 4px gap from the `R + 17` tick tip on every mark. Positioning numerals at a fixed radius instead leaves wide labels visibly tighter to the arc than narrow ones on the same ring.
- **Dial centre = centre of the printed tick arc = the needle pivot.** Every knob is drawn inside a square wrapper of `2R + 68` px with the knob body centred in it; the arc, the numerals and the pivot all share that wrapper's centre. The knob name sits *outside* the wrapper, below it, so the element centre and the arc centre do not diverge — the TapeRot 7.27px error cannot recur here.

### Per-knob mark tables

Angles are given to 0.1°. **These are the contract**; the exported plate must match.

**FEEDBACK** — 84px, 0–110 %, linear.

| Value | f | Angle | Type |
|---|---|---|---|
| 0 | 0.000 | −135.0 | major, "0" |
| 20 | 0.182 | −85.9 | major, "20" |
| 40 | 0.364 | −36.8 | major, "40" |
| 60 | 0.545 | +12.3 | major, "60" |
| 80 | 0.727 | +61.4 | major, "80" |
| 100 | 0.909 | +110.5 | major, "100" |
| 110 | 1.000 | +135.0 | minor (unity-plus end stop, unlabelled) |

**MIX** — 82px, 0–100 %, linear. **CROSS-FEED** — 62px and **DAMPING**, **SATURATION** — 66px, same range and taper; the 62/66px knobs label 0/50/100 only and carry minors at 25/75, to keep numerals from crowding.

| Value | f | Angle | MIX | CROSS-FEED / DAMPING / SATURATION |
|---|---|---|---|---|
| 0 | 0.00 | −135.0 | major, "0" | major, "0" |
| 25 | 0.25 | −67.5 | major, "25" | minor |
| 50 | 0.50 | 0.0 | major, "50" | major, "50" |
| 75 | 0.75 | +67.5 | major, "75" | minor |
| 100 | 1.00 | +135.0 | major, "100" | major, "100" |

**OUTPUT TRIM** — 62px, −24…+12 dB, linear. **Superseded — the parameter is −24…+24.** This table was cut against the wrong range and put 0 dB at +45°. It is kept here as the record of the pass; BUILD-HANDOFF §4.4 carries the correct nine marks.

| Value | f | Angle | Type |
|---|---|---|---|
| −24 dB | 0.000 | −135.0 | major, "-24" |
| −18 dB | 0.167 | −90.0 | minor |
| −12 dB | 0.333 | −45.0 | major, "-12" |
| −6 dB | 0.500 | 0.0 | minor |
| 0 dB | 0.667 | +45.0 | major, "0" |
| +6 dB | 0.833 | +90.0 | minor |
| +12 dB | 1.000 | +135.0 | major, "+12" |

**TIME** — 66px, 1–2000 ms, **skewed**. Required before the plate is cut:

```
NormalisableRange<float> (1.0f, 2000.0f, 1.0f, 0.300f)
skew exponent   0.300
centre value    200 ms at 50% rotation
f(v) = ((v - 1) / 1999) ^ 0.300
```

| Value | f | Angle | Type |
|---|---|---|---|
| 1 ms | 0.000 | −135.0 | major, "1" |
| 10 ms | 0.198 | −81.6 | major, "10" |
| 50 ms | 0.329 | −46.3 | minor |
| 100 ms | 0.406 | −25.4 | major, "100" |
| 200 ms | 0.500 | 0.0 | minor |
| 500 ms | 0.660 | +43.1 | major, "500" |
| 1000 ms | 0.812 | +84.3 | minor |
| 2000 ms | 1.000 | +135.0 | major, "2K" |

Note the marks are **not** evenly spaced — 100 ms sits at −25.4°, not at the −27° an even seven-step ring would put it. That is the point of the skew, and the old decorative ring was wrong here specifically.

### The three multi-label Delay Character dials

**This is the case that could not take a numbered scale, and the reason is not space — it is arithmetic.** Dial 1 is WOW (0–100 %) in TAPE, MOD RATE (0.05–8 Hz) in BBD and REPEAT DEGRADE (0–100 %) in DIGITAL. One printed ring cannot legend three different ranges and two different units; a "%" scale would be actively wrong two-thirds of the time, and the panel is forbidden from relabelling itself.

**Resolution: a normalised 0–10 dial scale**, the standard hardware answer for a multi-function control (Roland, Boss and Korg all shipped arbitrary 0–10 legends for exactly this reason). It is a true legend — the pointer at 6 always means 60% of travel, in every mode — and it is honest about what it can and cannot tell you. The unit-bearing value is available on move in the PROGRAM LCD, which every mode shares.

Applied to all three dials so the row is consistent, including GENERATION LOSS which has only one identity; a single dial on a different scale from its two neighbours reads as an error.

| Mark | f | Angle |
|---|---|---|
| 0 | 0.0 | −135.0 |
| 2 | 0.2 | −81.0 |
| 4 | 0.4 | −27.0 |
| 6 | 0.6 | +27.0 |
| 8 | 0.8 | +81.0 |
| 10 | 1.0 | +135.0 |

Space: 76px dial + 22px numeral radius = 136px wrapper in a 140px column, with the three-entry label stack below it at 11px/5px gap. Both fit at full size; no type was shrunk to make room.

Underlying parameter ranges the 0–10 scale maps onto — all linear, so 0–10 is a true linear legend in every mode:

| Dial | TAPE | BBD | DIGITAL |
|---|---|---|---|
| 1 | WOW 0–100 % | MOD RATE 0.05–8 Hz | REPEAT DEGRADE 0–100 % |
| 2 | FLUTTER 0–100 % | MOD DEPTH 0–100 % | — |
| 3 | GENERATION LOSS 0–100 % | — | — |

**MOD RATE is the one to confirm with the build.** If it ships with a skew rather than linear, the 0–10 legend stays correct (it legends travel, not value) but the LCD mapping changes; get the exponent from the parameter inventory before the plate is cut.

---

## 5a. Stencil vs tape

Permanent identification is stencilled; per-show information is written on tape. Four tape elements, no more.

| Element | Kind | Why |
|---|---|---|
| FIFTH MEMBER nameplate | tape — largest, most deliberate | the unit's name, but still crew-applied |
| DLY 4, right ear | tape — small cable label | patch identity, unchanged by this pass |
| HALDEN HALL · LOAD-IN 06 | **tape (was stencil)** | tonight's venue and load-in slot |
| CH 4 — GTR / STAGE LEFT | **tape (was recessed label window)** | patching changes every show |
| RACK 4 · MON WORLD | **stays stencil** | which rack, whose world — years, not nights |

Hierarchy, largest to scrappiest: nameplate (27px marker, 268px wide) → CH 4 scribble strip (15px) → HALDEN HALL (14px) → DLY 4 (12px).

### RACK 4 · MON WORLD — stencil, corrected

Sprayed ink on brushed aluminium, rotated −90°, centred on the left ear. Barlow Condensed 600 11px, tracking .34em. Ink raised from `rgba(30,28,25,.62)` to **`rgba(24,22,19,.92)`**, which composites to `#211f1c` on the `#8e8a82` ear mid tone: **4.78:1**, up from 2.73:1. Flavour text, so it clears the 3:1 flavour floor with margin without becoming a hard black that would read as printed rather than sprayed.

### HALDEN HALL · LOAD-IN 06 — tape strip

| Property | Value |
|---|---|
| Element | 36 × 252 px, right ear |
| Placement | `left: 8px`, `bottom: 48px` within the 52px ear |
| Rotation | **+1.6°** |
| Face | `linear-gradient(96deg, #efeadc, #ded8c8 55%, #cdc7b6)` |
| Torn ends | `clip-path: polygon(0% 2%, 34% 0%, 66% 2.4%, 100% 0.6%, 100% 97%, 62% 100%, 30% 97.6%, 0% 99%)` — both short ends only; long edges cut straight, as tape off a roll |
| Weave | `repeating-linear-gradient(4deg, rgba(0,0,0,.05) 0 1px, rgba(255,255,255,.05) 1px 5px)` |
| Shadow | `0 2px 5px rgba(0,0,0,.5)`, inset `0 1px 0 rgba(255,255,255,.55)` |
| Lettering | Permanent Marker **14px**, `#23211c`, tracking .02em, rotated −90°, centred along a 244px run with `white-space: nowrap` |
| Contrast | **11.3:1** on the tape face |

### CH 4 — GTR / STAGE LEFT — scribble strip

Replaces the recessed label window entirely; the window, its inset shadow and its `#33312b` border are gone.

| Property | Value |
|---|---|
| Element | 236 × 34 px, foot row, second — after DL-88 · TOURING SPEC · 5U |
| Rotation | **−1.1°** |
| Face | `linear-gradient(178deg, #f0ebdd, #e0dac9 58%, #cec8b7)` |
| Torn ends | `clip-path: polygon(1.6% 0%, 0.4% 34%, 2.2% 68%, 0% 100%, 98.6% 100%, 100% 64%, 98% 32%, 99.4% 0%)` — left and right ends only |
| Weave | `repeating-linear-gradient(88deg, rgba(0,0,0,.05) 0 1px, rgba(255,255,255,.05) 1px 5px)` |
| Shadow | `0 2px 6px rgba(0,0,0,.5)`, inset `0 1px 0 rgba(255,255,255,.55)` |
| Lettering | Permanent Marker **15px**, `#23211c`, tracking .01em, centred |
| Contrast | **11.3:1** on the tape face |

Both strips are baked into the plate — neither carries a live value.

---

## 6. Asset format

Vector / code-drawn, as before. Specifically:

- **Baked into the plate:** fascia, wear, nameplate, all four tape strips, rack ears, rack-ear stencil, section frames, all printed labels, all tick marks, all scale numerals — **except Dial 1's two rings and the three Delay Character multi-label stacks**, both of which change ink with the selected mode and are therefore drawn at runtime (BUILD-HANDOFF §1 carve-out).
- **Drawn at runtime:** pointer lines, LEDs, button pressed/raised states, backlit Program legends (SAVE/STORE, DELETE/CANCEL — see BUILD-HANDOFF §1.3), LCD text, scope canvas, meter values, chevron.
- Nothing carrying a live value is baked.

Button states needing a sprite each: division ×5 (raised/pressed × LED on/off), stereo ×3, character ×3, SAVE (normal/hover), DELETE (**enabled/disabled** — two faces, currently `#d3ccbe` and `#6f6a61`).

---

## 7. What is unchanged

Explicitly untouched by this pass:

- **Whole visual identity.** Road-worn black rack chassis, fascia gradient, corner wear to bare metal, scuff streaks, brushed aluminium rack ears with screws, gaffer-tape nameplate in Permanent Marker, "DLY 4" cable-tape label, "RACK 4 · MON WORLD" rotated stencil (ink darkened only — see §5a), "DLY 4" cable tape. **Changed by this pass:** "HALDEN HALL · LOAD-IN 06" and "CH 4 — GTR / STAGE LEFT" are now tape rather than stencil and recessed window respectively.
- **Accent colour** `#ff9d3c`, still used only for the REPEATS LIVE lamp and the scope pulses.
- **Unlit multi-label stack entries** at `#615c54` — deliberately dim, LED carries the state.
- **Panel architecture** — header composition, three fixed Delay Character dials with permanent stacked labels, Repeat Timeline scope, section grouping, control positions and sizes.
- **Program management** — SAVE always creates a new User Program, DELETE disabled on Factory, single FACT/USER tag in the LCD.
- **Knob bodies, pointers, LED treatment, all interaction behaviour** apart from the removal of the tooltip.
- **Canvas** 1240px design width, proportional scaling.
- **Wordmark, function and model wording**; `0 dB` scope reference; IN/OUT meter geometry.

---

## 8. Still outstanding

**The plate has since been cut** — `plate/fifth-member-plate-{1,2,3}x.png`, 1240 × 931 at 1×. The parameter inventory that blocked it arrived and every ring in BUILD-HANDOFF §4 was recut against it.

- Read-only parameter inventory from the build to confirm every range, taper and skew in §5 — in particular **MOD RATE** and **TIME**. The TIME skew of 0.300 is a proposal chosen to put 200 ms at half rotation; if the build already has a different exponent, its marks must be recomputed before the plate is cut.
- By-ear tuning of the six factory Programs.
