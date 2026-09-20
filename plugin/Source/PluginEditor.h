#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class Km60LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Km60LookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
};

class Km60LabAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit Km60LabAudioProcessorEditor(Km60LabAudioProcessor&);
    ~Km60LabAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Km60LabAudioProcessor& processorRef;
    Km60LookAndFeel km60LookAndFeel;

    juce::Slider inputGainKnob, trebleKnob, bassKnob, outputGainKnob;
    juce::Label inputGainLabel, trebleLabel, bassLabel, outputGainLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttach, trebleAttach,
                                       bassAttach, outputGainAttach;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Km60LabAudioProcessorEditor)
};
