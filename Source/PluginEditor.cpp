#include "PluginEditor.h"

WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(650, 405);
    title.setText("WRAITH", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(31.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe1d6ff));
    addAndMakeVisible(title);

    subtitle.setText("POLYPHONIC RESONANCE ENGINE  /  SOURCE 0.2", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffaaa8bd));
    addAndMakeVisible(subtitle);

    styleSlider(resonance, "Resonance");
    styleSlider(decay, "Decay");
    styleSlider(space, "Space");
    styleSlider(motion, "Motion");
    styleSlider(output, "Output");

    auto& state = processor.parameters;
    resonanceAttachment = std::make_unique<Attachment>(state, "resonance", resonance);
    decayAttachment = std::make_unique<Attachment>(state, "decay", decay);
    spaceAttachment = std::make_unique<Attachment>(state, "space", space);
    motionAttachment = std::make_unique<Attachment>(state, "motion", motion);
    outputAttachment = std::make_unique<Attachment>(state, "output", output);
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
    g.drawLine(22.0f, 76.0f, 628.0f, 76.0f, 1.0f);
    g.setColour(juce::Colour(0xffaaa8bd));
    g.setFont(12.0f);
    g.drawText("16-VOICE MIDI  •  CHORDS  •  CC64 SUSTAIN  •  STEREO SPACE",
               24, 358, 600, 22, juce::Justification::centredLeft);
}

void WraithAudioProcessorEditor::resized()
{
    title.setBounds(24, 13, 250, 42);
    subtitle.setBounds(26, 48, 420, 20);
    const int top = 103, w = 116, h = 220, gap = 8;
    juce::Slider* sliders[] = { &resonance, &decay, &space, &motion, &output };
    for (int i = 0; i < 5; ++i)
        sliders[i]->setBounds(15 + i * (w + gap), top, w, h);
}
