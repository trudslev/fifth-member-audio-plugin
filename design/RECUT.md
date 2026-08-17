# RE-CUT SHEET — FIFTH MEMBER DL-88

**Every row carries delivered *and* target dimensions.** A target dimension read without its
base is how three figures went wrong in this round: a needle height taken from a placement
offset, a plate "already 3×" of a canvas that no longer exists, a sprite "2×" against an old
frame. All three were true ratios with the base left out. This sheet exists so the target never
travels without it — `../MANIFEST.md` has the same rows for the whole suite.

| File | Drawn at 1× | Delivered | Ratio | **Target (3×)** |
|---|---|---|---|---|
| `plate/fifth-member-plate-3x.png` — **CUT, this bundle** | **1340 × 1012** (current frame) | **4020 × 3036** | 3× | **4020 × 3036** |

**The flags exist now, and they are what made the cut possible.** The panel carries
`data-plate="off"` on the layers the plate omits and `data-plate="blank"` on the wells whose
contents it drops, and the logic class reads them: 36 `off` layers and 3 `blank` wells, with the
scope's repaint frozen while the flag is on. The panel repaints ~22 times a second for the scope,
so suppression from outside the source does not survive a capture — Chorus-60's plate cut in one
pass only because that panel has no animation loop.

**A specified mechanism that was never implemented read identically to one that was, right up
until it was used.** These flags were in this casting's handoff before they existed in its source.
The reader is now in the logic class beside them, so the next re-cut exercises the same path.

**Plate mode empties the live strings at render level as well as hiding the layers.** A capture
that re-renders the tree still comes out with empty wells — the DOM pass alone would have left a
re-render to repaint the readouts.

**The cut carries §10.8's two ink changes**, because this plate bakes panel typography including
the model line: `#a8a294 → #b0aa9c` on the fascia's functional class, and the four header-block
roles to `#cfc6b4`. It was cut from the current panel, not repainted onto the old plate.

**The frame is 1340; the two 52 px rack ears sit outside it and are drawn, never baked.** The cut
was taken from the 1444-wide window at 3× and cropped to the frame at x 156, so the ears — and the
DLY 4 and HALDEN HALL tapes on them — are not in the file.

**`plate/buttons/01–04` are not in this bundle** — this casting's own retired Program-button faces,
superseded by the shared dark cap (GUI-SPEC §7.1). Shipping them would have put the retired faces
beside the part's in one bundle.
