#include "TestUtils.h"

#include "../Source/DSP/FactoryPrograms.h"

#include <juce_core/juce_core.h>

#include <set>

/**
    Structural guards on the bank. The headline case is the zero-fill: per Correction 1 a Program
    stores only its active path, so the fields belonging to a non-selected alternative must be
    exact zeros rather than plausible-looking filler. Asserting that here is what stops someone
    later "completing" the table and quietly reintroducing the every-parameter-stored model.
*/
class FactoryProgramsTests final : public juce::UnitTest
{
public:
    FactoryProgramsTests() : juce::UnitTest ("FactoryPrograms", "programs") {}

    void runTest() override
    {
        beginTest ("the bank is the eleven settled names, in order");
        {
            expectEquals (kNumFactoryPrograms, 11);

            const juce::StringArray expected {
                "YOU TOO?", "SKY WIDE", "NEW YEAR'S", "GREAT GIG", "DARK ECHOES", "LONG LOOP",
                "SLOW BUILD", "SLAP HAPPY", "DOUBLED UP", "SIXTEENTH SENSE", "HOWL"
            };

            for (int i = 0; i < kNumFactoryPrograms; ++i)
                expectEquals (juce::String (kFactoryPrograms[(size_t) i].name), expected[i]);

            expectEquals (defaultFactoryProgramIndex, 0);
        }

        beginTest ("names are unique, upper case, and fit the LCD");
        {
            std::set<juce::String> seen;

            for (const auto& p : kFactoryPrograms)
            {
                const juce::String name { p.name };
                expect (name.isNotEmpty());
                expect (name.length() <= 22, "too long for the LCD: " + name);
                expect (name == name.toUpperCase(), "not upper case: " + name);
                expect (seen.insert (name).second, "duplicate: " + name);
            }
        }

        beginTest ("ONLY the active path carries data - everything else is exactly zero");
        {
            for (const auto& p : kFactoryPrograms)
            {
                const juce::String name { p.name };

                // Timing: exactly one of the two.
                if (p.sync)
                {
                    expect (p.division >= 0 && p.division < numNoteDivisions, name + ": bad division");
                    expect (p.timeMs == 0.0f, name + ": stores a Time it does not own");
                }
                else
                {
                    expect (p.division == -1, name + ": stores a division it does not own");
                    expect (p.timeMs >= Timing::minDelayMs && p.timeMs <= Timing::maxDelayMs,
                            name + ": Time out of range");
                }

                // Character: exactly one group of three.
                const bool tape = p.character == (int) DelayCharacter::tape;
                const bool bbd = p.character == (int) DelayCharacter::bbd;
                const bool digital = p.character == (int) DelayCharacter::digital;

                expect (tape || bbd || digital, name + ": bad character index");

                if (! tape)
                {
                    expect (p.wow == 0.0f && p.flutter == 0.0f && p.genLoss == 0.0f,
                            name + ": stores Tape dials it does not own");
                }
                else
                {
                    expect (p.wow > 0.0f && p.flutter > 0.0f && p.genLoss > 0.0f,
                            name + ": a Tape Program with a dead dial");
                }

                if (! bbd)
                    expect (p.modRateHz == 0.0f && p.modDepth == 0.0f,
                            name + ": stores BBD dials it does not own");
                else
                    expect (p.modRateHz >= 0.1f && p.modRateHz <= 5.0f && p.modDepth > 0.0f,
                            name + ": BBD values out of range");

                if (! digital)
                    expect (p.degrade == 0.0f, name + ": stores a Degrade it does not own");
                else
                    expect (p.degrade > 0.0f, name + ": a Digital Program with no degrade");

                // Cross-Feed is the third conditional axis, on Stereo Mode rather than on Sync or
                // Character. The lower bound is not decoration: Ping-Pong at cross 0 leaves the
                // right line silent, and at 0.5 both lines get the same mix and the bounce collapses
                // to the centre, so a Ping-Pong Program below ~0.6 is not the effect it is named for.
                if ((StereoMode) p.stereoMode != StereoMode::pingPong)
                    expect (p.crossFeedPercent == 0.0f, name + ": stores a Cross-Feed it does not own");
                else
                    expect (p.crossFeedPercent >= 60.0f && p.crossFeedPercent <= 100.0f,
                            name + ": a Ping-Pong Program whose cross does not bounce");
            }
        }

        beginTest ("every always-stored field is inside its declared range");
        {
            for (const auto& p : kFactoryPrograms)
            {
                const juce::String name { p.name };

                expect (p.feedbackPercent >= 0.0f && p.feedbackPercent <= 110.0f, name + ": feedback");
                expect (p.stereoMode >= 0 && p.stereoMode < numStereoModes, name + ": stereo mode");
                expect (p.dampingHz >= 1000.0f && p.dampingHz <= 16000.0f, name + ": damping");
                expect (p.saturationPercent >= 0.0f && p.saturationPercent <= 100.0f, name + ": saturation");
                expect (p.mixPercent > 0.0f && p.mixPercent <= 100.0f, name + ": mix (a dry delay is a broken one)");
                expect (p.trimDb >= -24.0f && p.trimDb <= 24.0f, name + ": trim");
            }
        }

        beginTest ("the bank is eleven distinct starting points, and exercises every selector");
        {
            std::set<int> characters, stereoModes;
            int syncedCount = 0, freeCount = 0;

            for (const auto& p : kFactoryPrograms)
            {
                characters.insert (p.character);
                stereoModes.insert (p.stereoMode);
                (p.sync ? syncedCount : freeCount)++;
            }

            expectEquals ((int) characters.size(), 3, "not all three Delay Characters are represented");
            expectEquals ((int) stereoModes.size(), 3, "not all three Stereo Modes are represented");
            expect (syncedCount > 0 && freeCount > 0, "the bank should cover both Sync states");
        }

        beginTest ("HOWL self-oscillates, which is the point of a 110% ceiling");
        {
            const auto& howl = kFactoryPrograms[10];
            expectEquals (juce::String (howl.name), juce::String ("HOWL"));
            expect (howl.feedbackPercent > 100.0f, "HOWL is not above unity");
            expect (howl.trimDb < 0.0f, "a self-oscillating Program should pull its output down");
        }
    }
};

static FactoryProgramsTests factoryProgramsTests;
