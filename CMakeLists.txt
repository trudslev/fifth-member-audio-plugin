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
        design/plate/fifth-member-plate-2x.png
        design/fonts/barlow-condensed/BarlowCondensed-Medium.ttf
        design/fonts/barlow-condensed/BarlowCondensed-SemiBold.ttf
        design/fonts/barlow-condensed/BarlowCondensed-Bold.ttf
        design/fonts/share-tech-mono/ShareTechMono-Regular.ttf
        design/fonts/permanent-marker/PermanentMarker-Regular.ttf
        design/fonts/special-elite/SpecialElite-Regular.ttf)

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
    NF_VERSION_SHORT="${FIFTHMEMBER_VERSION_SHORT}")

target_link_libraries(FifthMember
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        FifthMemberBinaryData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_subdirectory(Tests)
