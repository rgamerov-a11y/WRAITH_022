cmake_minimum_required(VERSION 3.22)
project(WRAITH VERSION 0.8.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

function(add_wraith target code synth product)
    juce_add_plugin(${target}
        COMPANY_NAME "WRAITH Audio"
        BUNDLE_ID "audio.wraith.${target}"
        PLUGIN_MANUFACTURER_CODE Wrth
        PLUGIN_CODE ${code}
        FORMATS VST3
        PRODUCT_NAME "${product}"
        IS_SYNTH ${synth}
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE)
    juce_generate_juce_header(${target})
    target_sources(${target} PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
    target_compile_definitions(${target} PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(${target} PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endfunction()
add_wraith(WRAITH_Instrument Wr08 TRUE "WRAITH Instrument")
add_wraith(WRAITH_FX Wf08 FALSE "WRAITH FX")
