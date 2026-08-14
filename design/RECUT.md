# RE-CUT SHEET — FIFTH MEMBER DL-88

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `plate/fifth-member-plate-3x.png` | **1340 × 1012** (current frame) | 3720 × 2793 | 3× of **1240 × 931** — superseded | **4020 × 3036** |

**This asset was on the round’s completed list.** It is a correct 3× — of a canvas that no longer
exists. An asset marked done against a superseded base leaves the queue, which is worse than one
marked outstanding.

**The re-cut must also carry two ink changes** (GUI-SPEC §10.8), because this plate bakes panel
typography including the model line:

| Role | Was | Now |
|---|---|---|
| fascia functional class — section labels, sub-headings, captions, numerals, units, SYNC caption, foot strip | `#a8a294` (6.84 worst case) | **`#b0aa9c`** (7.52) |
| header-block roles — descriptor, model line, PROGRAM, IN / OUT captions | `#a8a294` (6.47 on the block) | **`#cfc6b4`** (9.70) |

Cut from the current panel rather than repainting the old plate. **The frame is 1340; the two
52 px rack ears sit outside it and are drawn, never baked.**

**Delete `plate/buttons/01–04` — do not ship.** Those four are this casting’s own retired
Program-button faces, superseded by the shared dark cap (GUI-SPEC §7.1). Shipping them would put
the retired faces beside the part’s in one bundle.
