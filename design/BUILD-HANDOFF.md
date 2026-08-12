# Fifth Member — Build Handoff

Model **DL-88**, tempo-synced stereo delay. Neon Foundry suite.
Panel approved 2026-08-09. This document is **authoritative for the build** and supersedes any conflicting figure in `README.md` or `GUI-SPEC.md`.

Canvas **1240 × 931 px** at 1×, fixed aspect, proportional scaling only. Left ear 52px, fascia 1136px, right ear 52px. Fascia padding 16px top / 18px sides / 12px bottom.

---

## 1. Asset format — decided, per element

**Decision: one exported bitmap for the entire static panel plate; everything that moves or lights is drawn at runtime.** No knob filmstrips. The reasoning is below the table; the table is the contract.

### Exported bitmap — the plate

One PNG, 1240 × 931 at 1× / 2× / 3×, alpha, sRGB.

**Delivered** — `plate/fifth-member-plate-1x.png` (1240 × 931), `-2x.png` (2480 × 1862), `-3x.png` (3720 × 2793). Cut from the approved prototype with every runtime layer suppressed. In the prototype source each such layer carries `data-plate="off"` (hidden entirely) or `data-plate="blank"` (well kept, contents dropped) — that attribute is the machine-readable form of the two tables below, so a recut stays in step with the panel.

It contains:

| Element | In the plate |
|---|---|
| Chassis, fascia gradient, vertical brush texture, corner wear, scuff streaks | yes |
| Rack ears (brushed gradient, grain, highlights) and the four screws | yes |
| RACK 4 · MON WORLD stencil | yes |
| All four tape elements, including their lettering | yes |
| Section frames, their labels, the header rule and foot rule | yes |
| Panel typography — function line, model line, section headings, knob names, foot strip, IN/OUT captions, PROGRAM caption | yes |
| **Delay Character multi-label text (WOW / MOD RATE / REPEAT DEGRADE, etc.)** | **no** — see the carve-out below |
| Every printed tick mark and scale numeral | yes |
| Empty wells and frames: PROGRAM LCD recess, IN/OUT meter wells, scope well and its 1px border | yes — **frames only, never contents** |
| Knob bodies, LEDs, buttons, switch | **no** |
| **Dial 1's two scale rings — ticks and numerals** | **no** — see the carve-out below |

#### Carve-out: what changes ink is not in the plate

**Baked pixels cannot change ink.** Two things on this panel change ink with the mode, and neither is in the plate — the same wall Gatecrasher hit with its §0.4 labels and resolved the same way:

1. **Dial 1's inner and outer rings.** §4.5 requires each to light with its mode and dim when the other is live. Both are drawn at runtime in full — ticks, minors and numerals, at §4.5's angles, in the lit or dimmed ink of §8.3.
2. **All three Delay Character multi-label stacks** — the *text* only; the LED beside each entry was already runtime. The lit entry is `#e7e1d4` and the unlit ones `#615c54`, and which is which follows the selected mode. Baking them would freeze one mode's lighting into the bitmap: the plate would print WOW lit while the pointer drove MOD RATE.

So "all panel typography is baked" is **false** in exactly one place — those three stacks. Every other printed word on the panel, including the knob names under dials 4–8 and the CHARACTER · ALL MODES heading, is in the plate.

The other five dials keep their rings in the plate — none of them changes ink.

`KnobScale::bakedInPlate` is `true` for TIME, FEEDBACK, CROSS-FEED, MIX, OUTPUT TRIM, DAMPING and SATURATION, and `false` for Dial 1's two rings. One table describes where every mark sits either way.

### Runtime-drawn

| Element | How |
|---|---|
| Knob bodies + pointers | vector, per §1.2 |
| LEDs | vector, per §2.1 |
| Mode buttons (division ×5, stereo ×3, character ×3) | vector rounded rects, per §1.3 |
| SAVE·STORE / DELETE·CANCEL button faces | vector, per §1.3 — face never changes |
| The four Program-button legends | text, lit or dark per §1.3.1 |
| Sync switch | vector, per §1.4 |
| SYNC ON / OFF caption | text, per §8.2 — state-dependent, so not baked |
| REPEATS LIVE lamp | vector, accent, 1.6s pulse |
| Dial 1's two scale rings (ticks, minors, numerals) | vector + text, per §4.5 — `bakedInPlate = false` |
| Delay Character multi-label stack text (3 stacks) | text, lit `#e7e1d4` / unlit `#615c54` per selected mode, per §4.5 |
| Scope trace and grid | canvas / `Graphics` per §5 |
| PROGRAM LCD text, bank tag, chevron, dropdown | vector + text, per §6 |
| IN/OUT numerals | text |
| Scope readout strip contents | text |

**Nothing carrying a live value is baked.** The scope trace, the PROGRAM LCD text, the IN/OUT numerals and the readout strip contents are all runtime. Their wells, frames and captions are in the plate.

### 1.1 Why not filmstrips

Five diameters (62, 66, 76, 82, 84) × 128 frames × 3 scale factors is 1,920 frames and roughly 60 MB of atlas for six visually identical knob designs that differ only in radius. The knob is a flat-shaded cylinder with one static specular and one rotating pointer line — the specular does **not** rotate with the knob, so a filmstrip would encode 128 copies of an unchanging body. Draw it.

If the build nevertheless wants filmstrips, export **each of the five diameters at 128 frames**, vertical strip, 0-indexed frame 0 at −135° through frame 127 at +135° (step 2.1260°), alpha, at 1×/2×/3×. The pointer must be the only thing that differs between frames.

### 1.2 Knob body — drawn primitives

Two families. `D` = body diameter.

**Large family — 76, 82, 84 px** (Delay Character dials, MIX, FEEDBACK)

```
body fill     radial gradient, centre at (38%, 28%) of the bounding box
              stop 0%    #55524a      (76px dial: #514e46)
              stop 55%   #2a2823      (76px dial: #282621)
              stop 100%  #131210      (76px dial: #121110)
border        1px  #090908
inner light   inset 0 +1px 1px rgba(255,255,255,0.18)     (76px: 0.17)
inner shade   inset 0 −8px 16px rgba(0,0,0,0.70)          (76px: 0 −7px 14px 0.68)
drop shadow   0 +8px 16px rgba(0,0,0,0.55)                (76px: 0 +7px 14px 0.52)
pointer       3.0 × 33 px, radius 1px, top inset 7px      (76px: 2.5 × 30, inset 7px)
              vertical gradient #f6f1e6 → #b8b1a2         (76px: #f5f0e5 → #b7b0a1)
              pivot at 50% of body width, 35px from pointer top (76px: 31px)
```

**Small family — 62, 66 px** (CROSS-FEED, OUTPUT TRIM, TIME, DAMPING, SATURATION)

```
body fill     radial gradient, centre at (38%, 28%)
              stop 0%    #4c4942
              stop 55%   #262420
              stop 100%  #121110
border        1px  #090908
inner light   inset 0 +1px 1px rgba(255,255,255,0.16)
inner shade   inset 0 −6px 12px rgba(0,0,0,0.65)
drop shadow   0 +6px 12px rgba(0,0,0,0.50)
pointer       2.0 × 26 px (66px body) / 2.0 × 24 px (62px body), radius 1px,
              top inset 6px, gradient #f4efe4 → #b6afa1
              pivot 27px (66px) / 25px (62px) from pointer top
```

The specular is the gradient's off-centre origin at (38%, 28%) — it is **fixed to the panel**, not to the knob, and must not rotate. Pointer motion is eased `cubic-bezier(.18,.9,.2,1)` over 620ms only on program recall; direct drags track the pointer with no easing.

### 1.3 Buttons — drawn

All mode buttons and SAVE/DELETE share one construction. Radius 3px, 1px border `#0a0a08`.

```
face          linear-gradient 180°  #2b2924 → #171613
face, hover   linear-gradient 180°  #332f29 → #1b1a16
raised        inset 0 +1px 0 rgba(255,255,255,0.13) , 0 +3px 6px rgba(0,0,0,0.50)
pressed       inset 0 +2px 6px rgba(0,0,0,0.75) , 0 +1px 0 rgba(255,255,255,0.10)
```

Selected = pressed shadow + lit LED + `#f0eade` label. Unselected = raised + unlit LED + `#b0aa9c` label.

Sizes: division buttons 5 across at 30px tall, 5px gap; stereo buttons 3 across at 32px tall, 5px gap; character buttons 158 × 54 stacked, 12px gap.

Section boxes: TIMING 296 fixed, REPEATS flexible, OUTPUT 322 fixed, DELAY CHARACTER full width. See §4.1.

SAVE 66 × 32 content (**34 border-box**, the suite header height), face `#2a2823 → #161512`, inset light 0.14.
DELETE 74 × 32 content (**34 border-box**), face `#242219 → #141310`, inset light 0.10.

**One face each. No disabled face, no pressed variant** — see §1.3.1.

### 1.3.1 The two Program buttons carry two legends each

SAVE sits above STORE; DELETE above CANCEL. Resting function on top, what the button becomes while a Program is being named beneath it. **Neither button ever changes its face or its text** — only which legend is lit.

Layout inside the 34px border-box: column, centred, **2px gap**, both legends Barlow Condensed **600 10px**, tracking .14em, `line-height: 1`. 10px is the brand floor for functional text and both legends are functional, so neither is set smaller than the other.

Fifth Member's fascia is near-black, so the indication is **backlit — the legend itself lights.** No lamp beside it; there is no pale face here that would need one. Do not mix the two forms on this panel.

| Legend | Lit | Dark |
|---|---|---|
| Ink | `#f2ece0` | `#77736a` |
| Glow | `0 0 7px rgba(242,236,224,.55), 0 0 15px rgba(242,236,224,.25)` | none |
| On SAVE face `#211f1b` | 13.98:1 | **3.48:1** |
| On DELETE face `#1c1a15` | 14.71:1 | **3.66:1** |

Both weights are 600/10px in both states. **Ink weight and colour never stand in for illumination** — the change is luminance plus bloom, which reads as a lamp; a bolder or blacker legend would read as emphasis.

The lit ink is a **neutral bright, not the accent** and not any of the three mode LED colours (Tape / BBD / Digital) — the accent stays reserved for the live delay process.

Dark is not absent. Both legends dark is the resting state of an unmodified Factory Program and must read as *nothing to do here*, never as a blank button, which is what the 3:1 state floor buys.

**Which legend is live**

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | dark | dark | dark | dark |
| Factory Program, edited | **lit** | dark | dark | dark |
| User Program, unmodified | dark | dark | **lit** | dark |
| User Program, edited | **lit** | dark | **lit** | dark |
| Naming a Program | dark | **lit** | dark | **lit** |

SAVE's lamp and the LCD's trailing ` *` read **the same edited flag**, so they cannot disagree. The flag sets on any parameter move or switch change and clears on Program recall and on a completed store; Escape out of naming leaves it set, because nothing was stored.

Delivered faces: `plate/buttons/01-rest-nothing-to-do.png`, `02-edited-factory-save-lit.png`, `03-edited-user-save-delete-lit.png`, `04-naming-store-cancel-lit.png` — 3×, the button pair as a unit.

### 1.4 Sync switch — drawn

Track 52 × 24, radius 2px, fill `#0b0a09`, 1px border `#2b2924`, inset `0 +2px 5px rgba(0,0,0,0.8)`.
Thumb 23px wide, inset 2px top and bottom, radius 1px, gradient 180° `#cfc9bb → #8d887d`, shadow `0 +1px 3px rgba(0,0,0,0.7)`. Travel: `left: 2px` (off) → `left: 27px` (on), 180ms ease.

---

## 2. LEDs and the multi-label stacks

### 2.1 LED geometry and colour

One lamp design at three diameters. Circular, no border.

| Use | Ø |
|---|---|
| Multi-label stack entries | 6 px |
| Division buttons | 6 px |
| Stereo buttons, knob-name LEDs (TIME, CROSS-FEED), NOTE DIVISION heading | 7 px |
| Character mode buttons | 10 px |

```
lit      radial gradient, centre (35%, 30%)
         #ffffff → #efe9d6 at 45% → #b9b09a at 100%
         glow  0 0 8px rgba(240,236,220,0.75) , 0 0 18px rgba(230,224,205,0.35)
         (10px character LEDs: 0 0 10px / 0 0 22px, same colours)
         (6px stack LEDs:      0 0 7px  / 0 0 15px, same colours)

unlit    radial gradient, centre (35%, 30%)
         #3a3833 → #191816
         inset 0 +1px 2px rgba(0,0,0,0.8) , no glow
         (stack LEDs use #34322d → #171614)
```

The LED is not the accent colour. `#ff9d3c` is reserved for the REPEATS LIVE lamp and the scope pulses and appears nowhere else.

### 2.2 The knob body never changes

**The knob body, its pointer, its printed ring and its numerals are identical whether the parameter that knob currently drives is engaged or not.** No dimming, no desaturation, no reduced-contrast pointer, no greyed ring. Only the LED changes. This applies to every case in §2.3 and §2.4. A build that dims an inactive dial is wrong.

### 2.3 Multi-label stacks

Three knob positions in DELAY CHARACTER carry stacked labels. Each column is 140px wide; the dial wrapper is 144 × 144 with a 76px body; the stack sits directly beneath it, left-aligned, 5px between rows.

Each row: 6px LED, 7px gap, then the label — Barlow Condensed 600 **11px**, tracking .18em.

| State | LED | Label colour | Contrast |
|---|---|---|---|
| lit | lit 6px lamp, glow 0 0 7px / 0 0 15px | `#e7e1d4` | 14.36:1 on fascia |
| unlit | `#34322d → #171614`, inset shadow, no glow | `#615c54` | 2.82:1 on fascia |

**Stack 1** — 3 rows: WOW / MOD RATE / REPEAT DEGRADE
**Stack 2** — 2 rows: FLUTTER / MOD DEPTH
**Stack 3** — 1 row: GENERATION LOSS

Row 1 sits at the top of the stack; rows are in the order listed. Stack 3's single row occupies row 1 and leaves rows 2–3 empty — the column does not re-centre.

### 2.4 The only other LEDs on the panel

| Control | LED | Lit when |
|---|---|---|
| NOTE DIVISION heading | 7px, left of the heading | Sync ON |
| Division buttons (×5) | 6px, inside each button | Sync ON **and** that division selected |
| TIME knob name | 7px, left of the name | Sync **OFF** |
| CROSS-FEED knob name | 7px, left of the name | Stereo mode = PING-PONG |
| Stereo buttons (×3) | 7px, inside each button | that mode selected |
| Character buttons (×3) | 10px, inside each button | that mode selected |

**No other control has an LED.** FEEDBACK, MIX, OUTPUT TRIM, DAMPING and SATURATION are unconditional and carry a name only.

---

## 3. The three Delay Character pages

Selecting a mode changes which stack labels are lit and the mode descriptor in the scope readout strip. It changes nothing else — no knob is added, removed, repositioned or dimmed, and every button and dial stays visible.

| | **TAPE** | **BBD** | **DIGITAL** |
|---|---|---|---|
| Character button lit | TAPE | BBD | DIGITAL |
| Stack 1 lit row | WOW | MOD RATE | REPEAT DEGRADE |
| Stack 2 lit row | FLUTTER | MOD DEPTH | *none lit* |
| Stack 3 lit row | GENERATION LOSS | *none lit* | *none lit* |
| Dial 1 drives | Wow 0–100 %, linear | Mod Rate 0.1–5.0 Hz, **skew 0.4090339496** | Repeat Degrade 0–100 %, linear |
| Dial 2 drives | Flutter 0–100 % | Mod Depth 0–100 % | — (holds last value, inert) |
| Dial 3 drives | Generation Loss 0–100 % | — | — |
| Dial 1 lit scale | inner percent arc | **outer Hz arc** | inner percent arc |
| Scope descriptor | `TAPE ECHO · 3-HEAD` | `BBD BUCKET-BRIGADE` | `DIGITAL · 16-BIT` |

Where a stack has no lit row in a mode, **every** row in that stack renders unlit and the dial still draws at full strength, pointing at its last value for that mode's parameter, or at its minimum if that mode does not use it. Dial 2 in DIGITAL and dial 3 in BBD/DIGITAL are the cases; per §2.2 they must not be dimmed.

DAMPING and SATURATION under CHARACTER · ALL MODES are active in all three modes and carry no LED.

**DAMPING is a cutoff frequency, not a percentage.** Range **1000–16000 Hz**, skew **0.4306765581**, 4 kHz at half rotation. Earlier revisions of this document and its state table described it as 0–100 % — that was wrong in unit, not merely in spacing, and a percentage ring on that control is incorrect. The panel label reads **DAMPING · Hz**; the LCD readout formats as `DAMPING · Hz: 4.0 kHz`.

---

## 4. Printed scales

### 4.1 Width budget

The fascia has **1100px** of usable width (1240 − 52 − 52 ears − 18 − 18 padding). Both control bands are near that ceiling, so any wrapper enlargement must be checked against the band, not the dial:

| Band | Composition | Width |
|---|---|---|
| Row A | TIMING 296 + gap 16 + REPEATS (flex) + gap 16 + OUTPUT 322 | 1092 |
| DELAY CHARACTER | modes 158 + rule/pad 22 + dials 536 + rule/pad 20 + shared 268 | 1092 |

OUTPUT is **322px**, not 300 — MIX 150 + OUTPUT TRIM 130 + 8 gap + 32 padding = 320, and a 300px box overflows it by 22px.

**Verify `panel.scrollWidth <= 1240` after any change to a knob wrapper.** Checking the dial row alone passes while the failure sits two levels up: the chassis is `overflow: hidden`, so an over-wide fascia silently pushes the rack ears outside the clip box instead of visibly breaking.

### 4.2 Sweep and centre

```
sweep          270°
start          −135°   (parameter minimum)
end            +135°   (parameter maximum)
0°             pointer straight up
angle(f)       −135 + 270·f      f = rotation fraction, 0…1
```

**Each dial's centre is the centre of its printed tick arc, which is also the needle pivot — it is *not* the centre of the control element.** Every knob is laid out inside a square wrapper of `2R + 68` px (`R` = body radius) with the body centred in it; the arc, the numerals and the pivot all share that wrapper's centre. Where a name or a label stack is printed beneath the dial it sits **outside** that wrapper, so the element bounding box is taller than the wrapper and its centre is lower. Quoting the element centre puts the pivot below the ring it sweeps.

Wrapper sizes as built:

| Body Ø | Wrapper | C | Controls |
|---|---|---|---|
| 62 | 130 | 65 | CROSS-FEED, OUTPUT TRIM |
| 66 | 134 | 67 | TIME, DAMPING · Hz, SATURATION |
| 76 | **232** | 116 | Delay Character **dial 1 only** — it carries the outer Hz arc |
| 76 | 144 | 72 | Delay Character dials 2 and 3 — one ring each |
| 82 | 150 | 75 | MIX |
| 84 | 152 | 76 | FEEDBACK |

**The build's layout constant is the pivot.** For most controls the printed name is centred on it and the composite element centre coincides. Two do not: **TIME** and **CROSS-FEED** carry an LED-plus-text caption left-aligned from the lamp, which makes the caption wider on one side, so their composite element centres sit **3.00px** and **3.58px** left of the pivot respectively. Those two offsets describe the *caption block*, not the dial — position both from the pivot and let the caption hang.

### 4.2a Tick and numeral geometry

```
major tick     2.0 × 9 px   #a8a294   spans R+8  → R+17
minor tick     1.5 × 5 px   #8a857a   spans R+12 → R+17
numerals       Barlow Condensed 600 10px, #a8a294, tracking .06em
```

Both rings end on a common outer tip at **R + 17** (dial 1's outer arc at R + 39, §4.5).

**A tick is drawn at every printed numeral and nowhere else; minor ticks fall on real values too.** There is no fixed-pitch decorative ring: no marks outside the 270° travel arc, none below the horizontal at the arc's ends, no mark at +135° without its twin at −135°. Any ring exhibiting those symptoms is a pre-conformance asset and must be discarded. Numerals are anchored **by the box edge facing the dial**, not by their centre, so a one-character and a three-character label clear the tip by the same amount:

```
r = R + 21 + halfWidth·|sin θ| + halfCapHeight·|cos θ|
    θ measured from vertical, halfCapHeight = 5 px, ≈2.7 px per character at 10px
```

This places every label's inner edge at a constant R + 21 — a 4px gap from the tick tip on every mark. A fixed numeral radius instead leaves wide labels visibly tighter to the arc than narrow ones on the same ring.

### 4.3 Skew — three parameters, from the build inventory

Earlier revisions of this document proposed a TIME skew of 0.300 and assumed MOD RATE and DAMPING were linear. **All three were wrong.** The figures below are the build's, and the tables in §4.4 carry the resulting angles verbatim.

| Parameter | Range | Skew | Half-rotation value |
|---|---|---|---|
| TIME | 1–2000 ms | **0.4135361469** | **375 ms** |
| MOD RATE (Dial 1, BBD) | 0.1–5.0 Hz | **0.4090339496** | **1.0 Hz** |
| DAMPING | 1000–16000 Hz | **0.4306765581** | **4 kHz** |

```
f(v) = ((v − min) / (max − min)) ^ skew
```

375 ms at mid-travel is deliberate: it is the default, and a dotted eighth at 120 BPM, so it is the value the knob should sit centred at.

**Marks on these three rings are placed at the angles given in §4.4 and must never be derived by even spacing.** The error is large, not marginal — under the old 0.300 proposal 200 ms sat at +0.1°; it is really at −31.00°, and 100 ms moves 31.7°, 50 ms 30.5°. A ring cut to the old figures is wrong across its whole middle.

Every other parameter on the panel is linear, so its marks fall evenly by value and may be derived from it.

### 4.4 Mark tables

**TIME** — 66px body, 1–2000 ms, **skew 0.4135361469**

Printed subset chosen for legibility at this diameter; every mark sits at the build's angle.

| Value | Printed | Angle | Type |
|---|---|---|---|
| 1 ms | `1` | −135.00° | major |
| 5 ms | — | −114.33° | minor |
| 10 ms | `10` | −106.10° | major |
| 20 ms | — | −95.63° | minor |
| 50 ms | — | −76.75° | minor |
| 100 ms | `100` | −57.08° | major |
| 200 ms | — | −31.00° | minor |
| 375 ms | `375` | 0.00° | major |
| 500 ms | — | +17.10° | minor |
| 750 ms | — | +44.91° | minor |
| 1000 ms | `1K` | +67.67° | major |
| 1500 ms | — | +104.70° | minor |
| 2000 ms | `2K` | +135.00° | major |

**DAMPING · Hz** — 66px body, 1000–16000 Hz, **skew 0.4306765581**

| Value | Printed | Angle | Type |
|---|---|---|---|
| 1 kHz | `1K` | −135.00° | major |
| 2 kHz | `2K` | −50.89° | major |
| 3 kHz | — | −21.63° | minor |
| 4 kHz | `4K` | 0.00° | major |
| 6 kHz | — | +33.22° | minor |
| 8 kHz | `8K` | +59.45° | major |
| 12 kHz | — | +101.24° | minor |
| 16 kHz | `16K` | +135.00° | major |

**SATURATION** — 66px body, 0–100 %, linear. Labels 0 / 50 / 100 at −135.00° / 0.00° / +135.00°, minors at 25 (−67.50°) and 75 (+67.50°).

**FEEDBACK** — 84px body, 0–110 %, linear

| Mark | f | Angle | Type |
|---|---|---|---|
| 0 | 0.0000 | −135.0° | major |
| 20 | 0.1818 | −85.9° | major |
| 40 | 0.3636 | −36.8° | major |
| 60 | 0.5455 | +12.3° | major |
| 80 | 0.7273 | +61.4° | major |
| 100 | 0.9091 | +110.5° | major |
| — | 1.0000 | +135.0° | minor, unlabelled (110 % end stop) |

**MIX** — 82px body, 0–100 %, linear

| Mark | f | Angle | Type |
|---|---|---|---|
| 0 | 0.00 | −135.0° | major |
| 25 | 0.25 | −67.5° | major |
| 50 | 0.50 | 0.0° | major |
| 75 | 0.75 | +67.5° | major |
| 100 | 1.00 | +135.0° | major |

**CROSS-FEED** — 62px body, 0–100 %, linear. Same angles as MIX, but only 0 / 50 / 100 are labelled; 25 and 75 are minors, to keep numerals off each other on the smaller body.

| Mark | f | Angle | Type |
|---|---|---|---|
| 0 | 0.00 | −135.0° | major |
| — | 0.25 | −67.5° | minor |
| 50 | 0.50 | 0.0° | major |
| — | 0.75 | +67.5° | minor |
| 100 | 1.00 | +135.0° | major |

**OUTPUT TRIM** — 62px body, −24…+24 dB, linear

The earlier table was cut against −24…+12, which is not the parameter. On the real range it put 0 dB at +45° where the pointer reaches it at 0°, and every other numeral was wrong by a similar amount. Recut and in the build:

| Value | Printed | Angle | Type |
|---|---|---|---|
| −24 dB | `-24` | −135.00° | major |
| −18 dB | — | −101.25° | minor |
| −12 dB | `-12` | −67.50° | major |
| −6 dB | — | −33.75° | minor |
| 0 dB | `0` | 0.00° | major |
| +6 dB | — | +33.75° | minor |
| +12 dB | `+12` | +67.50° | major |
| +18 dB | — | +101.25° | minor |
| +24 dB | `+24` | +135.00° | major |

**The printed `+12` and `+24` keep their leading plus.** It is a bipolar dB scale and the sign is the convention; the LCD formats the same parameter as `+2.5 dB`, so a ring printing a bare `12` contradicts its own readout. Labels on this ring are explicit strings, not derived from the value — deriving them drops the plus.

### 4.5 The three Delay Character dials

The normalised 0–10 legend proposed in earlier revisions is **retired**. With the build's real ranges in hand, dials 2 and 3 can carry true percentage scales, and dial 1 gets two.

**Dials 2 and 3** — 76px body, 0–100 %, linear in every mode. One ring each.

| Value | Printed | Angle | Type |
|---|---|---|---|
| 0 | `0` | −135.00° | major |
| 25 | `25` | −67.50° | major |
| 50 | `50` | 0.00° | major |
| 75 | `75` | +67.50° | major |
| 100 | `100` | +135.00° | major |

**Dial 1 — two concentric arcs.** It binds to a percentage in Tape and Digital and a frequency in BBD. These cannot share numerals: 1 Hz sits at 0.00° where the percentage ring prints `50`, and the spacing differs at every other mark. Unlike Chorus-60 this cannot be resolved by unifying the ranges — they are different quantities.

The panel already solves the equivalent problem for *labels* by printing all of them stacked with an LED showing which is live. The scale extends the same idea: **both rings are permanently printed on separate radii, and each lights or dims with its mode, exactly as the stacked labels behave.** Real multi-function hardware does this.

```
inner ring   percent   ticks R+8  → R+17   numerals inner edge at R+21
outer ring   hertz     ticks R+44 → R+53   numerals inner edge at R+57
```

**The outer band is placed against the inner numerals' OUTER extent, not their inner edge.** This is the trap: numerals are anchored by the edge facing the dial (§4.2), so their inner edge at R+21 says nothing about how far out the glyphs reach. At R = 38 the widest inner label, `100`, extends to radius **77.3**, so the outer tick ring cannot begin before ~R+43. It begins at **R+44 (radius 82)**, clearing the inner numerals by 4.7px. Placing it at R+30 — as an earlier revision of this document did — drives a 7.7 × 6.9px collision between `100` and an outer tick, with smaller ones on `50` and `0`.

Measured clearances as built: inner numerals reach 77.3, outer ticks start at 81.0, outer numerals reach 112.6. No box on either ring intersects any other.

With the outer numerals reaching 112.6px, dial 1 needs a **232 × 232 wrapper** (C = 116, body at 78, 78).

**Only dial 1 takes that wrapper.** Dials 2 and 3 carry one ring each and stay **144 wide** (C = 72, body at 34, 34). Giving all three a 232px box costs 176px the fascia does not have — it overflows the 1240px chassis by 168px and clips both rack ears off the panel. The three-dial row measures **536px** with mixed widths.

**Unequal wrappers register on the pivot, not on their boxes.** This is the trap in the other direction: a row that top-aligns wrappers differing by 88px in height puts the pivots 44px apart and the label stacks a full 88px apart, and a matched row of three dials at three different heights reads as a build error.

Dials 2 and 3 are therefore drawn in a **144 × 232 registration box** with their 144 × 144 ring inset **44px from the top** — 144 wide for the width budget, 232 tall so the pivot and the label stack land on dial 1's. Ring geometry inside is unchanged (C = 72).

```
all three dials   pivot on one Y     label stack top on one Y
                  body centre 706    stack top 810        (as built, from the panel's top edge)
```

Uneven *horizontal* pitch between the pivots is correct and expected — the dials genuinely differ in how much printed scale they carry. Uneven vertical registration is not.

*Inner ring — percent* (Wow 0–100 % in Tape, Repeat Degrade 0–100 % in Digital), linear: identical angles to the dials 2/3 table above.

*Outer ring — hertz* (Mod Rate 0.1–5.0 Hz, **skew 0.4090339496**):

| Value | Printed | Angle | Type |
|---|---|---|---|
| 0.1 Hz | `0.1` | −135.00° | major |
| 0.25 Hz | — | −70.13° | minor |
| 0.5 Hz | `0.5` | −38.11° | major |
| 1.0 Hz | `1` | 0.00° | major |
| 1.5 Hz | — | +26.74° | minor |
| 2.0 Hz | `2` | +48.26° | major |
| 3.0 Hz | `3` | +82.86° | major |
| 4.0 Hz | — | +110.93° | minor |
| 5.0 Hz | `5` | +135.00° | major |

**Lit / dim per mode** — same treatment and the same colours as the stacked labels:

| Mode | Inner (percent) | Outer (Hz) |
|---|---|---|
| TAPE | lit | dim |
| BBD | dim | **lit** |
| DIGITAL | lit | dim |

| | Ticks | Minors | Numerals |
|---|---|---|---|
| lit | `#a8a294` | `#8a857a` | `#a8a294` (7.36:1) |
| dim | `#5a564e` | `#4a463f` | `#615c54` (2.82:1) |

The dim numerals are a **third instance of the documented exception in §8.3**, not a new one: printed legend for a mode that is not live, with an LED and a lit companion ring carrying the current state. Per §2.2 the knob body, its pointer and both rings' geometry are unchanged between modes — only the two rings' ink changes.

---

## 5. Scope — Repeat Timeline

### 5.1 Two rectangles, specified separately

```
All figures are BORDER-BOX unless marked content — the 1px border is INCLUDED
in the width and height given. A build reading these as content boxes and then
adding a border lands 2px oversize.

display rect   1092 × 98 px   at fascia-local (22, 157)   ← border-box
               content box 1090 × 96 inside a 1px border
               fill #04060a , 1px border #23221e , radius 2px
               inset 0 +3px 14px rgba(0,0,0,0.9)

readout strip  1090 × 23 px   top of the display rect's CONTENT box, full width
               22px of content + a 1px bottom rule rgba(150,175,155,0.13)

plot region    1090 × 74 px   the display rect's content box below the strip
               left edge   display left + 1
               top edge    display top + 1 + 23  =  display top + 24
               bottom edge display bottom − 1
```

**The trace is clamped to the plot region and must never enter the readout strip.** Within the plot region:

```
baseline       y = plotHeight − 9        (envelope feet sit here)
headroom       peak height = plotHeight − 17, i.e. baseline − 8 max
grid           7 vertical lines at plotWidth/8 intervals, rgba(150,175,155,0.10), 1px
centre line    horizontal at plotHeight/2, same colour
baseline rule  rgba(160,180,165,0.22), 1px, full plot width
```

Clamping to the plot region rather than the display rect is what keeps a tall envelope off the `RPT ENV` and `0 dB` labels at the strip's two ends.

### 5.2 Trace

Time runs right-to-left over a **2600 ms** window; a pulse enters at the right edge and exits at the left. Pulses spawn one delay period apart. Amplitude of repeat *n* is `aₙ = aₙ₋₁ · feedback · lossPerRepeat`, reset to 1.0 when it falls below 0.03.

```
lossPerRepeat   TAPE      1 − generationLoss/260
                BBD       1 − modDepth/420
                DIGITAL   1 − degrade/300

height(a)       plotSpan · clamp(a^0.45, 0.04, 1.0)
```

Each pulse draws as a ghost bar behind (`rgba(190,200,190,0.07)`, width max(8, plotWidth·0.014)) and a stem in front: accent `#ff9d3c`, 3.0px for the dry hit and 2.2px for repeats, with a filled cap circle of r 3.2 / 2.2 and a glow of `max(4, 16 − age/260·6)` px. In PING-PONG a dashed `rgba(200,210,200,0.16)` rule (3 on, 4 off) marks the half-amplitude line.

### 5.3 Readout strip contents

Share Tech Mono **11px**, tracking .12em, vertically centred, 9px side padding, 16px between fields, `white-space: nowrap` with overflow hidden. All of it is LCD segment face — the same face as the PROGRAM LCD, one size down.

Left to right:

| Field | Colour | Example |
|---|---|---|
| `RPT ENV` — strip identifier | `#93a894` | `RPT ENV` |
| mode descriptor · stereo mode · feedback · division rate | `#a9bda9` | `BBD BUCKET-BRIGADE · PING-PONG · FB 62% · 375 ms / DIV` |
| *(flexible gap)* | | |
| note value · BPM · time | `#93a894` | `1/8. · 120 BPM · 375 ms` |
| reference | `#93a894` | `0 dB` |

With Sync off the fourth field reads `FREE · 375 ms`.

The strip carries **whole-effect state only**. `FB 62%` is the single parameter value it holds, because the envelope drawn beneath it is a direct plot of that value. Everything else per-control belongs in the PROGRAM LCD.

---

## 6. PROGRAM LCD

### 6.1 Geometry

**The header band is 34px, and all five elements measure it.** PROGRAM LCD, SAVE, DELETE, IN and OUT are each **34px border-box** (32px content + 1px border top and bottom) and share one bottom edge at **y = 78.23** — verified on the render, not approximated. The IN/OUT captions use the PROGRAM caption's construction (`margin-bottom: 6px`, no flex gap) so the two column types resolve to the identical baseline; a 5px gap put the meters 0.4px high.

Widths stay per-casting: LCD 449, SAVE 68, DELETE 76, IN 76, OUT 76 (border-box).

```
All figures are BORDER-BOX unless marked content.

outer          449 × 34 px , radius 3px   ← border-box
               content box 447 × 32 inside a 1px border
fill           linear-gradient 180°  #071009 → #040806
border         1px #2a2823
recess         inset 0 +2px 10px rgba(0,0,0,0.9) , 0 +1px 0 rgba(255,255,255,0.05)

bank cell      67.2 px wide (content-sized: 8px padding, text, 8px padding)
               right border 1px rgba(255,255,255,0.09)
name cell      351.8 px wide  ← the character budget below is against this
chevron cell   28.0 px wide  = 7px padding + 14px glyph + 7px padding
```

### 6.2 Type and character budget

Bank tag and program name are **identical**: Share Tech Mono **19px**, tracking .12em, `#cfd8cb`, glow `0 0 9px rgba(180,210,185,0.35)`. At that size and tracking the advance is **12.54 px/char**, so the 335.8px name cell holds **26 characters**.

| String | Chars | Width at 19px | Fits |
|---|---|---|---|
| Longest factory name, `06 BUS TIRE DUB` | 15 | 188.1 px | yes |
| `FEEDBACK: 62 %` | 14 | 175.6 px | yes |
| `MOD RATE: 0.05 Hz` | 17 | 213.2 px | yes |
| `OUTPUT TRIM: +12.0 dB` | 21 | 263.3 px | yes |
| `GENERATION LOSS: 100 %` | 22 | 275.9 px | yes |

**Everything fits at 19px.** The longest live readout, `GENERATION LOSS: 100 %` at 22 characters, clears the cell by 75.9px.

**Budget 28 characters. User-name cap 26 — held, not shrunk.**

```
name cell 351.8 px  ÷  12.54 px/char  =  28 characters   ← the budget
                    less the 2-char dirty marker " *"    =  26   ← the cap
```

User Programs carry no index (§3), so nothing else is deducted; Factory labels spend 3 of the 28 on `NN ` and are authored, not typed.

The trailing ` *` marker was not previously drawn, and adding it to a 26-char budget would have cut the cap to 24 — **orphaning every already-saved name between 25 and 26 characters.** The two characters were taken out of padding instead, exactly as the brand rule prescribes: the bank cell went 12px → 8px per side and the chevron cell 11px → 7px, +16px in total, which lifted the budget 26 → 28 and left the cap where it was. Type, tracking, size and LCD width are all unchanged. The naming input follows the new cells at `left: 60px; right: 30px`.

**Any future change to this row must state the resulting budget and confirm the cap has not fallen.**

This required narrowing the wordmark column — see §7.1. At the previous 326px column the name cell was 278px and the two longest readouts overflowed, which would have forced a size step-down on every live readout; widening the LCD removed the need for one.

A guard remains for safety: **any LCD string over 28 characters renders at 16px** instead of 19px, same face, tracking, colour and glow (advance 10.56 px/char, so 31 characters fit). It does not fire for any authored string, and the step is instantaneous — no animation between the two sizes.

### 6.3 Live values vs. the scope strip

While a control is being moved the LCD shows `NAME: value` — `FEEDBACK: 62 %`, `MOD RATE: 0.45 Hz`, `TIME: 375 ms`, `OUTPUT TRIM: +2.5 dB` — reverting to the program name **900 ms** after release. **There is no tooltip.** Only direct user manipulation triggers this; host automation must never drive it.

Rule for the build: if a string answers *what is this delay doing*, it goes in the scope strip; if it answers *what is this knob set to*, it goes in the LCD and only while the knob is moving.

### 6.4 Chevron — drawn path

Not a typographic character, so it renders identically across platforms and font fallbacks.

```
viewBox   0 0 14 8
path      M1 1.6 L7 6.4 L13 1.6
stroke    #a9bda9        9.69:1 on the LCD substrate
width     1.6
linecap   round
linejoin  round
fill      none
```

Rendered 14 × 8 px, vertically centred in the 32px content height, 11px padding each side. In JUCE build it as a `Path` with `PathStrokeType (1.6f, curved, rounded)` scaled to the 14 × 8 box; do not bake it into the plate. The chevron keeps this dimmer green — only the bank tag was promoted to the program-name treatment.

### 6.5 Dropdown

Opens flush under the bar at `top: 34px` (the LCD's border-box height, unchanged), full LCD border-box width, max height 210px, scrolling. Fill `#060d09`, 1px border `#2c2b26`, shadow `0 18px 32px rgba(0,0,0,0.75)`. Rows: 8px/14px padding, 1px `rgba(255,255,255,0.05)` separator, Share Tech Mono 15px `#93a894`, selected row `#d7e2d6`, trailing bank tag 12px `#93a894`.

---

## 7. Tape elements

Four, and only four. Hierarchy largest to scrappiest: nameplate (27px) → CH 4 scribble strip (15px) → HALDEN HALL (14px) → DLY 4 (12px).

### 7.1 FIFTH MEMBER nameplate

| | |
|---|---|
| Element | 268px column, height from content (8px top / 10px bottom padding, 17px sides) |
| Placement | header, top-left of the fascia |
| Rotation | **−1.2°** |
| Face | `linear-gradient(180deg, #d9d4c6, #bdb7a7 60%, #a8a293)` |
| Weave | `repeating-linear-gradient(90deg, rgba(0,0,0,.055) 0 1px, rgba(255,255,255,.05) 1px 4px)` |
| Soiling | `radial-gradient(60% 100% at 80% 20%, rgba(90,80,66,.30), transparent 70%)` |
| Torn ends | `clip-path: polygon(1% 6%, 99% 0%, 100% 92%, 60% 100%, 12% 96%, 0% 88%)` |
| Shadow | `0 3px 7px rgba(0,0,0,.55)`, inset `0 1px 0 rgba(255,255,255,.5)` |
| Lettering | Permanent Marker **27px**, `#151310`, line-height .98, tracking .005em, highlight `0 1px 0 rgba(255,255,255,.25)` |

The column was 326px at 33px lettering in earlier revisions; it was narrowed to 268px to buy the PROGRAM LCD the width it needed to hold a full live readout at a single type size (§6.2). The nameplate remains the largest and most deliberate piece of tape on the unit.

**The nameplate lettering is deliberately loose** — hand-drawn, uneven, not on a baseline grid. It is the one element in this document that should be traced from the reference render rather than reconstructed from metrics. The other three should be reproducible from their tables.

### 7.2 DLY 4 — cable tape (unchanged)

| | |
|---|---|
| Element | 44 × ~26 px (6px vertical padding), right ear |
| Placement | `left: 4px`, `top: 150px` |
| Rotation | **+1.4°** |
| Face | `linear-gradient(180deg, #e8e3d3, #cdc7b6)` |
| Torn ends | none — cut square, as a short cable label |
| Shadow | `0 2px 4px rgba(0,0,0,.45)` |
| Lettering | Permanent Marker **12px**, `#20201c`, centred |

### 7.3 HALDEN HALL · LOAD-IN 06 — venue tape

| | |
|---|---|
| Element | 36 × 252 px, right ear |
| Placement | `left: 8px`, `bottom: 48px` within the 52px ear |
| Rotation | **+1.6°** |
| Face | `linear-gradient(96deg, #efeadc, #ded8c8 55%, #cdc7b6)` |
| Weave | `repeating-linear-gradient(4deg, rgba(0,0,0,.05) 0 1px, rgba(255,255,255,.05) 1px 5px)` |
| Torn ends | `clip-path: polygon(0% 2%, 34% 0%, 66% 2.4%, 100% 0.6%, 100% 97%, 62% 100%, 30% 97.6%, 0% 99%)` — short ends only; long edges cut straight, as tape off a roll |
| Shadow | `0 2px 5px rgba(0,0,0,.5)`, inset `0 1px 0 rgba(255,255,255,.55)` |
| Lettering | Permanent Marker **14px**, `#23211c`, tracking .02em, rotated −90°, centred along a 244px run, `nowrap` |

### 7.4 CH 4 — GTR / STAGE LEFT — scribble strip

| | |
|---|---|
| Element | 236 × 34 px, foot row, positioned **after** `DL-88 · TOURING SPEC · 5U` |
| Rotation | **−1.1°** |
| Face | `linear-gradient(178deg, #f0ebdd, #e0dac9 58%, #cec8b7)` |
| Weave | `repeating-linear-gradient(88deg, rgba(0,0,0,.05) 0 1px, rgba(255,255,255,.05) 1px 5px)` |
| Torn ends | `clip-path: polygon(1.6% 0%, 0.4% 34%, 2.2% 68%, 0% 100%, 98.6% 100%, 100% 64%, 98% 32%, 99.4% 0%)` — left and right ends only |
| Shadow | `0 2px 6px rgba(0,0,0,.5)`, inset `0 1px 0 rgba(255,255,255,.55)` |
| Lettering | Permanent Marker **15px**, `#23211c`, tracking .01em, centred |

Tape lettering measures **11.3:1** on the tape face in all four cases.

### 7.5 RACK 4 · MON WORLD — stencil, not tape

Sprayed ink on brushed aluminium, rotated −90°, centred on the left ear. Barlow Condensed 600 **11px**, tracking .34em, ink `rgba(24,22,19,0.92)`, which composites to `#211f1c` on the `#8e8a82` ear mid tone: **4.78:1**. Flavour text — clears the 3:1 flavour floor with margin without becoming a hard black that would read as printed rather than sprayed.

---

## 8. Palette and typography

### 8.1 Substrates

| Substrate | Hex | Rel. luminance |
|---|---|---|
| Fascia (mid of #1b1a18 → #131211 → #0e0e0c) | `#131211` | 0.00612 |
| Button face (mid of #2b2924 → #171613) | `#211f1b` | 0.01383 |
| PROGRAM LCD | `#071009` | 0.00436 |
| Scope glass | `#04060a` | 0.00178 |
| Dropdown | `#060d09` | 0.00347 |
| Meter glass | `#05080a` | 0.00228 |
| Left rack ear (mid tone) | `#8e8a82` | 0.25540 |
| Tape face | `#ded8c8` | 0.68300 |

### 8.2 Every text role

| Role | Hex | Size / weight | Substrate | Ratio |
|---|---|---|---|---|
| TEMPO-SYNCED DELAY | `#a8a294` | BC 600 13px / .30em | fascia | 7.36 |
| MODEL DL-88 · STEREO | `#a8a294` | BC 500 12px / .30em | fascia | 7.36 |
| PROGRAM caption | `#a8a294` | BC 600 11px / .28em | fascia | 7.36 |
| Section labels (TIMING, REPEATS, OUTPUT, DELAY CHARACTER) | `#a8a294` | BC 600 11px / .28em | fascia | 7.36 |
| Sub-headings (NOTE DIVISION, STEREO MODE, CHARACTER · ALL MODES, REPEAT TIMELINE) | `#a8a294` | BC 600 11px / .24em | fascia | 7.36 |
| SYNC ON / OFF | `#a8a294` | BC 600 12px / .22em | fascia | 7.36 |
| IN / OUT captions | `#a8a294` | BC 600 11px / .26em | fascia | 7.36 |
| Foot strip (DL-88 · TOURING SPEC · 5U, SN 0417 · v1.0) | `#a8a294` | BC 600 11px / .26em | fascia | 7.36 |
| Printed scale numerals | `#a8a294` | BC 600 10px / .06em | fascia | 7.36 |
| Knob names | `#c3bcae` | BC 600 11px / .20em | fascia | 9.91 |
| REPEATS LIVE | `#c3bcae` | BC 600 12px / .26em | fascia | 9.91 |
| Division button label, selected | `#f0eade` | STM 12px | button | 13.73 |
| Division button label, unselected | `#b0aa9c` | STM 12px | button | 7.11 |
| Stereo / character button label, selected | `#f0eade` | BC 600 12px / .18em (character: 13px / .24em) | button | 13.73 |
| Stereo / character button label, unselected | `#b0aa9c` | same | button | 7.11 |
| Program legend, lit (SAVE / STORE / DELETE / CANCEL) | `#f2ece0` | BC 600 10px / .14em | button | 13.98 |
| Program legend, dark, on SAVE face | `#77736a` | same | button | 3.48 ✱ |
| Program legend, dark, on DELETE face | `#77736a` | same | button | 3.66 ✱ |
| Bank tag (FACT / USER / NAME) | `#cfd8cb` | STM 19px / .12em | LCD | 13.19 |
| Program name / live readout | `#cfd8cb` | STM 19px (16px guard over 26 chars) | LCD | 13.19 |
| Chevron stroke | `#a9bda9` | 1.6px path | LCD | 9.69 |
| Rename field | `#d7e2d6` | STM 17px / .10em | `#0a1410` | 12.6 |
| Dropdown row / bank tag | `#93a894` | STM 15px / 12px | dropdown | 7.73 |
| Dropdown row, selected | `#d7e2d6` | STM 15px | dropdown | 12.9 |
| Scope readout, base | `#93a894` | STM 11px / .12em | scope | 7.99 |
| Scope readout, highlighted | `#a9bda9` | STM 11px / .12em | scope | 10.18 |
| Meter values | `#b9c3c8` | STM 16px | meter | 11.19 |
| Multi-label stack, lit | `#e7e1d4` | BC 600 11px / .18em | fascia | 14.36 |
| Multi-label stack, unlit | `#615c54` | BC 600 11px / .18em | fascia | 2.82 ✱ |
| Tape lettering, all four | `#23211c` | Permanent Marker 27 / 15 / 14 / 12px | tape | 11.3 |
| RACK 4 stencil | `rgba(24,22,19,.92)` | BC 600 11px / .34em | ear | 4.78 |

BC = Barlow Condensed, STM = Share Tech Mono.

### 8.3 ✱ The two intentional exceptions

Both stay dimmer **by design**, because an LED carries the state and the brand rule forbids conveying relevance by dimming the control itself:

- **Unlit multi-label stack entries** (2.82:1). The lit entry and its lamp identify what the dial is driving; the unlit rows are printed legend for the other two modes, not live labels.
- **Unselected mode button labels** (7.11:1 — above the 7:1 floor, but deliberately below the 13.73:1 of the selected label). The pairing of a pressed face, a lit lamp and a brighter label is what marks selection.

**Dimmed Dial 1 scale numerals** (2.82:1) are the same exception in a third place — see §4.5. Printed legend for a mode that is not live; the lit ring and the stack LED carry the current state.

The dark Program legends (3.48:1 and 3.66:1) are the fourth case, and the clearest one: a legend whose lamp is out is *state*, exempt from the functional floor, and it clears the brand's 3:1 state floor on both faces with margin. There is no longer a disabled DELETE face — the lamp goes out instead, which is what §1.3.1 replaced it with. It was raised from 1.99:1 regardless, since at that value it read as absent rather than disabled.

### 8.4 Accent

`#ff9d3c`, exactly one per plugin per BRAND.md. Used **only** for the REPEATS LIVE lamp (13px, pulsing 1.6s ease-in-out between 1.0 and 0.72 opacity, glow `0 0 10px #ff9d3c, 0 0 22px rgba(255,157,60,.45)`) and the scope pulses. Nowhere else.

---

## 9. Bypass

**Fifth Member has no bypass treatment today** — no bypass parameter, no disengaged state, nothing on the panel. This is a gap, not a decision; I have not invented one.

**Do not print the word BYPASS anywhere on the panel.** The foot strip previously carried `BYPASS · v1.0` as static text, borrowed from Chorus-60, where the footer is a live readout that reads `ENGAGED` and flips to `BYPASS` when the engines are off. With no state behind it here, it permanently announced that a unit passing audio was bypassed. It now reads `SN 0417 · v1.0` — a static serial number. If a bypass is added later, it gets the lighting treatment below, not a caption.

When it is added it must follow BRAND.md exactly, and that rule is short enough to restate:

- **A lighting change only.** No blur, no defocus, no desaturation, no flattened or redrawn controls, nothing implying the hardware changed.
- **Pointers stay exactly where they are.** A real panel's knobs don't move when a lamp goes out.
- **Applied as a multiply over the panel**, not an alpha blend toward the background. Multiply preserves relative contrast and reads as darkness; blending toward the panel colour reads as fog laid over it.
- **Pitched dark enough to read as off, not merely dimmed.** Chorus-60 lands at **0.50**; 0.70 read as a dimmer switch rather than a light going out. Match 0.50 unless testing on this fascia says otherwise.
- **No desaturation** — where colour distinguishes a control from its neighbours, draining it removes information rather than signalling state.
- **The legibility floor does not apply.** The job is to convey *not usable*, not to stay readable.
- **No caption.** Nothing printed to explain the state.

The scope trace should freeze rather than continue running, and the REPEATS LIVE lamp goes out — both follow from the panel being unlit, and neither is an exception to the multiply.

---

## 10. Unchanged — keep as-is

- **The product icon.** Concept 1b, the phosphor repeat train. Delivered at `icons/fifth-member-icon-1024.png` (Projucer Large Icon) and `icons/fifth-member-icon-256.png` (Small Icon), PNG with alpha, transparent rounded-square corners so JUCE/macOS masking will not double-round it. Source of truth `IconPulse.dc.html` at a 256px design size; re-render at any multiple from there. No text in the mark; the decay train is vertically centred and reads at every size with no simplification variant.
- **Whole visual identity.** Road-worn black rack chassis, fascia gradient, corner wear to bare metal, scuff streaks, brushed aluminium ears with screws, the gaffer-tape nameplate, the DLY 4 cable tape, the RACK 4 · MON WORLD stencil.
- **Panel architecture.** Header composition, the three fixed Delay Character dials with permanently printed stacked labels, the Repeat Timeline scope, section grouping, all control positions and sizes.
- **Program management.** SAVE always creates a new User Program. The two buttons carry stacked SAVE·STORE and DELETE·CANCEL legends, backlit individually, with no disabled face (§1.3.1); the bank tag reads FACT / USER / NAME.
- **Interaction behaviour** apart from the removal of the tooltip: vertical drag, 190px of travel for full range, 620ms eased pointer motion on program recall only.
- **Canvas** 1240px design width, proportional scaling.
- **Accent** `#ff9d3c`.

---

## 11. Outstanding before the plate is cut

1. ~~The plate.~~ **Cut and delivered** at 1×/2×/3× in `plate/`, without Dial 1's two rings per the §1 carve-out.
2. ~~Read-only parameter inventory from the build.~~ **Received and applied.** TIME, MOD RATE and DAMPING are all skewed; DAMPING is a frequency, not a percentage; every printed scale in §4 has been recut against the build's angles and the normalised 0–10 legend retired. No open question remains here.
3. **A bypass decision** per §9. Still the one substantive gap.
4. **By-ear tuning of the six factory Programs.** The names are authored; the values are not final.
5. **Confirm the printed subsets.** §4.4's TIME ring labels 6 of its 13 marks and §4.5's Hz ring labels 6 of 9 — chosen for legibility at those diameters. Every mark sits at the build's angle either way, so promoting a minor to a labelled major is a free change if the build wants more of them.
