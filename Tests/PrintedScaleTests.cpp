#include "../Source/Parameters.h"
#include "../Source/GUI/FifthMemberTheme.h"
#include <nf/PrintedScale.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <vector>

/**
    The printed scales are the panel's only at-rest value reference, so a mark that names an angle
    the knob does not reach is a lie the user has no way to check. These tests are the guard on
    exactly that.

    Two things are asserted, and the first matters more than the second. Every mark must land on the
    angle the BUILD's own taper puts it at — computed from the same NormalisableRange the pointer
    uses, not from a table of angles — so a future range or skew change fails here rather than
    silently invalidating a ring. And every mark must lie inside its parameter's range, since a mark
    outside it clamps to an end stop and stacks on its neighbours (which is precisely the bug dial 1
    shipped with for one commit).
*/
class PrintedScaleTests final : public juce::UnitTest
{
public:
    PrintedScaleTests() : juce::UnitTest ("PrintedScale", "GUI") {}

    void runTest() override
    {
        using namespace FifthMemberTheme::Layout;

        // The angles build-handoff section 4.4 prints, to its own 0.01 degree.
        beginTest ("Every skewed ring's marks land on the angles the handoff prints");
        {
            const auto timeRange = [] { juce::NormalisableRange<float> r { Timing::minDelayMs,
                                                                            Timing::maxDelayMs, 0.0f };
                                        r.setSkewForCentre (375.0f); return r; }();
            checkAngles (timeRange, timeMarks, (int) std::size (timeMarks),
                          { -135.00f, -114.33f, -106.10f, -95.63f, -76.75f, -57.08f, -31.00f,
                             0.00f, 17.10f, 44.91f, 67.67f, 104.70f, 135.00f }, "TIME");

            const auto dampingRange = [] { juce::NormalisableRange<float> r { 1000.0f, 16000.0f, 1.0f };
                                            r.setSkewForCentre (4000.0f); return r; }();
            checkAngles (dampingRange, dampingMarks, (int) std::size (dampingMarks),
                          { -135.00f, -50.89f, -21.63f, 0.00f, 33.22f, 59.45f, 101.24f, 135.00f },
                          "DAMPING");

            // **The range comes from Parameters.h, not from a third copy built here.** It used to be
            // rebuilt inline, which made this test agree with itself: the parameter, dial 1's Hz
            // ring and this assertion each carried their own 0.1/5.0/centre-1.0, and a change to
            // any one of them left the other two quietly legending the old taper.
            checkAngles (modRateRange(), modRateMarks, (int) std::size (modRateMarks),
                          { -135.00f, -70.13f, -38.11f, 0.00f, 26.74f, 48.26f, 82.86f, 110.93f, 135.00f },
                          "MOD RATE");
        }

        beginTest ("A linear ring spaces evenly, and the arc ends carry a mark at BOTH ends");
        {
            juce::NormalisableRange<float> percent { 0.0f, 100.0f, 0.1f };
            checkAngles (percent, percentFullMarks, (int) std::size (percentFullMarks),
                          { -135.0f, -67.5f, 0.0f, 67.5f, 135.0f }, "percent");

            // The ring this replaced ran a full 360 at fixed pitch, which put a mark at +135 with
            // no twin at -135 and marks below the horizontal at both ends. Assert the endpoints.
            expectWithinAbsoluteError (angleOf (percent, percentFullMarks[0].value), -135.0f, 0.01f,
                                        "first mark must sit at the arc start");
            expectWithinAbsoluteError (angleOf (percent, percentFullMarks[4].value), 135.0f, 0.01f,
                                        "last mark must sit at the arc end");
        }

        beginTest ("Output Trim's ring is cut against the build's range, not the handoff's");
        {
            // Section 4.4 prints -24..+12; Parameters.h is -24..+24. On the real range its table
            // would put 0 dB at +45 where the pointer reaches it at 0.
            juce::NormalisableRange<float> trim { -24.0f, 24.0f, 0.1f };
            checkAngles (trim, trimMarks, (int) std::size (trimMarks),
                          { -135.0f, -101.25f, -67.5f, -33.75f, 0.0f, 33.75f, 67.5f, 101.25f, 135.0f },
                          "OUTPUT TRIM");

            expectEquals (juce::String (trimMarks[6].printed), juce::String ("+12"),
                           "a bipolar dB ring keeps its leading plus - deriving the label drops it");
        }

        beginTest ("No mark lies outside the range its ring legends");
        {
            // A mark outside the range clamps to an end stop and stacks on its neighbours, which is
            // invisible in code and obvious only on the panel.
            const auto inRange = [this] (const ScaleMark* m, int n, float lo, float hi, const char* what)
            {
                for (int i = 0; i < n; ++i)
                    expect (m[i].value >= lo && m[i].value <= hi,
                             juce::String (what) + ": " + juce::String (m[i].value) + " is outside "
                                 + juce::String (lo) + ".." + juce::String (hi));
            };

            inRange (timeMarks,    (int) std::size (timeMarks),    1.0f, 2000.0f, "TIME");
            inRange (dampingMarks, (int) std::size (dampingMarks), 1000.0f, 16000.0f, "DAMPING");
            inRange (modRateMarks, (int) std::size (modRateMarks), 0.1f, 5.0f, "MOD RATE");
            inRange (feedbackMarks,(int) std::size (feedbackMarks), 0.0f, 110.0f, "FEEDBACK");
            inRange (trimMarks,    (int) std::size (trimMarks),   -24.0f, 24.0f, "OUTPUT TRIM");
            inRange (percentFullMarks,   (int) std::size (percentFullMarks),   0.0f, 100.0f, "percent");
            inRange (percentSparseMarks, (int) std::size (percentSparseMarks), 0.0f, 100.0f, "percent sparse");
        }

        beginTest ("Every ring passes the suite's shared printed-scale check");
        {
            // The cases above assert this panel's rings against the handoff's PUBLISHED angles,
            // which also checks the transcription. This one asserts them against the parameter's own
            // range through nf::printedScaleDefects - the check every casting runs, which is what
            // catches a taper moving out from under a ring rather than a figure being mistyped.
            //
            // Both are worth having and they fail for different reasons: the first says "the spec
            // and the build disagree", this one says "the ring and the control disagree".
            checkShared (modRateRange(), modRateMarks, (int) std::size (modRateMarks), "MOD RATE");

            juce::NormalisableRange<float> percent { 0.0f, 100.0f, 0.1f };
            checkShared (percent, percentFullMarks, (int) std::size (percentFullMarks), "percent");

            juce::NormalisableRange<float> feedback { 0.0f, 110.0f, 0.1f };
            checkShared (feedback, feedbackMarks, (int) std::size (feedbackMarks), "FEEDBACK");

            juce::NormalisableRange<float> trim { -24.0f, 24.0f, 0.1f };
            checkShared (trim, trimMarks, (int) std::size (trimMarks), "OUTPUT TRIM");
        }
    }

private:
    /** Runs the suite's shared check over one ring and reports every defect by name. */
    void checkShared (const juce::NormalisableRange<float>& range,
                      const FifthMemberTheme::Layout::ScaleMark* marks, int count,
                      const juce::String& ringName)
    {
        std::vector<nf::PrintedMark> printed;

        for (int i = 0; i < count; ++i)
            printed.push_back ({ marks[i].value,
                                 angleOf (range, marks[i].value) });

        for (const auto& defect : nf::printedScaleDefects (range, printed))
            expect (false, ringName + ": " + defect);
    }

    static float angleOf (const juce::NormalisableRange<float>& r, float value)
    {
        using namespace FifthMemberTheme::Layout;
        return knobArcStartDegrees
             + r.convertTo0to1 (value) * (knobArcEndDegrees - knobArcStartDegrees);
    }

    void checkAngles (const juce::NormalisableRange<float>& range,
                      const FifthMemberTheme::Layout::ScaleMark* marks, int count,
                      std::initializer_list<float> expected, const char* what)
    {
        expectEquals (count, (int) expected.size(), juce::String (what) + ": mark count");

        int i = 0;
        for (float want : expected)
        {
            if (i >= count) break;
            // 0.02 degrees: the handoff quotes to 0.01, and this is derived rather than transcribed,
            // so anything larger than rounding is a real disagreement.
            expectWithinAbsoluteError (angleOf (range, marks[i].value), want, 0.02f,
                                        juce::String (what) + " mark " + juce::String (i));
            ++i;
        }
    }
};

static PrintedScaleTests printedScaleTests;
