juce_add_console_app(FifthMemberTests PRODUCT_NAME "FifthMemberTests")

target_sources(FifthMemberTests PRIVATE
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
    ../Source/DSP/ProgramManager.cpp)

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
