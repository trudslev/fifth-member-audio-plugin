#pragma once

#include "FifthMemberTheme.h"

/**
    The panel plate, blitted.

    Everything static is in `design/plate/fifth-member-plate-2x.png` as of revision 2 - the fascia
    and its wear, the rack ears and screws, the nameplate and the three other tape elements, the
    section frames and their labels, every panel label, and every printed tick and numeral. Build
    handoff section 1 is the manifest and it cuts both ways: **redrawing something the plate carries
    double-prints it at a one-pixel offset, and baking something live freezes it.**

    This class used to rasterise all of that procedurally into a juce::Image at construction, because
    the prototype was 100 % CSS with no assets at all and there was nothing to blit. That is what the
    plate replaced; roughly 500 lines of fascia gradients, corner wears, scuff angles, screw slots
    and silkscreen went with it. If something static looks wrong now, it is wrong in the plate and
    gets re-cut from `Chorus-60`-style prototype source, not patched here.

    2x only. A 1:1 blit to a Retina display resolves the fascia's 3px brush and the ears' 2px one to
    a flat wash and the metal stops reading as metal - which is why the old bake ran at 2x too.
*/
class PanelBackground final : public juce::Component
{
public:
    PanelBackground();

    void paint (juce::Graphics& g) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelBackground)
};
