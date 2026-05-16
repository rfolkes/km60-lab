#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class Km60LabAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit Km60LabAudioProcessorEditor(Km60LabAudioProcessor&);
    ~Km60LabAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    juce::GenericAudioProcessorEditor genericEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Km60LabAudioProcessorEditor)
};
