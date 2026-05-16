#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Parameters ordered by signal flow: input → EQ → output.
    // Input Gain extended to +36 dB so the signal can be pushed into the HA-1457
    // saturation region, modelling preamp overload rather than a separate drive control.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGainDb", 1}, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 36.0f, 0.1f), 0.0f));

    // Treble shelf at ~1600 Hz, bass shelf at ~350 Hz — frequencies from KM-60 schematic.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"trebleDb", 1}, "Treble",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bassDb", 1}, "Bass",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGainDb", 1}, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    return {params.begin(), params.end()};
}

Km60LabAudioProcessor::Km60LabAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

void Km60LabAudioProcessor::prepareToPlay(double sampleRate, int)
{
    constexpr double rampSeconds = 0.02;

    smoothInputGain.reset(sampleRate, rampSeconds);
    smoothInputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGainDb")->load()));

    smoothOutputGain.reset(sampleRate, rampSeconds);
    smoothOutputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGainDb")->load()));

    km60.prepare(sampleRate);
    km60.updateCoefficients(
        apvts.getRawParameterValue("trebleDb")->load(),
        apvts.getRawParameterValue("bassDb")->load());
}

void Km60LabAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Gain parameters: smooth targets per sample to prevent zipper noise.
    smoothInputGain.setTargetValue(
        juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGainDb")->load()));
    smoothOutputGain.setTargetValue(
        juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGainDb")->load()));

    // EQ: recompute biquad coefficients once per block.
    km60.updateCoefficients(
        apvts.getRawParameterValue("trebleDb")->load(),
        apvts.getRawParameterValue("bassDb")->load());

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0)
        return;

    float* L = buffer.getWritePointer(0);
    float* R = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inGain  = smoothInputGain.getNextValue();
        const float outGain = smoothOutputGain.getNextValue();

        L[i] = km60.processSample(L[i], 0, inGain, outGain);
        if (R != nullptr)
            R[i] = km60.processSample(R[i], 1, inGain, outGain);
    }
}

juce::AudioProcessorEditor* Km60LabAudioProcessor::createEditor()
{
    return new Km60LabAudioProcessorEditor(*this);
}

void Km60LabAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Km60LabAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Km60LabAudioProcessor();
}
