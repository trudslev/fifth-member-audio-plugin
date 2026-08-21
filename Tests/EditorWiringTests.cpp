#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/GUI/FifthMemberTheme.h"

#include <nf/HeaderPart.h>
#include <nf/UserProgramDirectory.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    The first tests in this casting that run against the REAL editor.

    **Why this file exists, and why it did not until now.** Reflect-84 shipped the stale-replay
    guard with **zero call sites** for its disarm. The guard was correct, the processor was correct,
    and every suite passed — because the defect was in the editor, which no target a test could run
    compiled. The harness port gave all six castings the ability to construct their shipping editor;
    a port with nothing using it is just a longer build.

    **It was deliberately deferred for five castings until 2026-08-20**, and the deferral is spent:
    a test written the day before its GUI is rebuilt is a test written against the wrong thing, and
    all six editors are now rebuilt. Reflect-84's was the exception because its editor was already
    conformant, and it is the model this file follows.

    **What can honestly be asserted here, and what cannot.** A knob takes the LCD over, and disarms
    the guard, only while it is GENUINELY being dragged — `nf::connectUserEdit` guards on
    `isMouseButtonDown()`, and that state lives in the mouse source rather than in the component. A
    headless test cannot fake it: there is no windowed peer for a synthetic event to arrive through.
    So "a user edit disarms the guard" is **not available from here**, and asserting it would be a
    claim of coverage rather than coverage — which this suite records as worse than no test at all.
    `tools/check_user_edit_wiring.py` covers the call-site question statically and
    `nf::UserEditGate`'s own tests cover the mechanism.

    What IS available is the other half, and it has a rejected design behind it: **with a real
    editor attached and its attachments firing, a host's parameter writes must not disarm the
    guard.** That is precisely what a `ValueTree::Listener` inside core would have broken — the
    extraction plan specified one and it was rejected, because a listener cannot tell a person from
    an automation lane, and a host may write automation on session load before replaying its
    remembered program index. This file is the regression test for that decision.
*/
class EditorWiringTests final : public juce::UnitTest
{
public:
    EditorWiringTests() : juce::UnitTest ("Editor wiring", "GUI") {}

    void runTest() override
    {

        beginTest ("The WINDOW is wider than the frame, and the frame is the shared part's");
        {
            /*  **This arm has inverted, and the note it replaces described a panel that no longer
                exists.** It read: *"Absent by construction, not clean. Fifth Member does not
                reference `nf::HeaderGeometry` anywhere: it is 1240 x 931 and its band is its
                own... It becomes possible the moment this panel is moved onto the shared part."*

                That moment is this commit.

                **This is the one casting whose window is wider than the shared frame**, and §1 is
                explicit that call 1 is stated as the FRAME for exactly this reason: full-height
                rack ears flank the fascia, 52 px each side, and they carry three of the panel's
                five identity marks. A 1308-wide block at x 16 inside a 1340 window leaves zero for
                ears. **Named cost: this window is 104 px wider than the other five.**

                So there are two width claims here and they are different claims. The frame is
                core's and must equal it; the window is this casting's and is the frame plus two
                ears. Asserting only the window would let the frame drift; asserting only the frame
                would not notice an ear going missing. */
            namespace L = FifthMemberTheme::Layout;

            expectEquals ((int) L::frameWidth, nf::HeaderGeometry::canvasWidth);
            expectEquals ((int) L::canvasWidth, 1444);
            expectEquals ((int) L::canvasHeight, 1012);

            expectEquals ((int) (L::frameWidth + L::earWidth * 2.0f), (int) L::canvasWidth,
                          "the window is the frame plus two ears, and one of the three has moved");
            expectEquals ((int) L::frameX, (int) L::earWidth,
                          "the frame starts where the left ear ends");
        }

        beginTest ("Every header cell is core's, and the nameplate lands on the shared anchor");
        {
            /*  A literal that happens to agree with core is indistinguishable from an alias by
                READING, which is how Chorus-60's header pass left SAVE, DELETE and both meter wells
                29 px right and 29 px down of where they belonged — invisible for as long as the
                plate baked their faces. Every cell is compared, and each is offset by `frameX`
                because `nf::HeaderGeometry` speaks frame-local.

                What it cannot do is prove provenance: a derivation and a literal agree until the
                shared figure moves. This catches **divergence**, which is the only window in which
                the two are different at all. */
            namespace L = FifthMemberTheme::Layout;
            using H = nf::HeaderGeometry;      // a struct of constants, not a namespace

            expectEquals ((int) L::lcdX,      (int) L::frameX + H::lcdX);
            expectEquals ((int) L::lcdW,      H::lcdW);
            expectEquals ((int) L::headerRowY, H::bandY);
            expectEquals ((int) L::headerRowH, H::bandH);
            expectEquals ((int) L::saveX,     (int) L::frameX + H::saveX);
            expectEquals ((int) L::saveW,     H::saveW);
            expectEquals ((int) L::deleteX,   (int) L::frameX + H::deleteX);
            expectEquals ((int) L::deleteW,   H::deleteW);
            expectEquals ((int) L::meterInX,  (int) L::frameX + H::inWellX);
            expectEquals ((int) L::meterOutX, (int) L::frameX + H::outWellX);
            expectEquals ((int) L::meterBoxW, H::meterWellW);
            expectEquals ((int) L::nameplateX, (int) L::frameX + H::nameplateX);
            expectEquals ((int) L::nameplateY, H::nameplateY);

            /*  §4's shared descriptor anchor, and this casting's row of
                `design-asks/header-nameplate-offsets.md` answering itself the way TapeRot's did.
                Core records the PUBLISHED stack as `30 + 34 + 9 = 73`, five short. The delivered
                prototype draws the tape's bounding box at 268.8 x 45.6 rotated -1.2 deg, which is a
                **268.2 x 40** strip — and 30 + 40 + 8 is 78 exactly. The published height was
                measured on something other than the tape. */
            expectEquals ((int) L::taglineY1, H::descriptorY);
            expect (nf::HeaderGeometry::landsOnDescriptorAnchor ((int) L::nameplateY,
                                                                 (int) L::nameplateH, 8),
                    "the nameplate stack no longer lands the descriptor on the shared anchor");
        }

        beginTest ("The real editor constructs, lays out and tears down");
        {
            /*  Worth its own case even though it asserts little: until the harness port, nothing
                here executed a line of the editor, so a null dereference or a failed assertion in
                layout would have been found by opening the plugin in a DAW.

                This constructs the SHIPPING processor, which builds its ProgramManager from the
                resolved user-Programs path — it has no injectable override, only ProgramManager
                does. **That path is redirected process-wide** by the
                `nf::ScopedUserProgramDirectoryOverride` in `TestMain`, so nothing here reaches real
                Programs; the case below asserts that is true in this binary rather than trusting
                it. */
            FifthMemberAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor> (processor.createEditor());

            expect (editor != nullptr, "createEditor returned nothing");

            if (editor != nullptr)
            {
                expectGreaterThan (editor->getWidth(), 0);
                expectGreaterThan (editor->getHeight(), 0);
            }
        }

        beginTest ("A host's parameter writes do not disarm the stale-replay guard");
        {
            FifthMemberAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor> (processor.createEditor());
            expect (editor != nullptr);

            processor.userEdits.armRestore();

            // Every attachment in the editor fires for these, exactly as it does when a host
            // replays automation on session load. None of them is a drag.
            int written = 0;

            for (auto* parameter : processor.getParameters())
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameter))
                {
                    const auto original = ranged->getValue();
                    ranged->setValueNotifyingHost (original < 0.5f ? 0.9f : 0.1f);
                    ++written;
                }
            }

            // **The vacuity guard.** An empty parameter list would make the assertion below pass
            // while proving nothing, which is the shape this suite has three recorded cases of.
            expectGreaterThan (written, 0, "no parameters were written, so this asserted nothing");

            expect (processor.userEdits.isRestorePending(),
                    "automation disarmed the stale-replay guard. A ValueTree listener would do "
                    "exactly this - see nf/UserEditGate.h for why the shared model does not use "
                    "one, and do not re-introduce it");
        }
    }
};

static EditorWiringTests editorWiringTests;

/** Proves the process-wide redirect is in force in THIS binary, rather than merely installed in a
    file somebody could delete.

    `run_tests.py` refuses a target whose `TestMain` does not install it, and core's own tests prove
    the mechanism redirects. **Neither establishes that this process is redirected**, which is the
    thing keeping a suite off the user's disk — and the exposure arrived as a side effect of
    unrelated work, since the harness port gave every casting the ability to construct a processor
    that resolves the real path because that is its job.
*/
class ProgramDirectoryRedirectTests final : public juce::UnitTest
{
public:
    ProgramDirectoryRedirectTests() : juce::UnitTest ("Program directory redirect", "programs") {}

    void runTest() override
    {
        beginTest ("The shipping processor cannot reach the user's real Programs directory");
        {
            expect (nf::userProgramDirectoryOverrideRoot() != juce::File(),
                    "no redirect is installed in this process - TestMain must install "
                    "nf::ScopedUserProgramDirectoryOverride before the runner");

            FifthMemberAudioProcessor processor;
            const auto used = processor.getProgramManager().getUserProgramDirectory();

            expect (used.isAChildOf (nf::userProgramDirectoryOverrideRoot()),
                    "the processor resolved " + used.getFullPathName()
                        + ", which is outside the redirect root");

            // Named explicitly rather than compared against a rebuilt "real" path: the point is
            // that the application-data root is not on this path at all.
            const auto appData =
                juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

            expect (! used.isAChildOf (appData),
                    "the processor is pointing inside the user's application data");
        }
    }
};

static ProgramDirectoryRedirectTests programDirectoryRedirectTests;
