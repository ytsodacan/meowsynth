cmake_minimum_required(VERSION 3.22)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)  # Apple Silicon + Intel
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()
project(MeowSynth VERSION 1.0.0)

if(APPLE)
    set(MEOW_FORMATS VST3 AU Standalone)
else()
    set(MEOW_FORMATS ${MEOW_FORMATS})
endif()

# JUCE is fetched automatically on first configure
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(MeowSynth
    COMPANY_NAME "MeowLabs"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Meow
    PLUGIN_CODE Mwsy
    FORMATS ${MEOW_FORMATS}
    PRODUCT_NAME "MeowSynth")

file(GLOB FRAMES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/frames/*.png")
juce_add_binary_data(MeowData SOURCES
    Assets/boykisser.wav
    Assets/catmeme.wav
    ${FRAMES})
set_target_properties(MeowData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_sources(MeowSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(MeowSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MeowSynth
    PRIVATE MeowData juce::juce_audio_utils juce::juce_audio_formats
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
