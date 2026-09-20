#include "PluginEditor.h"

namespace
{
    const auto colFrame   = juce::Colour(0xffA0A0A0);
    const auto colPanel   = juce::Colour(0xff2A2A2A);
    const auto colBrandBg = juce::Colour(0xff1A1A1A);
    const auto colGold    = juce::Colour(0xffD4952B);
    const auto colLabel   = juce::Colour(0xffD8D8D8);
    const auto colValue   = juce::Colour(0xff909090);
    const auto colTick    = juce::Colour(0xff707070);

    constexpr int kFramePad = 12;
    constexpr int kBrandH   = 52;
    constexpr int kLabelH   = 16;
    constexpr int kTextBoxH = 14;
}

// ===== Km60LookAndFeel =====

Km60LookAndFeel::Km60LookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, colValue);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void Km60LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                        float sliderPos, float startAngle, float endAngle,
                                        juce::Slider&)
{
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;
    const float outerR = juce::jmin((float)w, (float)h) * 0.5f;
    const float knobR  = outerR - 8.0f;
    const float angle  = startAngle + sliderPos * (endAngle - startAngle);

    // Tick dots around the knob
    {
        constexpr int n = 11;
        const float tickR = knobR + 5.0f;
        g.setColour(colTick);
        for (int i = 0; i < n; ++i)
        {
            float a = startAngle + (float)i / (float)(n - 1) * (endAngle - startAngle);
            g.fillEllipse(cx + tickR * std::sin(a) - 1.5f,
                          cy - tickR * std::cos(a) - 1.5f, 3.0f, 3.0f);
        }
    }

    // Shadow beneath knob
    g.setColour(juce::Colour(0xff151515));
    g.fillEllipse(cx - knobR - 0.5f, cy - knobR + 1.0f,
                  knobR * 2.0f + 1.0f, knobR * 2.0f + 1.0f);

    // Knob body — top-lit metallic gradient
    {
        juce::ColourGradient grad(juce::Colour(0xffD5D5D5), cx, cy - knobR,
                                   juce::Colour(0xff888888), cx, cy + knobR, false);
        g.setGradientFill(grad);
        g.fillEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);
    }

    // Rim
    g.setColour(juce::Colour(0xff585858));
    g.drawEllipse(cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, 1.2f);

    // Specular highlight (upper-left)
    {
        const float off = knobR * 0.3f;
        juce::ColourGradient hl(juce::Colour(0x38ffffff), cx - off, cy - off,
                                 juce::Colour(0x00ffffff), cx + off * 1.5f, cy + off * 1.5f, true);
        g.setGradientFill(hl);
        const float hlR = knobR * 0.82f;
        g.fillEllipse(cx - hlR, cy - hlR, hlR * 2.0f, hlR * 2.0f);
    }

    // Indicator line (dark notch)
    {
        const float r1 = knobR * 0.18f;
        const float r2 = knobR * 0.78f;
        g.setColour(juce::Colour(0xff1E1E1E));
        g.drawLine(cx + r1 * std::sin(angle), cy - r1 * std::cos(angle),
                   cx + r2 * std::sin(angle), cy - r2 * std::cos(angle), 2.5f);
    }

    // Center dot
    g.setColour(juce::Colour(0xff4A4A4A));
    g.fillEllipse(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
}

// ===== Editor =====

Km60LabAudioProcessorEditor::Km60LabAudioProcessorEditor(Km60LabAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&km60LookAndFeel);

    setupKnob(inputGainKnob,  inputGainLabel,  "INPUT GAIN");
    setupKnob(trebleKnob,     trebleLabel,     "TREBLE");
    setupKnob(bassKnob,       bassLabel,       "BASS");
    setupKnob(outputGainKnob, outputGainLabel, "OUTPUT GAIN");

    inputGainKnob.setTextValueSuffix(" dB");
    trebleKnob.setTextValueSuffix(" dB");
    bassKnob.setTextValueSuffix(" dB");
    outputGainKnob.setTextValueSuffix(" dB");

    inputGainAttach  = std::make_unique<SliderAttachment>(p.apvts, "inputGainDb",  inputGainKnob);
    trebleAttach     = std::make_unique<SliderAttachment>(p.apvts, "trebleDb",     trebleKnob);
    bassAttach       = std::make_unique<SliderAttachment>(p.apvts, "bassDb",       bassKnob);
    outputGainAttach = std::make_unique<SliderAttachment>(p.apvts, "outputGainDb", outputGainKnob);

    setSize(220, 520);
}

Km60LabAudioProcessorEditor::~Km60LabAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void Km60LabAudioProcessorEditor::setupKnob(juce::Slider& knob, juce::Label& label,
                                              const juce::String& text)
{
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, kTextBoxH);
    knob.setNumDecimalPlacesToDisplay(1);
    knob.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(knob);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(12.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, colLabel);
    addAndMakeVisible(label);
}

void Km60LabAudioProcessorEditor::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();
    const int panelX = kFramePad;
    const int panelW = w - kFramePad * 2;

    // Grey frame
    g.fillAll(colFrame);

    // Branding panel (dark inset)
    g.setColour(colBrandBg);
    g.fillRoundedRectangle((float)panelX, (float)kFramePad,
                           (float)panelW, (float)kBrandH, 3.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("KM-60", panelX, kFramePad + 6, panelW, 24, juce::Justification::centred);

    g.setColour(colGold);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("Lab", panelX, kFramePad + 28, panelW, 16, juce::Justification::centred);

    // Dark channel strip panel
    const int chPanelY = kFramePad + kBrandH + 6;
    const int chPanelH = h - chPanelY - kFramePad;
    g.setColour(colPanel);
    g.fillRoundedRectangle((float)panelX, (float)chPanelY,
                           (float)panelW, (float)chPanelH, 3.0f);
    g.setColour(juce::Colour(0xff1A1A1A));
    g.drawRoundedRectangle((float)panelX, (float)chPanelY,
                           (float)panelW, (float)chPanelH, 3.0f, 1.0f);

    // Orange accent line on left edge of channel strip
    g.setColour(colGold);
    g.fillRect((float)panelX + 1.0f, (float)chPanelY + 6.0f,
               2.0f, (float)chPanelH - 12.0f);

}

void Km60LabAudioProcessorEditor::resized()
{
    const int panelX = kFramePad;
    const int panelW = getWidth() - kFramePad * 2;
    const int knobTop = kFramePad + kBrandH + 14;
    const int knobBot = getHeight() - kFramePad - 8;
    const int sectionH = (knobBot - knobTop) / 4;

    struct Row { juce::Slider& k; juce::Label& l; };
    Row rows[] = {
        { inputGainKnob,  inputGainLabel  },
        { trebleKnob,     trebleLabel     },
        { bassKnob,       bassLabel       },
        { outputGainKnob, outputGainLabel },
    };

    for (int i = 0; i < 4; ++i)
    {
        int sy = knobTop + i * sectionH;
        rows[i].l.setBounds(panelX, sy, panelW, kLabelH);

        int knobW = juce::jmin(panelW - 8, 84);
        int knobH = sectionH - kLabelH - 4;
        int knobX = panelX + (panelW - knobW) / 2;
        rows[i].k.setBounds(knobX, sy + kLabelH + 2, knobW, knobH);
    }
}
