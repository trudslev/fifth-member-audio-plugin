# FIFTH MEMBER — GUI SPEC

Model **DL-88**, tempo-synced stereo delay. Neon Foundry casting, harmonisation round.
Authoritative for the build; supersedes the conformance-pass spec and `BUILD-HANDOFF.md`
wherever the three disagree.

**Read `shared/HEADER-PART.md` first.** The block, the band, the LCD cell with its budget and
cap, the Program buttons and their state matrix, and the meter wells are the shared part and are
not restated except where this casting's material meets them.

**Asset format: one exported plate, everything else drawn at runtime.** The plate carries the
chassis, fascia, wear, all four tape elements with their lettering, the rack ears and screws, the
rack-ear stencil, section frames, printed labels, ticks and scale numerals, and the empty wells.
**Two carve-outs are never in the plate** because they change ink with the mode: dial 1's two
scale rings, and all three Delay Character label stacks. Nothing carrying a live value is baked.

---

## 1 · Canvas — and the one casting that is wider than the frame

| Figure | Value |
|---|---|
| **Window** | **1444 × 1012** |
| **Frame** | **1340** wide — the shared coordinate frame, at x 52 |
| Rack ears | 52 px each side, full height, **outside the frame** |
| Header block | frame-local 16, 16, 1308 × 104 |
| Fascia | `linear-gradient(#1b1a18, #131211 58%, #0e0e0c)`, frame padding 16 |

**Call 1 is stated as the panel frame, not the window**, and Fifth Member is why: it is the only
casting with full-height rack ears flanking the fascia, and they carry three of its five identity
marks — the RACK 4 stencil, the DLY 4 cable tape and the HALDEN HALL tape. A 1308-wide block at
x 16 inside a 1340 window leaves **zero** for ears.

The alternative was 16 px bezels, which would have removed those three marks to protect a window
dimension — inverting the round's scope, where the header is the part and body layout is the
casting's. **Named cost: this window is 104 px wider than the other five.** Right at 1444 against
1340; it would not have been at 960.

Ears are chassis: `linear-gradient(90deg, #6f6b64, #b9b5ac 22%, #9d998f 52%, #7e7a73 78%, #63605a)`
with a 1 px brush repeat, two Ø22 screws each, and the stencil rotated −90° on the left.

### Layout order

Header · REPEATS LIVE + REPEAT TIMELINE caption row · scope · the three control bands
(TIMING 320 · REPEATS flex · OUTPUT 340) · DELAY CHARACTER · foot row. **The scope sits above
the control bands**, which is the built arrangement and not the one the old spec implied.

---

## 2 · Knobs — two classes from five diameters

| Class | Ø | Controls | Wrapper |
|---|---|---|---|
| Primary | **76** | TIME, FEEDBACK, MIX, character dials 1–3 | 144, C 72 (dial 1: **232**, C 116) |
| Standard | **56** | CROSS-FEED, OUTPUT TRIM, DAMPING · Hz, SATURATION | 124, C 62 |

**Fifth Member takes no Ø104**: it has no MODEL control — mode selection is the three character
buttons — so the signature diameter would land on nothing that earned it. Five diameters
(84 · 82 · 76 · 66 · 62) map onto two.

**TIME is promoted 66 → Ø76**, and it is the round's one deliberate class promotion. At Ø56 it
would carry three numerals, and TIME is the primary control on a delay with a skewed 1–2000 ms
ring; at Ø76 it keeps five and the rest demote to minors at the build's angles. Primary means
*a section's lead control*, and TIME is TIMING's.

**Tick and numeral law, this casting's own and unchanged:** major **2 × 9** spanning R+8 → R+17,
minor **1.5 × 5** from R+12, both ending on a common tip at R+17. Numerals Barlow Condensed 600
10 px, anchored **by the box edge facing the dial** —
`r = R + 21 + halfWidth·|sin θ| + 5·|cos θ|` — so a one-character and a three-character label
clear the tip by the same 4 px. A fixed numeral radius leaves wide labels visibly tighter to the
arc than narrow ones.

Bodies: primary `radial-gradient(circle at 38% 28%, #514e46, #282621 55%, #121110)`, pointer
2.5 × 30 `linear-gradient(#f5f0e5, #b7b0a1)` — **13.30:1**; standard
`radial-gradient(circle at 38% 28%, #4c4942, #262420 55%, #121110)`, pointer 2 × 24 — **13.51:1**.
**The specular is the gradient's off-centre origin and is fixed to the panel, not the knob** — it
must not rotate.

### 2.1 Registration — mixed rows, and both suite rules hold at once

REPEATS (FEEDBACK Ø76 + CROSS-FEED Ø56) and OUTPUT (MIX Ø76 + OUTPUT TRIM Ø56) are mixed rows.
Each Ø56 knob sits in a **144-tall registration box with its ring inset `dy = (76 − 56) / 2 = 10`**.
The label registers on the box, so labels land on one line; the ring registers on itself, so
pivots land on one Y. **Both hold** — the apparent trade between them only exists if the label is
positioned off the ring wrapper.

The character row is the same construction at **dy 44**: dials 2 and 3 are drawn in 144 × 232
boxes with their rings inset 44, so all three pivots and all three label stacks share a line
against dial 1's 232 wrapper. **Unequal wrappers register on the pivot, not on their boxes** — a
row top-aligning wrappers that differ by 88 px puts the pivots 44 px apart, and a matched row of
three dials at three different heights reads as a build error.

### 2.2 Mark lists

Angles are the build's and **must never be derived by even spacing** where a skew is stated.
Numerals **bold**.

| Knob | Range · taper | Marks |
|---|---|---|
| TIME | 1–2000 ms, **skew 0.4135361469** | **1** (−135) · −114.33 · −106.10 · −76.75 · **100** (−57.08) · −31 · **375** (0) · +17.10 · **1K** (+67.67) · +104.70 · **2K** (+135) |
| FEEDBACK | 0–110 %, linear | **0 / 25 / 50 / 75 / 100** at −135 / −73.64 / −12.27 / +49.09 / +110.45, plus an unlabelled minor at +135 (the 110 % end stop) |
| MIX · dials 2–3 · dial 1 inner | 0–100 %, linear | **0 / 25 / 50 / 75 / 100** at −135 / −67.5 / 0 / +67.5 / +135 |
| dial 1 outer | 0.1–5.0 Hz, **skew 0.4090339496** | **0.1** (−135) · −70.13 · **0.5** (−38.11) · **1** (0) · +26.74 · **2** (+48.26) · +82.86 · +110.93 · **5** (+135) |
| CROSS-FEED · SATURATION | 0–100 %, linear | **0 / 50 / 100**, minors at 25 and 75 |
| DAMPING · Hz | 1000–16000 Hz, **skew 0.4306765581** | **1K** (−135) · −50.89 · −21.63 · **4K** (0) · +33.22 · +59.45 · +101.24 · **16K** (+135) |
| OUTPUT TRIM | −24…+24 dB, linear | **−24 / 0 / +24**, minors at ±18, ±12, ±6 — **leading plus kept** on +24 |

**TIME's 10 ms mark keeps its angle and loses its numeral.** The first cut of this ring printed
six, one over the primary ceiling; the demotion drops the numeral, not the mark.

**DAMPING is a cutoff frequency, not a percentage** — earlier revisions described it as 0–100 %,
which was wrong in unit rather than in spacing. The panel label reads `DAMPING · Hz`.

### 2.3 Dial 1 — two concentric rings, and the five-numeral cap is per ring

Dial 1 binds to a percentage in TAPE and DIGITAL and a frequency in BBD. These cannot share
numerals: 1 Hz sits at 0° where the percentage ring prints `50`.

```
inner ring   percent   ticks R+8  → R+17   numerals inner edge at R+21
outer ring   hertz     ticks R+44 → R+53   numerals inner edge at R+57
```

**The outer band is placed against the inner numerals' OUTER extent, not their inner edge.**
Numerals are anchored by the edge facing the dial, so their inner edge at R+21 says nothing about
how far the glyphs reach: at R 38 the widest inner label reaches radius **77.3**, so the outer
ring cannot begin before ~R+43. It begins at **R+44 (radius 81)**, clearing by 4.7 px. An earlier
revision put it at R+30 and drove a 7.7 × 6.9 px collision.

**Ten numerals on one knob is correct.** The five-numeral ceiling is **per ring**, not per knob,
and that follows from its derivation: the figure came from five numeral boxes over 270° falling
inside 16 px of arc *at Ø56's numeral radius*, so it measures circumference. A ring at a larger
radius on the same body has the circumference the constraint was measuring. **The outer Hz ring
was cut from six numerals to five** in this round; dial 1 is the only two-ring knob in the suite.

**Lit / dim per mode** — same treatment and colours as the stacked labels:

| Mode | Inner (percent) | Outer (Hz) |
|---|---|---|
| TAPE · DIGITAL | **lit** | dim |
| BBD | dim | **lit** |

Lit ticks `#a8a294`, minors `#8a857a`, numerals `#a8a294`; dim `#5a564e`, `#4a463f`, `#615c54`.
**Only the ink changes** — geometry, body and pointer are identical between modes.

---

## 3 · The three Delay Character pages

Selecting a mode changes which stack labels are lit, which of dial 1's rings is lit, and the
scope's mode descriptor. **It changes nothing else** — no knob is added, removed, repositioned or
dimmed.

| | **TAPE** | **BBD** | **DIGITAL** |
|---|---|---|---|
| Stack 1 lit | WOW | MOD RATE | REPEAT DEGRADE |
| Stack 2 lit | FLUTTER | MOD DEPTH | *none* |
| Stack 3 lit | GENERATION LOSS | *none* | *none* |
| Dial 1 drives | Wow 0–100 % | Mod Rate 0.1–5.0 Hz, skew | Repeat Degrade 0–100 % |
| Scope descriptor | `TAPE ECHO · 3-HEAD` | `BBD BUCKET-BRIGADE` | `DIGITAL · 16-BIT` |

Where a stack has no lit row, **every row in it renders unlit and the dial still draws at full
strength**. Dial 2 in DIGITAL and dial 3 in BBD/DIGITAL are the cases. **A build that dims an
inactive dial is wrong**: the knob body, pointer, ring and numerals are identical whether the
parameter that dial currently drives is engaged or not. Only the LED changes.

Stacks: 6 px LED, 7 px gap, label Barlow Condensed 600 **11 / 14 / .18 em**, 5 px between rows,
left-aligned, lit `#e7e1d4` / unlit `#615c54`. Stack 3's single row occupies row 1 and the column
does not re-centre.

---

## 4 · Scope — Repeat Timeline

| Rectangle | Figure |
|---|---|
| Display rect | **1308 × 98** border-box, `#04060a`, `inset 0 0 0 1px #23221e`, `inset 0 3px 14px rgba(0,0,0,.9)` |
| Readout strip | 23 px at the top of the content box, 1 px bottom rule `rgba(150,175,155,.13)` |
| Plot region | the content box below the strip, **1306 × 73** |

**The trace is clamped to the plot region and must never enter the readout strip.** Baseline
`plotHeight − 9`, peak ceiling `plotHeight − 17`, grid 7 verticals at eighths plus a centre line,
baseline rule `rgba(160,180,165,.22)`.

Time runs **right to left over 2600 ms**; pulses spawn one delay period apart, amplitude
`aₙ = aₙ₋₁ · feedback · lossPerRepeat`, reset when it falls below 0.03, height
`plotSpan · clamp(a^0.45, 0.04, 1.0)`. Loss per repeat: TAPE `1 − genLoss/260`, BBD
`1 − modDepth/420`, DIGITAL `1 − degrade/300`. Each pulse is a ghost bar behind
(`rgba(190,200,190,.07)`) and an accent stem in front — 3 px for the dry hit, 2.2 for repeats,
with a filled cap. In PING-PONG a dashed `rgba(200,210,200,.16)` rule marks the half-amplitude
line.

**Readout strip carries whole-effect state only**, Share Tech Mono 11 / 13 / .12 em:
`RPT ENV` · mode descriptor · stereo mode · `FB <n>%` · `<n> ms / DIV` · note value · BPM · time
· `0 dB`. **`FB 62%` is the one parameter value it holds**, because the envelope drawn beneath it
is a direct plot of it. **If a string answers *what is this delay doing*, it belongs here; if it
answers *what is this knob set to*, it belongs in the LCD and only while the knob is moving.**

---

## 5 · Switches, LEDs and tape

**Sync switch** — track 52 × 24 `#0b0a09`, thumb 23 px `linear-gradient(#cfc9bb, #8d887d)`,
travel `left: 2` → `left: 27`, 180 ms. Caption `SYNC ON` / `SYNC OFF` is **state-dependent text
and therefore not baked**.

**Buttons** — one construction: radius 3, face `linear-gradient(#2b2924, #171613)`, raised
`inset 0 1px 0 rgba(255,255,255,.13)` + `0 3px 6px rgba(0,0,0,.5)`, pressed
`inset 0 2px 6px rgba(0,0,0,.75)`. Selected = pressed + lit LED + `#f0eade` label; unselected =
raised + unlit LED + `#b0aa9c`. Division ×5 at 30 px, stereo ×3 at 32 px, character ×3 at 54 px.

**LEDs** — one lamp at three diameters: 6 px in stacks and division buttons, 7 px on stereo
buttons and the TIME / CROSS-FEED / NOTE DIVISION captions, 10 px on character buttons. Lit
`radial-gradient(circle at 35% 30%, #ffffff, #efe9d6 45%, #b9b09a)` with a two-stop glow; unlit
`#3a3833 → #191816` with an inset shadow and no glow. **The LED is not the accent.**

| LED | Lit when |
|---|---|
| NOTE DIVISION caption | Sync ON |
| Division buttons | Sync ON **and** that division selected |
| TIME caption | Sync **OFF** |
| CROSS-FEED caption | Stereo mode = PING-PONG |
| Stereo · character buttons | that mode selected |

**No other control has an LED** — FEEDBACK, MIX, OUTPUT TRIM, DAMPING and SATURATION are
unconditional and carry a name only.

**Four tape elements, and only four**, largest to scrappiest: the FIFTH MEMBER nameplate
(Permanent Marker 27 px, 268 px column, −1.2°) · the CH 4 — GTR / STAGE LEFT scribble strip
(15 px, 236 × 34, −1.1°) · HALDEN HALL · LOAD-IN 06 on the right ear (14 px, 36 × 252, +1.6°,
rotated −90°) · DLY 4 cable tape (12 px, 44 px, +1.4°). Permanent identification is stencilled —
`RACK 4 · MON WORLD`, Barlow Condensed 600 11 / .34 em at `rgba(24,22,19,.92)` on the left ear —
**per-show information is written on tape**. All four are torn-ended with a weave overlay, and
all are baked.

**The nameplate lettering is deliberately loose** — hand-drawn, uneven, off any baseline grid.
It is the one element to trace from the reference render rather than reconstruct from metrics.

---

## 6 · Palette and measured contrast

Computed in one pass from this panel's own hexes against each ground **by name**. Fascia figures
are against **`#1b1a18`, the lightest end** — the worst case for pale ink on a dark gradient.
Functional 7:1, flavour 4.5:1, state 3:1.

### On fascia (worst `#1b1a18`)

| Ink | Role | Ratio | Class |
|---|---|---|---|
| `#b0aa9c` | section labels, sub-headings, captions, printed numerals, units, SYNC caption, foot strip | **7.52** | functional |
| `#c3bcae` | knob names, REPEATS LIVE | **9.21** | functional |
| `#e7e1d4` | lit stack label | **13.35** | functional |
| `#615c54` | unlit stack label | 2.62 | **intentional — see below** |

**The fascia's whole functional ink class was `#a8a294` and measured 6.84 here.** Its previous
spec quoted **7.36**, which is true against the fascia's *mid* tone `#131211` — not the point the
text sits on. Same error class as this round's two model lines and Chorus-60's units: a figure
measured against the wrong ground reads as passing. `#b0aa9c` is **7.52** worst case and 8.09 at
the mid.

**`#615c54` at 2.62 is deliberate and is not a legibility failure.** Unlit stack rows are printed
legend for the two modes that are not live; the lit row and its lamp carry the current identity.
Dimness *is* the state signal, and per §3 the brand rule forbids conveying it by dimming the
control itself. Dim dial-1 numerals (§2.3) are the same exception in a second place.

### On the header block (worst `#211f1c`)

| Ink | Role | Ratio |
|---|---|---|
| `#cfc6b4` | function descriptor, model line, PROGRAM caption, IN / OUT captions | **9.70** |

**All four of those were `#a8a294`, at 6.47 against this block.** The role sits on the block, not
the fascia, and the block is darker material; reconciled to the six-material strip's `#cfc6b4`.

### On glass, buttons and tape

| Ink | Ground | Role | Ratio |
|---|---|---|---|
| `#cfd8cb` | LCD `#040806` | program name, bank tag, live readout | **13.75** |
| `#b9c3c8` | meter `#05080a` | IN / OUT values | **11.19** |
| `#93a894` | scope `#04060a` | readout base | **7.99** |
| `#a9bda9` | scope | readout highlight, chevron | **10.18** |
| `#ff9d3c` | scope | accent pulses | 9.79 — graphic |
| `#f0eade` | button `#211f1b` | selected label | **13.73** |
| `#b0aa9c` | button | unselected label | **7.11** |
| `#f4f8fa` | cap `#23282c` | Program legend lit | **13.93** |
| `#9aa1a6` | cap | Program legend idle | **5.68** |
| `#23211c` | tape `#ded8c8` | all four tape strings | **11.31** |
| `rgba(24,22,19,.92)` → `#211f1c` | ear `#8e8a82` | RACK 4 stencil | **4.78** — flavour |

**Unselected button labels at 7.11 are deliberately below the selected 13.73** — above the
functional floor, and the pairing of a pressed face, a lit lamp and a brighter label is what
marks selection.

### Accent

**One accent: `#ff9d3c`.** The REPEATS LIVE lamp (13 px, pulsing 1.6 s between 1.0 and 0.72) and
the scope pulses. Nowhere else — not on the LEDs, not on the Program legends, not on a knob.

---

## 7 · State matrices

### 7.1 Program legends — shared part

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | idle | idle | idle | idle |
| Factory Program, edited | **lit** | idle | idle | idle |
| User Program, unmodified | idle | idle | **lit** | idle |
| User Program, edited | **lit** | idle | **lit** | idle |
| Naming a Program | idle | **lit** | idle | **lit** |

**This casting's own 10 px / .14 em legends and `#f2ece0` / `#77736a` inks are retired** in favour
of the part's 11 / 13 / .12 em and `#F4F8FA` / `#9AA1A6` on the shared dark cap. Its four
delivered button-state PNGs (`plate/buttons/01–04`) are therefore superseded and should not ship.

### 7.2 Pages · sync · stereo · image

| Control | States |
|---|---|
| Character | TAPE · BBD · DIGITAL — exactly one lit, per §3 |
| Sync | ON (thumb right, NOTE DIVISION LED lit, TIME LED out) · OFF (inverse; scope reads `FREE · <n> ms`) |
| Division | one of `1/4 · 1/8. · 1/8 · 1/8T · 1/16`, LEDs lit only with Sync ON |
| Stereo | MONO · STEREO · PING-PONG — CROSS-FEED's caption LED lit on PING-PONG only |

### 7.3 Bypass — the §9 gap, now closed

**Fifth Member had no disengaged state at all.** It now has one, host-driven, with no on-panel
control and **no printed BYPASS anywhere** — the foot strip previously carried `BYPASS · v1.0`
borrowed from Chorus-60, where the footer is live; with no state behind it, it permanently
announced that a unit passing audio was bypassed. It reads `SN 0417 · v1.1`.

Treatment: **0.50 `#808080` multiply**, pointers unmoved, scope frozen, REPEATS LIVE out, no
caption, no desaturation, floors suspended.

**Pending amendment:** `shared/BRAND-AMENDMENT-BYPASS.md` would make the multiply cover only what
is disengaged. If adopted this casting takes **two one-line changes** — the header leaves the
multiply, and so do the rack ears, which are chassis rather than controls.

---

## 8 · Type

Every size is a CSS px em size with a pinned line box (call 4). Nothing functional below 11 px
except printed numerals at 10, which sit exactly on the floor.

| Role | Face | Size / line box | Tracking | Ink |
|---|---|---|---|---|
| Wordmark | Permanent Marker | 27 / 28 | .005 em | `#151310` |
| Function descriptor | Barlow Condensed 600 | 13 / 17 | .30 em | `#cfc6b4` |
| Model line | Barlow Condensed 500 | 12 / 14 | .30 em | `#cfc6b4` |
| PROGRAM · IN / OUT captions | Barlow Condensed 600 | 10 / 13 | .24 / .28 em | `#cfc6b4` |
| Section label | Barlow Condensed 600 | 11 / 13 | .28 em | `#b0aa9c` |
| Sub-heading | Barlow Condensed 600 | 11 / 13 | .24 em | `#b0aa9c` |
| SYNC caption | Barlow Condensed 600 | 12 / 14 | .22 em | `#b0aa9c` |
| Knob name | Barlow Condensed 600 | 11 / 14 | .20 em | `#c3bcae` |
| REPEATS LIVE | Barlow Condensed 600 | 12 / 14 | .26 em | `#c3bcae` |
| Stack label | Barlow Condensed 600 | 11 / 14 | .18 em | see §3 |
| Printed numeral | Barlow Condensed 600 | 10 / 12 | .06 em | see §2.3 |
| Division label | Share Tech Mono | 12 / 14 | 0 | see §5 |
| Stereo · character label | Barlow Condensed 600 | 12 / 14 · 13 / 15 | .18 / .24 em | see §5 |
| Program legend | Barlow Condensed 600 | 11 / 13 | .12 em | see 7.1 |
| LCD · meter value | Share Tech Mono | 17 / 20 | .10 em | `#cfd8cb` / `#b9c3c8` |
| Scope readout | Share Tech Mono | 11 / 13 | .12 em | `#93a894` / `#a9bda9` |
| Foot strip | Barlow Condensed 600 | 11 / 13 | .26 em | `#b0aa9c` |
| Tape lettering | Permanent Marker | 27 / 15 / 14 / 12 | — | `#23211c` |
| RACK 4 stencil | Barlow Condensed 600 | 11 / 13 | .34 em | `rgba(24,22,19,.92)` |

**Division button labels stay Share Tech Mono.** They are note values, and this casting's own mono
happens to be the shared LCD face — numerals in the casting's mono, per call 7's split, not an
exception to it.

**The marker face is not distributable**, so the nameplate and all four tape strings **ship as
artwork inside the plate and `fonts/` deliberately contains no binary for Permanent Marker.** An
absent font that is not declared looks like a delivery defect and gets "fixed" by substituting a
face, which moves every measurement taken from the nameplate.

---

## 9 · Conformance — calls this casting already satisfied

**§9 and §10 together account for every call.** A call appearing in neither this section nor the
changelog is a gap by construction, not an omission.

| Call | State |
|---|---|
| **5** — code-drawn, cached, no filmstrips | **already conformed and already argued.** Five diameters × 128 frames × 3 scales is 1,920 frames and ~60 MB for a body whose specular does not rotate. `setBufferedToImage` is the build's to add. |
| **6** — plates export at 3× | **conformed on scale, invalidated on base.** A 3× plate exists at 3720 × 2793 — 3× the superseded 1240 × 931 canvas. See §10. |
| **7** — Barlow Condensed panel lettering | **already conformed** throughout, with the wordmark outside the call and note values in the casting's mono. |
| **4's floor** | **already conformed** — every functional string was already ≥ 11 px with numerals at 10. |
| **Tick law** | **already conformed.** Majors 2 × 9, minors 1.5 × 5 from a common R+17 tip, numerals anchored by the edge facing the dial — the suite's rule reached independently, before it was written down. |
| **Registration** | **conformed this round** by the `dy` construction in §2.1, which this casting's character row had already solved at dy 44 for dial 1. |
| **§4B shoes** | **not applicable, checked.** Fifth Member has no shoe: its multi-state controls are LED buttons, which is its own vocabulary. |

---

## 10 · Changelog and outstanding

### This round

1. **Frame 1240 → 1340; window 1444 × 1012** (call 1, as the frame clause — §1). Sections
   re-spaced: TIMING 296 → 320, OUTPUT 322 → 340, REPEATS 464 → 612, scope 1092 → 1308.
2. **Five diameters → two** (call 3): FEEDBACK 84 → 76, MIX 82 → 76, character dials stay 76,
   CROSS-FEED and OUTPUT TRIM 62 → 56, DAMPING and SATURATION 66 → 56.
3. **TIME promoted 66 → Ø76** to keep five numerals on a skewed ring (§2).
4. **Numeral counts re-cut**: FEEDBACK six → five quartiles with 110 % still marked; OUTPUT TRIM
   five → three; DAMPING five → three; dial 1's outer Hz ring six → five. TIME's first cut
   printed six and was corrected to five.
5. **LCD to Share Tech Mono 17 / .10 em in the shared 641 cell** (call 2): bank cell 67.2 → 72,
   chevron cell 28 → the 30 trim at a 16 px inset, **form unchanged** — the 14 × 8 stroked path
   was already this casting's, and is now the suite's. Budget 28 → **49**, cap 26 → **47**.
6. **Program buttons to the shared dark cap** — this casting's own faces, inks and 10 px legends
   retired, and its four delivered button PNGs superseded.
7. **Meter wells 76 → 64**, captions to 10 / 13 / .28 em with the `dB` unit added.
8. **Fascia functional ink `#a8a294` → `#b0aa9c`** (6.84 → 7.52 worst case) and the four
   header-block roles → `#cfc6b4` (6.47 → 9.70). Both were measured against the wrong ground.
9. **Bypass added**, closing the one substantive gap in the old spec (§7.3).
10. **Every size given a pinned line box** (call 4); canvas height pinned at 1012.

### Outstanding

- **The plate must be re-cut at 3× against the new frame: 4020 × 3036** (`handoff/MANIFEST.md`
  row 7). The existing 3720 × 2793 is 3× of a canvas that no longer exists — correct ratio,
  superseded base. **It also has to carry §10.8's two ink changes**, since the plate bakes panel
  typography including the model line: the panel and its plate would otherwise disagree on the
  one figure this round fixed twice.
- **Drop `plate/buttons/01–04`** from the bundle — superseded by the shared Program cap.
- Wire both meter wells and the scope to real signal.
- By-ear tuning of the six factory Programs; names are authored, values are not final.
- Confirm the three skews and dial 1's per-mode bindings against the build's parameter table.
- **`shared/HEADER-PART.md` revision 3** — three figure items waiting on build answers; its
  fourth, the propagation process, is written and does not wait.
- **`shared/BRAND-AMENDMENT-BYPASS.md`** — two one-line changes here if adopted (§7.3).
