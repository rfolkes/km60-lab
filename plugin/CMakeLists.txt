juce_add_plugin(KM60Lab
    VERSION                     0.1.0
    COMPANY_NAME                "KM60Lab"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    PLUGIN_MANUFACTURER_CODE    Km60
    PLUGIN_CODE                 Km6L
    FORMATS                     VST3
    PRODUCT_NAME                "KM60Lab"
)

juce_generate_juce_header(KM60Lab)

target_sources(KM60Lab
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/dsp/Km60Processor.cpp
)

target_include_directories(KM60Lab
    PRIVATE
        Source
)

target_compile_definitions(KM60Lab
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(KM60Lab
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
