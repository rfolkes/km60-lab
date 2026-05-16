#pragma once

#include <JuceHeader.h>
#include "dsp/Km60Processor.h"

class Km60LabAudioProcessor final : public juce::AudioProcessor
{
public:
    Km60LabAudioProcessor();
    ~Km60LabAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    Km60Processor km60;

    // Gain parameters are smoothed per-sample to prevent zipper noise on fader moves.
    // EQ parameters are not smoothed — biquad coefficients update once per block,
    // which is sufficient since shelf knobs are not typically automated at audio rate.
    juce::SmoothedValue<float> smoothInputGain;
    juce::SmoothedValue<float> smoothOutputGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Km60LabAudioProcessor)
};
