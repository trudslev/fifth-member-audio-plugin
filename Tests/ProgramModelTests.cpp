#include "TestUtils.h"

#include "../Source/DSP/ProgramManager.h"

#include <juce_core/juce_core.h>

/**
    Correction 1, in tests.

    A Program stores only the parameters on its active path. Everything a mutually exclusive
    selector is not currently pointing at persists independently across Program changes, the way a
    physical knob keeps its position regardless of which patch is recalled.

    This is the suite that would catch the model silently reverting to the every-parameter-stored
    approach - which is what the earlier design did, and which loading any Program would otherwise
    hide by simply overwriting everything.
*/
class ProgramModelTests final : public juce::UnitTest
{
public:
    ProgramModelTests() : juce::UnitTest ("Program model (Correction 1)", "programs") {}

    void runTest() override
    {
        beginTest ("Cross-Feed is never Program state and survives every Program change");
        {
            ScopedTestDirectory dir { "crossfeed" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();
            host.setPlain (ParamIDs::crossFeed, 17.5f);

            // Walk the whole factory bank; Cross-Feed must not move once.
            for (int i = 0; i < kNumFactoryPrograms; ++i)
            {
                manager.requestProgramChange (i);
                manager.flushPendingChange();

                expectWithinAbsoluteError (host.plain (ParamIDs::crossFeed), 17.5f, 0.05f,
                                           "Program " + manager.getProgramName (i) + " moved Cross-Feed");
            }
        }

        beginTest ("the two non-selected character modes keep their values");
        {
            ScopedTestDirectory dir { "character" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();

            // Distinctive values in all three modes' dials.
            host.setPlain (ParamIDs::wow, 77.0f);
            host.setPlain (ParamIDs::flutter, 66.0f);
            host.setPlain (ParamIDs::genLoss, 55.0f);
            host.setPlain (ParamIDs::modRate, 3.3f);
            host.setPlain (ParamIDs::modDepth, 44.0f);
            host.setPlain (ParamIDs::degrade, 22.0f);

            // 04 GREAT GIG is a Tape Program: it should write the Tape dials and leave BBD and
            // Digital exactly where they were.
            manager.requestProgramChange (3);
            manager.flushPendingChange();

            expectWithinAbsoluteError (host.plain (ParamIDs::wow), 30.0f, 0.05f, "Tape wow was not applied");
            expectWithinAbsoluteError (host.plain (ParamIDs::modRate), 3.3f, 0.02f, "BBD rate was clobbered");
            expectWithinAbsoluteError (host.plain (ParamIDs::modDepth), 44.0f, 0.05f, "BBD depth was clobbered");
            expectWithinAbsoluteError (host.plain (ParamIDs::degrade), 22.0f, 0.05f, "Digital degrade was clobbered");

            // 08 SLAP HAPPY is Digital: it writes Degrade and leaves the Tape dials alone.
            manager.requestProgramChange (7);
            manager.flushPendingChange();

            expectWithinAbsoluteError (host.plain (ParamIDs::degrade), 10.0f, 0.05f, "Digital degrade was not applied");
            expectWithinAbsoluteError (host.plain (ParamIDs::wow), 30.0f, 0.05f, "Tape wow was clobbered");
            expectWithinAbsoluteError (host.plain (ParamIDs::modRate), 3.3f, 0.02f, "BBD rate was clobbered");
        }

        beginTest ("the inactive timing control keeps its value");
        {
            ScopedTestDirectory dir { "timing" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();

            host.setPlain (ParamIDs::timeMs, 1234.0f);
            host.setPlain (ParamIDs::noteDivision, (float) NoteDivision::sixteenth);

            // 01 YOU TOO? is synced: it writes Note Division and must not touch Time.
            manager.requestProgramChange (0);
            manager.flushPendingChange();

            expectEquals (divisionOf (host), (int) NoteDivision::dottedEighth, "division was not applied");
            expectWithinAbsoluteError (host.plain (ParamIDs::timeMs), 1234.0f, 1.0f, "Time was clobbered by a synced Program");

            // 06 LONG LOOP is free-running: it writes Time and must not touch Note Division.
            manager.requestProgramChange (5);
            manager.flushPendingChange();

            expectWithinAbsoluteError (host.plain (ParamIDs::timeMs), 900.0f, 1.0f, "Time was not applied");
            expectEquals (divisionOf (host), (int) NoteDivision::dottedEighth,
                          "Note Division was clobbered by a free-running Program");
        }

        beginTest ("SAVE lights for an active-path move, and NOT for a persisting one");
        {
            ScopedTestDirectory dir { "dirty" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();   // 01 YOU TOO? - synced, BBD
            expect (! manager.isModifiedFromLoadedProgram(), "a freshly loaded Program is clean");

            // Not Program state: none of these may mark it dirty.
            host.setPlain (ParamIDs::crossFeed, 12.0f);
            expect (! manager.isModifiedFromLoadedProgram(), "Cross-Feed marked the Program dirty");

            host.setPlain (ParamIDs::timeMs, 999.0f);
            expect (! manager.isModifiedFromLoadedProgram(),
                    "Time marked a SYNCED Program dirty");

            host.setPlain (ParamIDs::wow, 90.0f);
            host.setPlain (ParamIDs::degrade, 90.0f);
            expect (! manager.isModifiedFromLoadedProgram(),
                    "another mode's dial marked a BBD Program dirty");

            // On the active path: this must.
            host.setPlain (ParamIDs::feedback, 88.0f);
            expect (manager.isModifiedFromLoadedProgram(), "Feedback did not mark the Program dirty");
        }

        beginTest ("a user Program round-trips only its own active path");
        {
            ScopedTestDirectory dir { "roundtrip" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();
            manager.requestProgramChange (3);        // 04 GREAT GIG - free-running, Tape
            manager.flushPendingChange();

            host.setPlain (ParamIDs::wow, 11.0f);
            host.setPlain (ParamIDs::timeMs, 321.0f);
            manager.saveNewUserProgram ("MY TAKE");

            const int saved = manager.getCurrentProgram();
            expect (! ProgramManager::isFactoryProgram (saved));

            // Move everything, including the things the Program does not own.
            host.setPlain (ParamIDs::wow, 99.0f);
            host.setPlain (ParamIDs::timeMs, 111.0f);
            host.setPlain (ParamIDs::crossFeed, 5.0f);
            host.setPlain (ParamIDs::degrade, 95.0f);

            manager.requestProgramChange (saved);
            manager.flushPendingChange();

            expectWithinAbsoluteError (host.plain (ParamIDs::wow), 11.0f, 0.1f, "user Program did not restore Wow");
            expectWithinAbsoluteError (host.plain (ParamIDs::timeMs), 321.0f, 1.0f, "user Program did not restore Time");
            expectWithinAbsoluteError (host.plain (ParamIDs::crossFeed), 5.0f, 0.1f, "user Program clobbered Cross-Feed");
            expectWithinAbsoluteError (host.plain (ParamIDs::degrade), 95.0f, 0.1f, "user Program clobbered Digital degrade");
        }

        beginTest ("Save always creates a new Program, even under a colliding name");
        {
            ScopedTestDirectory dir { "collision" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();

            host.setPlain (ParamIDs::feedback, 20.0f);
            manager.saveNewUserProgram ("SAME NAME");
            const auto firstName = manager.getProgramName (manager.getCurrentProgram());

            host.setPlain (ParamIDs::feedback, 80.0f);
            manager.saveNewUserProgram ("SAME NAME");
            const auto secondName = manager.getProgramName (manager.getCurrentProgram());

            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms + 2);

            // Compared by NAME, not index: the list re-sorts on every save, so a new Program can
            // take an index an older one held. Index equality would prove nothing.
            expect (firstName != secondName, "the second save reused the first Program's name");

            bool foundOriginal = false;

            for (int i = kNumFactoryPrograms; i < manager.getNumPrograms(); ++i)
            {
                if (manager.getProgramName (i) != firstName)
                    continue;

                manager.requestProgramChange (i);
                manager.flushPendingChange();
                expectWithinAbsoluteError (host.plain (ParamIDs::feedback), 20.0f, 0.2f);
                foundOriginal = true;
            }

            expect (foundOriginal, "the first Program is gone - it was overwritten");
        }

        beginTest ("Delete no-ops on Factory and falls back when the loaded Program goes");
        {
            ScopedTestDirectory dir { "del" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();

            for (int i = 0; i < kNumFactoryPrograms; ++i)
                manager.deleteUserProgram (i);

            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms);

            manager.saveNewUserProgram ("DOOMED");
            const int index = manager.getCurrentProgram();

            manager.deleteUserProgram (index);
            manager.flushPendingChange();

            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms);
            expectEquals (manager.getCurrentProgram(), defaultFactoryProgramIndex);
        }

        beginTest ("the active path is exactly what it claims to be");
        {
            // ActivePath is the single definition Correction 1 rests on. If it drifts, everything
            // above still passes for the wrong reason.
            const auto syncedTape = ActivePath::forState (true, (int) DelayCharacter::tape);
            expect (contains (syncedTape, ParamIDs::noteDivision));
            expect (! contains (syncedTape, ParamIDs::timeMs));
            expect (contains (syncedTape, ParamIDs::wow));
            expect (! contains (syncedTape, ParamIDs::modRate));
            expect (! contains (syncedTape, ParamIDs::crossFeed), "Cross-Feed is never Program state");

            const auto freeDigital = ActivePath::forState (false, (int) DelayCharacter::digital);
            expect (contains (freeDigital, ParamIDs::timeMs));
            expect (! contains (freeDigital, ParamIDs::noteDivision));
            expect (contains (freeDigital, ParamIDs::degrade));
            expect (! contains (freeDigital, ParamIDs::wow));
            expect (! contains (freeDigital, ParamIDs::flutter));

            const auto bbd = ActivePath::forState (true, (int) DelayCharacter::bbd);
            expectEquals ((int) bbd.size(), 11, "8 always + 1 timing + 2 BBD");
        }
    }

private:
    static bool contains (const std::vector<const char*>& ids, const char* id)
    {
        for (const auto* candidate : ids)
            if (juce::String (candidate) == juce::String (id))
                return true;

        return false;
    }

    static int divisionOf (TestHostProcessor& host)
    {
        if (const auto* p = dynamic_cast<const juce::AudioParameterChoice*> (
                host.apvts.getParameter (ParamIDs::noteDivision)))
            return p->getIndex();

        return -1;
    }
};

static ProgramModelTests programModelTests;
