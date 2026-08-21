#pragma once

#include "FifthMemberTheme.h"

/**
    The rack ears, drawn, and the panel plate blitted over the frame between them.

    **The plate is the FRAME, not the window.** `design/plate/fifth-member-plate-3x.png` is
    4020 × 3036 — 1340 × 1012 at 1× — which is the shared coordinate frame; this casting's window is
    1444 because §1's rack ears sit outside it, 52 px each side. So the plate blits at
    (`frameX`, 0) and the ears are painted either side. A blit at (0, 0) puts the whole panel under
    the left ear.

    **That is a change from the 2× plate, which baked the ears with everything else.** Its own
    manifest read "the fascia and its wear, **the rack ears and screws**, the nameplate…"; the
    re-cut drops them, because a plate covering the window would have to be 1444 wide and would
    then disagree with every other casting's 1340.

    What the plate still carries is §11's list read backwards: everything except the layers it
    flags `off` and the wells it flags `blank`. Build-handoff section 1 is the manifest and it cuts
    both ways — **redrawing something the plate carries double-prints it at a one-pixel offset, and
    baking something live freezes it.** If something static looks wrong, it is wrong in the plate
    and gets re-cut, not patched here.

    3× now, not 2×. A 1:1 blit to a Retina display resolves the fascia's 3 px brush and the ears'
    2 px one to a flat wash and the metal stops reading as metal.
*/
class PanelBackground final : public juce::Component
{
public:
    PanelBackground();

    void paint (juce::Graphics& g) override;

private:
    void paintEar (juce::Graphics&, juce::Rectangle<float> ear, bool mirrored) const;
    void paintEarMarks (juce::Graphics&) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelBackground)
};
