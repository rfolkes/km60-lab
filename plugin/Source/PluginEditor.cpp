#include "PluginEditor.h"

Km60LabAudioProcessorEditor::Km60LabAudioProcessorEditor(Km60LabAudioProcessor& p)
    : AudioProcessorEditor(&p), genericEditor(p)
{
    addAndMakeVisible(genericEditor);
    setSize(400, 280);
}

void Km60LabAudioProcessorEditor::resized()
{
    genericEditor.setBounds(getLocalBounds());
}
