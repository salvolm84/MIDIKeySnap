cmake_minimum_required(VERSION 3.22)

project(MIDIKeySnap VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version" FORCE)

set(JUCE_DIR "/Users/salvolm/JUCE" CACHE PATH "Path to the JUCE repository")
add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(MIDIKeySnap
    COMPANY_NAME "SLM"
    BUNDLE_ID "com.slm.midikeysnap"
    IS_SYNTH FALSE
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE SLMm
    PLUGIN_CODE MkSn
    FORMATS VST3
    PRODUCT_NAME "MIDIKeySnap")

target_sources(MIDIKeySnap
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/ScaleLibrary.h)

target_compile_definitions(MIDIKeySnap
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MIDIKeySnap
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(MIDIKeySnap)
