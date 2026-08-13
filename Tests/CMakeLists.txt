juce_add_console_app(FifthMemberTests PRODUCT_NAME "FifthMemberTests")

target_sources(FifthMemberTests PRIVATE
    RealtimeSafetyTests.cpp
    TestMain.cpp
    ProgramModelTests.cpp
    PrintedScaleTests.cpp
    ReadoutConformanceTests.cpp
    MenuMetricsTests.cpp
    FactoryProgramsTests.cpp
    TimingEngineTests.cpp
    DelayCoreTests.cpp
    ../Source/DSP/TimingEngine.cpp
    ../Source/DSP/DelayCore.cpp
    ../Source/DSP/CharacterEngine.cpp
    ../Source/DSP/ProgramManager.cpp
    # **The REAL processor and the REAL editor, not stand-ins.** Everything above this line is what
    # core does not own at the DSP layer; everything below it is the wiring and the panel, which no
    # test could reach while this target compiled DSP alone.
    #
    # That gap was not theoretical. Reflect-84 shipped the stale-replay guard with ZERO call sites
    # for its disarm, and no suite could have seen it: the defect lived in an editor .cpp that was
    # not compiled into anything a test could run.
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/GUI/FifthMemberEditorContent.cpp
    ../Source/GUI/ProgramHeader.cpp
    ../Source/GUI/PanelControls.cpp
    ../Source/GUI/PanelBackground.cpp
    ../Source/GUI/FifthMemberKnob.cpp
    ../Source/GUI/RepeatTimelineScope.cpp)

# NF_COMPANY_NAME/NF_PRODUCT_NAME come from the parent CMakeLists and must match what the plugin
# target gets - ProgramManager.cpp is compiled into both and builds the user-Programs path from
# them. Inherited from the parent scope rather than restated, so there is nothing to re-sync.
target_compile_definitions(FifthMemberTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NF_COMPANY_NAME="${FIFTHMEMBER_COMPANY_NAME}"
    NF_PRODUCT_NAME="${FIFTHMEMBER_PRODUCT_NAME}")

# FifthMemberBinaryData is linked so the tests can include FifthMemberTheme.h, which pulls in
# BinaryData.h for the embedded fonts. PrintedScaleTests asserts the mark tables that live there -
# the alternative was a second copy of every mark angle in the test, which is exactly the drift the
# tables exist to prevent.
target_link_libraries(FifthMemberTests PRIVATE
    # The bug sweep's measurement drivers. TEST target only — never the plugin, which must not
    # carry an operator new override. See nf/testing/ProcessorHarness.h.
    nf::testing
    FifthMemberBinaryData
    # ProgramManager.cpp is compiled into both targets and calls into core, so both link it.
    nf::core
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
