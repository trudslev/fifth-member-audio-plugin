cmake_minimum_required(VERSION 3.24)
# LANGUAGES needs C as well as CXX: JUCE and the VST3 SDK bundle some .c sources, and Xcode/Visual
# Studio tolerate that without an explicit enable_language(C), but Linux's single-config
# Makefiles/Ninja generator fails at CMake's generate step with "CMAKE_C_COMPILE_OBJECT not set".
# These MUST come before project(): project() creates these cache entries itself, after which a
# plain set(... CACHE ...) is a silent no-op. Sitting below project() they never take effect and
# builds quietly inherit the host architecture - which only surfaces when a CI-published .pkg turns
# out to contain an arm64-only binary.
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal: Apple Silicon + Intel")
endif()

project(FifthMember VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.14
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------------------------
# neon-foundry-core: shared BEHAVIOUR across the suite, pinned by tag exactly like JUCE above.
#
# **This is not a dependency on a sibling casting.** Depending on one of those is still forbidden -
# it couples two things that must ship on their own schedules. This is a versioned library that no
# casting owns, so a change to it forces nothing on anyone until they move their own pin, which is
# the same relationship every casting already has with JUCE. The root CLAUDE.md draws that
# distinction under "Structure".
#
# **Declared AFTER JUCE, and that ordering is load-bearing.** Core links juce::juce_core and does
# not fetch its own when consumed; it fails with an explicit message if JUCE is not already
# available. Two JUCE trees in one build link two juce_core builds into one binary and surface as
# duplicate symbols a long way from the cause.
#
FetchContent_Declare(NeonFoundryCore
    GIT_REPOSITORY https://github.com/trudslev/neon-foundry-core.git
    GIT_TAG v1.0.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(NeonFoundryCore)

# The panel is entirely vector/code-drawn, so the only runtime assets are typefaces. The design
# bundle ships no bitmaps at all - unlike Gatecrasher and CHORUS-60, whose designers supplied panel
# plates and 128-frame knob filmstrips - so there is nothing to embed but the fonts.
#
# design/README.md's type rule is "three voices, no exceptions": Barlow Condensed for anything
# silk-screened onto the panel, Share Tech Mono for anything displaying a variable value, and
# Permanent Marker / Special Elite for anything a human applied with a marker or a stencil.
#
# NOTE on licensing: design/README.md lists all four as OFL. Two of them are not - Permanent Marker
# and Special Elite are Apache 2.0, and are vendored with LICENSE.txt rather than OFL.txt
# accordingly. Both licences permit embedding; the attribution requirements differ.
#
# NOTE on JUCE's name mangling: it STRIPS non-alphanumeric characters rather than converting them to
# underscores, so BarlowCondensed-SemiBold.ttf becomes BinaryData::BarlowCondensedSemiBold_ttf.
# Check the generated BinaryData.h before assuming a symbol name.
# The plate, delivered with the revision-2 handoff. 2x only, per BRAND.md: a 1:1 blit to a Retina
# display resolves the fascia's 3px brush and the ears' 2px one to a flat wash and the metal stops
# reading as metal - the same reason PanelBackground used to bake at 2x. The 1x and 3x exports stay
# in design/plate/ unembedded.
juce_add_binary_data(FifthMemberBinaryData
    SOURCES
        # **The @3x plate, and it is 4020 x 3036 = 1340 x 1012 — the FRAME, not the window.**
        # The rack ears are chassis drawn at runtime either side of it, which is what lets §1 state
        # call 1 as the frame while this casting's window is 1444. Blitted at (52, 0); a blit at
        # (0, 0) would put the whole panel under the left ear.
        #
        # It replaces the 2x cut at 2480 x 1862 = 1240 x 931, which was the pre-harmonisation canvas.
        design/plate/fifth-member-plate-3x.png
        design/fonts/barlow-condensed/BarlowCondensed-Medium.ttf
        design/fonts/barlow-condensed/BarlowCondensed-SemiBold.ttf
        design/fonts/share-tech-mono/ShareTechMono-Regular.ttf
        design/fonts/permanent-marker/PermanentMarker-Regular.ttf
        # ABOUT-PART.md §2b's cursor. JUCE has no help cursor in StandardCursorType, so the
        # delivered asset is embedded and a MouseCursor is built from it.
        design/assets/about-cursor-2x.png)

# AU is Apple-only; Windows/Linux only ever build VST3 + Standalone. VST3_COPY_DIR/AU_COPY_DIR are
# only set on Apple - leaving them unset elsewhere lets JUCE apply its own correct per-OS default
# rather than a macOS path built on a non-guaranteed $ENV{HOME}.
if(APPLE)
    set(FIFTHMEMBER_FORMATS AU VST3 Standalone)
    set(FIFTHMEMBER_EXTRA_ARGS
        VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
        AU_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
else()
    set(FIFTHMEMBER_FORMATS VST3 Standalone)
    set(FIFTHMEMBER_EXTRA_ARGS "")
endif()

# Single source of truth for the two identity strings that appear BOTH in the bundle metadata and in
# ProgramManager's user-Programs directory path. ProgramManager.cpp cannot read JUCE's JucePlugin_*
# macros - they only exist in the plugin target's generated header, and that file is also compiled
# into the Tests console app. CHORUS-60's CMakeLists records what a hand-synced copy costs: it
# drifted from the real company name and quietly pointed saved Programs at a dead directory.
set(FIFTHMEMBER_COMPANY_NAME "Neon Foundry")
set(FIFTHMEMBER_PRODUCT_NAME "Fifth Member")
set(FIFTHMEMBER_VERSION_SHORT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# The CMake target name and the product name are two separate axes: a CMake target cannot contain a
# space, so the target is FifthMember (and the build directory FifthMember_artefacts/) while every
# bundle on disk is "Fifth Member.vst3". neon-foundry/manifest/suite.toml carries the same
# distinction as cmake_target vs product_name; anything deriving a path from the wrong one silently
# looks in a directory that does not exist.
juce_add_plugin(FifthMember
    PRODUCT_NAME "${FIFTHMEMBER_PRODUCT_NAME}"
    COMPANY_NAME "${FIFTHMEMBER_COMPANY_NAME}"
    BUNDLE_ID "com.neonfoundry.fifthmember"
    # Without this the Standalone cannot get audio INPUT on macOS, and fails silently doing it.
    # An app with no NSMicrophoneUsageDescription is never granted capture access - the OS does not
    # prompt and does not error, the input stream simply delivers zeros. On an effect that reads as
    # dead metering: IN sits at its floor and OUT follows, because there is nothing to process. The
    # meters are wired correctly; they have nothing to show.
    #
    # Plugin formats are unaffected - the host owns the input there - so this only shows up when
    # someone opens the Standalone to listen.
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "FIFTH MEMBER needs audio input so the Standalone can process what you play into it."
    PLUGIN_MANUFACTURER_CODE Nfdy
    PLUGIN_CODE Fm88
    FORMATS ${FIFTHMEMBER_FORMATS}
    VST3_CATEGORIES Fx Delay
    AU_MAIN_TYPE kAudioUnitType_Effect
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    # JUCE takes exactly two images and derives every platform icon from them. The macOS .icns gets
    # one entry per file, at the largest of 16/32/64/128/256/512/1024 that fits - so these must be
    # EXACTLY square at an exact valid size, or a 1000px source silently lands in the icns as 512.
    # The Windows .ico takes 16/32/48/256, each from the smallest supplied image still big enough,
    # which means all four come from the 256. 1024 + 256 matches all four siblings.
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/design/icons/fifth-member-icon-1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/design/icons/fifth-member-icon-256.png"
    ${FIFTHMEMBER_EXTRA_ARGS})

target_sources(FifthMember PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/TimingEngine.cpp
    Source/DSP/DelayCore.cpp
    Source/DSP/CharacterEngine.cpp
    Source/DSP/ProgramManager.cpp
    Source/GUI/FifthMemberKnob.cpp
    Source/GUI/PanelBackground.cpp
    Source/GUI/PanelControls.cpp
    Source/GUI/RepeatTimelineScope.cpp
    Source/GUI/ProgramHeader.cpp
    Source/GUI/FifthMemberEditorContent.cpp)

target_compile_definitions(FifthMember PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    NF_COMPANY_NAME="${FIFTHMEMBER_COMPANY_NAME}"
    NF_PRODUCT_NAME="${FIFTHMEMBER_PRODUCT_NAME}"
    NF_VERSION_SHORT="${FIFTHMEMBER_VERSION_SHORT}"
    # §12's stamp is `v<major.minor.patch>`, semver per ABOUT-PART.md §1 - a THIRD field the
    # major.minor short form cannot carry. Both are defined because they are different strings for
    # different places, not two spellings of one.
    NF_VERSION="${PROJECT_VERSION}")

target_link_libraries(FifthMember
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        FifthMemberBinaryData
        # ProgramManager.cpp is compiled into both this target and the tests, and calls into core,
        # so both link it.
        nf::core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_subdirectory(Tests)

# ---------------------------------------------------------------------------------------------
# THIRD-PARTY-LICENCES.txt — into every bundle's Contents/Resources
#
# **Embedding a font in the binary is redistribution**, and both licences these faces carry — OFL
# 1.1 and Apache 2.0 — require the copyright notice and licence text to travel with what is
# redistributed. The unit of redistribution is the PLUG-IN BUNDLE: someone receives a .vst3 on its
# own, so a licence file at the repo root discharges nothing for them. It has to be inside.
#
# The file is GENERATED from this CMakeLists' own juce_add_binary_data block by
# tools/generate_third_party_licences.py, so it cannot drift from what actually ships. Regenerate
# after adding or removing an embedded face; `--check` fails if any casting is stale.
#
# **macOS only, stated rather than silently true.** MACOSX_PACKAGE_LOCATION is what puts a file in
# a bundle, and on Windows and Linux the licence has to reach the user through the installer
# instead — see installer/ . Adding the .txt to target_sources off Apple would have CMake try to
# build it, which is why this is guarded rather than merely inert.
# **The path and its guard sit OUTSIDE the platform branch, and that is load-bearing.** Both used
# to live inside `if(APPLE)`, where the `set()` only executes on Apple — so every Windows and Linux
# build dereferenced an undefined variable and handed `copy_if_different` an empty source argument,
# and the missing-file FATAL_ERROR could not fire there either.
#
# It survived because nothing had ever built off Apple: all six castings' CI is macOS-first, three
# had never run at all, and a defect on a path nobody walks leaves no trace. Found while planning
# the release pipeline, proved with a minimal CMake reproduction rather than by reading.
set(NF_LICENCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/THIRD-PARTY-LICENCES.txt")
if(NOT EXISTS "${NF_LICENCE_FILE}")
    message(FATAL_ERROR
        "THIRD-PARTY-LICENCES.txt is missing. Run: python3 tools/generate_third_party_licences.py")
endif()

if(APPLE)
    set_source_files_properties("${NF_LICENCE_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    foreach(_nf_fmt IN ITEMS AU AUv3 VST3 Standalone)
        if(TARGET FifthMember_${_nf_fmt})
            target_sources(FifthMember_${_nf_fmt} PRIVATE "${NF_LICENCE_FILE}")
        endif()
    endforeach()
else()
    # **Windows and Linux: the VST3 is a bundle DIRECTORY too**, and it is the format that ships
    # there, so the licence belongs inside it for the same reason it does on macOS - the .vst3 is
    # what someone receives on its own. JUCE puts the binary at <bundle>/Contents/<arch>, so
    # Contents/Resources is one level up from TARGET_FILE_DIR.
    #
    # The Standalone off Apple is a bare executable with no bundle to put anything in; it reaches
    # the user through the installer or the tarball instead.
    if(TARGET FifthMember_VST3)
        add_custom_command(TARGET FifthMember_VST3 POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                    "$<TARGET_FILE_DIR:FifthMember_VST3>/../Resources"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${NF_LICENCE_FILE}"
                    "$<TARGET_FILE_DIR:FifthMember_VST3>/../Resources/THIRD-PARTY-LICENCES.txt"
            VERBATIM)
    endif()
endif()
