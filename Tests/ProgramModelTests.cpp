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
        beginTest ("Cross-Feed is Program state in Ping-Pong and persists in the other two modes");
        {
            ScopedTestDirectory dir { "crossfeed" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();
            host.setPlain (ParamIDs::crossFeed, 17.5f);

            // Walk the whole factory bank. A Ping-Pong Program must impose its own cross - this is
            // the reproducibility case: at cross 0 that mode leaves the right line silent, so a
            // Program that inherited whatever the knob happened to be at could recall as a
            // one-sided delay. Every other Program must leave the knob exactly where it was.
            for (int i = 0; i < kNumFactoryPrograms; ++i)
            {
                const auto& program = kFactoryPrograms[(size_t) i];
                const auto name = manager.getProgramName (i);

                host.setPlain (ParamIDs::crossFeed, 17.5f);
                manager.requestProgramChange (ProgramManager::factoryIdAt (i));
                manager.flushPendingChange();

                if ((StereoMode) program.stereoMode == StereoMode::pingPong)
                    expectWithinAbsoluteError (host.plain (ParamIDs::crossFeed), program.crossFeedPercent,
                                               0.05f, name + ": a Ping-Pong Program did not restore its cross");
                else
                    expectWithinAbsoluteError (host.plain (ParamIDs::crossFeed), 17.5f, 0.05f,
                                               name + ": a non-Ping-Pong Program moved Cross-Feed");
            }
        }

        beginTest ("moving Cross-Feed dirties a Ping-Pong Program but not a Stereo one");
        {
            ScopedTestDirectory dir { "crossfeeddirty" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();

            const auto loadModeAndNudgeCross = [&] (StereoMode mode)
            {
                // Pick the first factory Program in the wanted mode so the snapshot is a real one.
                for (int i = 0; i < kNumFactoryPrograms; ++i)
                {
                    if ((StereoMode) kFactoryPrograms[(size_t) i].stereoMode != mode)
                        continue;

                    manager.requestProgramChange (ProgramManager::factoryIdAt (i));
                    manager.flushPendingChange();
                    break;
                }

                expect (! manager.isModifiedFromLoadedProgram(), "a freshly loaded Program is dirty");
                host.setPlain (ParamIDs::crossFeed, host.plain (ParamIDs::crossFeed) > 50.0f ? 20.0f : 90.0f);
                return manager.isModifiedFromLoadedProgram();
            };

            expect (loadModeAndNudgeCross (StereoMode::pingPong),
                    "Cross-Feed did not light SAVE in Ping-Pong, where it is the sound");
            expect (! loadModeAndNudgeCross (StereoMode::stereo),
                    "Cross-Feed lit SAVE in Stereo, where DelayCore never reads it");
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
            manager.requestProgramChange (ProgramManager::factoryIdAt (3));
            manager.flushPendingChange();

            expectWithinAbsoluteError (host.plain (ParamIDs::wow), 30.0f, 0.05f, "Tape wow was not applied");
            expectWithinAbsoluteError (host.plain (ParamIDs::modRate), 3.3f, 0.02f, "BBD rate was clobbered");
            expectWithinAbsoluteError (host.plain (ParamIDs::modDepth), 44.0f, 0.05f, "BBD depth was clobbered");
            expectWithinAbsoluteError (host.plain (ParamIDs::degrade), 22.0f, 0.05f, "Digital degrade was clobbered");

            // 08 SLAP HAPPY is Digital: it writes Degrade and leaves the Tape dials alone.
            manager.requestProgramChange (ProgramManager::factoryIdAt (7));
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
            manager.requestProgramChange (ProgramManager::factoryIdAt (0));
            manager.flushPendingChange();

            expectEquals (divisionOf (host), (int) NoteDivision::dottedEighth, "division was not applied");
            expectWithinAbsoluteError (host.plain (ParamIDs::timeMs), 1234.0f, 1.0f, "Time was clobbered by a synced Program");

            // 06 LONG LOOP is free-running: it writes Time and must not touch Note Division.
            manager.requestProgramChange (ProgramManager::factoryIdAt (5));
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

            manager.initialise();   // 01 YOU TOO? - synced, BBD, Ping-Pong
            expect (! manager.isModifiedFromLoadedProgram(), "a freshly loaded Program is clean");

            // Not Program state: none of these may mark it dirty.
            host.setPlain (ParamIDs::timeMs, 999.0f);
            expect (! manager.isModifiedFromLoadedProgram(),
                    "Time marked a SYNCED Program dirty");

            host.setPlain (ParamIDs::wow, 90.0f);
            host.setPlain (ParamIDs::degrade, 90.0f);
            expect (! manager.isModifiedFromLoadedProgram(),
                    "another mode's dial marked a BBD Program dirty");

            // On the active path: these must. Cross-Feed is here because YOU TOO? is Ping-Pong -
            // in a Mono or Stereo Program the same move is a persisting one and stays clean, which
            // the mode-by-mode test above covers.
            host.setPlain (ParamIDs::crossFeed, 12.0f);
            expect (manager.isModifiedFromLoadedProgram(),
                    "Cross-Feed did not mark a Ping-Pong Program dirty");

            host.setPlain (ParamIDs::feedback, 88.0f);
            expect (manager.isModifiedFromLoadedProgram(), "Feedback did not mark the Program dirty");
        }

        beginTest ("a user Program round-trips only its own active path");
        {
            ScopedTestDirectory dir { "roundtrip" };
            TestHostProcessor host;
            ProgramManager manager { host.apvts, dir.directory };

            manager.initialise();
            manager.requestProgramChange (ProgramManager::factoryIdAt (3));        // 04 GREAT GIG - free-running, Tape
            manager.flushPendingChange();

            host.setPlain (ParamIDs::wow, 11.0f);
            host.setPlain (ParamIDs::timeMs, 321.0f);
            manager.saveNewUserProgram ("MY TAKE");

            const auto saved = manager.getCurrentProgramId();
            expect (saved.bank == ProgramBank::user);

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
            const auto firstId = manager.getCurrentProgramId();

            host.setPlain (ParamIDs::feedback, 80.0f);
            manager.saveNewUserProgram ("SAME NAME");
            const auto secondId = manager.getCurrentProgramId();

            // **The host list must NOT have grown** - that is the juce_AudioProcessor.h contract
            // this change exists to honour. The two saved Programs are counted from listPrograms.
            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms);

            int userCount = 0;

            for (const auto& id : manager.listPrograms())
                if (id.bank == ProgramBank::user)
                    ++userCount;

            expectEquals (userCount, 2);

            // Compared by IDENTITY, which is now the only thing they could be compared by - and
            // which is the point: the list re-sorts on every save, so a position proves nothing.
            expect (firstId != secondId, "the second save reused the first Program's identity");

            bool foundOriginal = false;

            for (const auto& id : manager.listPrograms())
            {
                if (id != firstId)
                    continue;

                manager.requestProgramChange (id);
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

            // Every Factory Program, plus INIT. The gate is on the BANK now, which is stronger
            // than the old index range: an id from any other bank cannot address a file.
            for (const auto& id : manager.listPrograms())
                manager.deleteUserProgram (id);

            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms);

            manager.saveNewUserProgram ("DOOMED");
            const auto doomed = manager.getCurrentProgramId();

            manager.deleteUserProgram (doomed);
            manager.flushPendingChange();

            expectEquals (manager.getNumPrograms(), kNumFactoryPrograms);
            expect (manager.getCurrentProgramId()
                        == ProgramManager::factoryIdAt (defaultFactoryProgramIndex));
        }

        beginTest ("the active path is exactly what it claims to be");
        {
            // ActivePath is the single definition Correction 1 rests on. If it drifts, everything
            // above still passes for the wrong reason.
            const auto syncedTape = ActivePath::forState (true, (int) DelayCharacter::tape,
                                                          (int) StereoMode::stereo);
            expect (contains (syncedTape, ParamIDs::noteDivision));
            expect (! contains (syncedTape, ParamIDs::timeMs));
            expect (contains (syncedTape, ParamIDs::wow));
            expect (! contains (syncedTape, ParamIDs::modRate));
            expect (! contains (syncedTape, ParamIDs::crossFeed),
                    "Cross-Feed is not Program state outside Ping-Pong");

            const auto freeDigital = ActivePath::forState (false, (int) DelayCharacter::digital,
                                                           (int) StereoMode::mono);
            expect (contains (freeDigital, ParamIDs::timeMs));
            expect (! contains (freeDigital, ParamIDs::noteDivision));
            expect (contains (freeDigital, ParamIDs::degrade));
            expect (! contains (freeDigital, ParamIDs::wow));
            expect (! contains (freeDigital, ParamIDs::flutter));
            expect (! contains (freeDigital, ParamIDs::crossFeed), "Mono stored a Cross-Feed");

            // Ping-Pong is the one mode DelayCore reads the cross term in, so it is the one mode
            // that stores it.
            const auto pingPong = ActivePath::forState (true, (int) DelayCharacter::bbd,
                                                        (int) StereoMode::pingPong);
            expect (contains (pingPong, ParamIDs::crossFeed), "Ping-Pong did not store its Cross-Feed");
            expectEquals ((int) pingPong.size(), 12, "8 always + 1 timing + 2 BBD + Cross-Feed");

            const auto bbd = ActivePath::forState (true, (int) DelayCharacter::bbd,
                                                   (int) StereoMode::stereo);
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
