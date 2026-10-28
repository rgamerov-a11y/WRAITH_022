#include "PluginEditor.h"

WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(790, 560);
    title.setText("WRAITH", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(31.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe1d6ff));
    addAndMakeVisible(title);

    subtitle.setText("LIVE INPUT GRANULAR RESONATOR  /  SOURCE 0.5", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffaaa8bd));
    addAndMakeVisible(subtitle);

    styleSlider(resonance, "Resonance");
    styleSlider(decay, "Decay");
    styleSlider(space, "Space");
    styleSlider(motion, "Motion");
    styleSlider(grainMix, "Grain Mix");
    styleSlider(grainSize, "Grain Size (ms)");
    styleSlider(grainDensity, "Grain Density");
    styleSlider(grainTone, "Grain Tone");
    styleSlider(output, "Output");

    auto& state = processor.parameters;
    resonanceAttachment = std::make_unique<Attachment>(state, "resonance", resonance);
    decayAttachment = std::make_unique<Attachment>(state, "decay", decay);
    spaceAttachment = std::make_unique<Attachment>(state, "space", space);
    motionAttachment = std::make_unique<Attachment>(state, "motion", motion);
    grainMixAttachment = std::make_unique<Attachment>(state, "grainMix", grainMix);
    grainSizeAttachment = std::make_unique<Attachment>(state, "grainSize", grainSize);
    grainDensityAttachment = std::make_unique<Attachment>(state, "grainDensity", grainDensity);
    grainToneAttachment = std::make_unique<Attachment>(state, "grainTone", grainTone);
    outputAttachment = std::make_unique<Attachment>(state, "output", output);

    for (auto* button : { &reverseButton, &freezeButton })
    {
        button->setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe1d6ff));
        button->setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffa58bff));
        addAndMakeVisible(*button);
    }
    reverseAttachment = std::make_unique<ButtonAttachment>(state, "reverse", reverseButton);
    freezeAttachment = std::make_unique<ButtonAttachment>(state, "freeze", freezeButton);
}

void WraithAudioProcessorEditor::styleSlider(juce::Slider& slider, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 22);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffa58bff));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe1d6ff));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff252633));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff454456));
    slider.setName(name);
    addAndMakeVisible(slider);
}

void WraithAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff171821));
    g.setColour(juce::Colour(0xff2b2c3b));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8.0f), 12.0f, 1.0f);
    g.setColour(juce::Colour(0xff8b7bd0));
    g.drawLine(22.0f, 76.0f, 768.0f, 76.0f, 1.0f);
    g.setColour(juce::Colour(0xffaaa8bd));
    g.setFont(12.0f);
    g.drawText("16-VOICE MIDI  •  LIVE AUDIO CAPTURE  •  REVERSE GRAINS  •  FREEZE TEXTURE",
               24, 530, 740, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8b7bd0));
    g.drawText("RESONATOR", 24, 82, 300, 16, juce::Justification::centredLeft);
    g.drawText("CAPTURED INPUT TEXTURE", 24, 282, 300, 16, juce::Justification::centredLeft);
}

void WraithAudioProcessorEditor::resized()
{
    title.setBounds(24, 13, 250, 42);
    subtitle.setBounds(26, 48, 420, 20);
    const int w = 142, h = 168, gap = 10;
    juce::Slider* topRow[] = { &resonance, &decay, &space, &motion, &output };
    for (int i = 0; i < 5; ++i)
        topRow[i]->setBounds(15 + i * (w + gap), 101, w, h);
    juce::Slider* bottomRow[] = { &grainMix, &grainSize, &grainDensity, &grainTone };
    const int bottomW = 170, bottomGap = 18;
    for (int i = 0; i < 4; ++i)
        bottomRow[i]->setBounds(28 + i * (bottomW + bottomGap), 300, bottomW, 150);
    reverseButton.setBounds(42, 464, 170, 28);
    freezeButton.setBounds(238, 464, 170, 28);
}
