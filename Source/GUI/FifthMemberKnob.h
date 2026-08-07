#pragma once

#include "FifthMemberTheme.h"

/**
    A panel knob.

    Subclasses juce::Slider for its drag-to-value mapping and SliderAttachment compatibility;
    paint() replaces the default look entirely.

    **The animation mechanic, ported from CHORUS-60.** SliderAttachment still sets the parameter
    instantly - automation, host reads and the value tooltip all stay correct. What is decoupled is
    what gets *drawn*: setDisplayProportion() overrides the drawn angle, and the editor's 60 Hz
    timer slews that override toward the parameter's own proportion. Three traps CHORUS-60
    documents the hard way:

      - slew in ROTATION proportion, not parameter value - for a skewed parameter (Time, Damping,
        Mod Rate here) those are not the same motion;
      - create knob components ONCE and re-attach them, never recreate per mode - recreating resets
        the very rotation the animation exists to move away from, degrading it to a snap;
      - a knob being dragged tracks the pointer 1:1, because slewing under the user's own hand
        reads as lag rather than as motion.

    Per Correction 2 there is no dim, disable or inert state of any kind. The body renders
    identically at all times regardless of mode or switch state; only LEDs elsewhere on the panel
    change. CHORUS-60's dimFactor is deliberately not ported.
*/
class FifthMemberKnob final : public juce::Slider
{
public:
    explicit FifthMemberKnob (FifthMemberTheme::Layout::KnobSize size);

    const FifthMemberTheme::Layout::KnobVariant& variant() const noexcept
    {
        return FifthMemberTheme::Layout::variantFor (knobSize);
    }

    void setCentrePosition (juce::Point<float> centre);

    void paint (juce::Graphics& g) override;

    /** Draw at a rotation other than the parameter's own, so the knob can slew instead of
        snapping. Negative returns it to following the parameter directly. */
    void setDisplayProportion (float proportion) noexcept;
    float getDrawnProportion();

    /** Paints a knob body at an explicit centre, independent of any Slider - so the geometry is
        testable without a live Component. */
    static void paintBody (juce::Graphics& g, juce::Point<float> centre,
                           const FifthMemberTheme::Layout::KnobVariant& v, float value01);

private:
    FifthMemberTheme::Layout::KnobSize knobSize;
    float displayOverride = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FifthMemberKnob)
};
