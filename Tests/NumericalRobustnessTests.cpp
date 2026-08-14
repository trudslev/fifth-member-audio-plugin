#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 2 of the suite-wide bug sweep, for Fifth Member.

    ## HOWL is the best test case in the suite, and this is why

    Every other numerical case here asks "does something go wrong that shouldn't". HOWL asks the
    harder question: **a designed instability must stay bounded.** Feedback reaches 110 % by design
    and factory Program `howl` ships at **105 %**, bounded by `DelayCore.h:67`'s
    `safetyCeiling = 1.4f`, whose own comment says its job is to make that howl rather than overflow.

    So the bar is **bounded and finite, not merely non-NaN.** A run that produced 1e30 every sample
    would pass an is-finite check and be completely broken; the ceiling is a claim about magnitude
    and that is what gets asserted.

    The comment names the case explicitly — "a self-oscillating patch at Saturation 0 % has nothing
    limiting it" — so Saturation 0 % is the configuration under test rather than a convenient one.

    ## The survey's ordering heuristic is disproven and is not used here

    Reflect-84 was scanned first because it had four tanks and only two tiny-constant guards against
    TapeRot's ten. **The scanner found nothing.** So guard count does not predict subnormal reach,
    and the remaining castings are not ordered by it — this one is next because HOWL is the most
    interesting case in the suite, not because a count said so.

    ## What a clean subnormal row means, stated so it is not read as more

    `processBlock` opens with `juce::ScopedNoDenormals`, so a subnormal intermediate is flushed to
    zero by the hardware and never reaches the output. Clean means "the guard covers every path the
    output can see", not "no denormals occur". Internal subnormals that never reach the output are
    invisible here and cost nothing where FTZ is honoured.
*/
class NumericalRobustnessTests final : public juce::UnitTest
{
public:
    NumericalRobustnessTests() : juce::UnitTest ("Numerical robustness", "dsp") {}

    void runTest() override
    {
        beginTest ("HOWL at Saturation 0 % — the case the safety ceiling names");
        {
            FifthMemberAudioProcessor processor;

            // The per-casting meaning: which parameters make this delay self-oscillate, and that
            // Saturation 0 % is the case DelayCore.h:67's comment calls out as otherwise unlimited.
            set (processor, ParamIDs::feedback,   1.0f);   // the top of the range, past HOWL's 105 %
            set (processor, ParamIDs::saturation, 0.0f);   // the named case
            set (processor, ParamIDs::mix,        1.0f);
            set (processor, ParamIDs::damping,    0.0f);   // nothing else removing energy

            logMessage ("  feedback -> " + readBack (processor, ParamIDs::feedback)
                            + ", saturation -> " + readBack (processor, ParamIDs::saturation));

            nf::testing::RenderSpec spec;
            spec.numBlocks = 64;                            // plenty of excitation to run away on

            const auto report = nf::testing::scanTail (processor, spec, 4000);
            logMessage ("  HOWL -> " + report.describe());

            expectEquals (report.nans, 0, "HOWL produced NaN: " + report.describe());
            expectEquals (report.infinities, 0, "HOWL produced Inf: " + report.describe());

            // **The real bar. `safetyCeiling` is 1.4, so a bounded howl peaks near it.** A generous
            // multiple is allowed for the stages after the ceiling — this asserts that the
            // oscillation is LIMITED, not that it is limited to a specific number, which would pin
            // an implementation detail the sweep is not entitled to fix.
            expectLessThan (report.peakAbs, 16.0,
                            "HOWL is finite but not bounded — it reached "
                                + juce::String (report.peakAbs, 3)
                                + ", which a non-NaN check would have passed. safetyCeiling is 1.4.");

            expectGreaterThan (report.peakAbs, 0.1,
                               "HOWL did not oscillate at all, so the ceiling was never tested");
        }

        beginTest ("An unstable loop RECOVERS rather than staying broken until reload");
        {
            // The plan's question, and the one that decides whether an absurd parameter sweep is a
            // support burden: after driving it into oscillation, does turning feedback down bring it
            // back, or is the instance dead?
            FifthMemberAudioProcessor processor;

            set (processor, ParamIDs::feedback,   1.0f);
            set (processor, ParamIDs::saturation, 0.0f);
            set (processor, ParamIDs::mix,        1.0f);

            nf::testing::RenderSpec spec;
            spec.numBlocks = 64;
            nf::testing::render (processor, spec);

            set (processor, ParamIDs::feedback, 0.0f);      // back to sane

            const auto after = nf::testing::scanTail (processor, spec, 2000);
            logMessage ("  after recovery -> " + after.describe());

            expect (after.clean(), "the instance did not recover: " + after.describe());
            expect (after.blocksUntilSilent >= 0,
                    "feedback was returned to zero and the tail still never fell silent: "
                        + after.describe());
        }

        beginTest ("Ordinary settings, whole audio path, long tail");
        {
            FifthMemberAudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.numBlocks = 16;

            const auto report = nf::testing::scanTail (processor, spec, 4000);
            logMessage ("  defaults -> " + report.describe());

            expectEquals (report.subnormals, 0,
                          "subnormals reached the output — ScopedNoDenormals is not covering this "
                          "path: " + report.describe());
            expect (report.clean());
        }

        beginTest ("The denormal guard is ACTIVE — the one line the whole suite rests on");
        {
            // **RULING TAKEN: assert the processor-level guard rather than putting a floor in one
            // filter.** ScopedNoDenormals is one line in one file per casting, and category 2's
            // survey established that no DSP stage in the suite carries its own guard. So every
            // decaying path in this plugin — including the control paths no output scan can reach —
            // is covered by a single statement that, until this test, nothing asserted.
            //
            // Mechanism: feed SUBNORMAL input and see whether it survives. Flush-to-zero also treats
            // subnormal inputs as zero, so a subnormal cannot survive a guarded processBlock while
            // an unguarded one passes it through. This therefore fails if the guard is REMOVED,
            // NARROWED to part of the function, or a path is SCOPED PAST it — the three ways one
            // line stops covering what it appears to.
            //
            // Core's own tests prove the checker can tell guarded from unguarded (1024 in -> 1024
            // out against 1024 in -> 0 out). Without that proof this assertion would be worthless,
            // because "no subnormals survived" is also what a checker that measures nothing reports.
            FifthMemberAudioProcessor processor;
            const auto guard = nf::testing::probeDenormalGuard (processor);

            logMessage ("  " + guard.describe());

            expect (guard.guardActive,
                    "ScopedNoDenormals is not covering processBlock. Every decaying path in this "
                    "plugin depends on it, and nothing else guards them: " + guard.describe());
        }
    }

private:
    static void set (FifthMemberAudioProcessor& p, const char* id, float normalised)
    {
        if (auto* param = p.apvts.getParameter (id))
            param->setValueNotifyingHost (normalised);
    }

    /** The parameter's own text, not the value that was requested — a sweep that collapsed shows as
        repeats here rather than as a plausible result. */
    static juce::String readBack (FifthMemberAudioProcessor& p, const char* id)
    {
        if (auto* param = p.apvts.getParameter (id))
            return param->getCurrentValueAsText();

        return "<missing>";
    }
};

static NumericalRobustnessTests numericalRobustnessTests;
