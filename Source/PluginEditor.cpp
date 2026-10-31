#include "PluginEditor.h"

WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(790, 815);
    title.setText("WRAITH", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(31.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe1d6ff));
    addAndMakeVisible(title);

    subtitle.setText(JucePlugin_IsSynth ? "POLYPHONIC MIDI INSTRUMENT / 0.8" : "MIDI-CONTROLLED RESONATOR FX / 0.8", juce::dontSendNotification);
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
    styleSlider(dryWet, "Dry / Wet");
    styleSlider(resonatorMix, "Resonator Mix");
    styleSlider(synthMix, "Synth Mix");
    styleSlider(character, "Character");
    styleSlider(harmonics, "Harmonics");
    styleSlider(damping, "Damping");
    resonatorMix.setVisible(! JucePlugin_IsSynth);
    synthMix.setVisible(! JucePlugin_IsSynth);
    dryWet.setVisible(! JucePlugin_IsSynth);

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
    dryWetAttachment = std::make_unique<Attachment>(state, "dryWet", dryWet);
    resonatorMixAttachment = std::make_unique<Attachment>(state, "resonatorMix", resonatorMix);
    synthMixAttachment = std::make_unique<Attachment>(state, "synthMix", synthMix);
    characterAttachment = std::make_unique<Attachment>(state, "character", character);
    harmonicsAttachment = std::make_unique<Attachment>(state, "harmonics", harmonics);
    dampingAttachment = std::make_unique<Attachment>(state, "damping", damping);

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
    g.drawText("MIDI-TUNED AUDIO RESONATORS  /  GRANULAR  /  HYBRID SYNTH", 24, 778, 740, 20, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8b7bd0));
    g.setColour(juce::Colour(0xffded6f7));
    for (auto* slider : { &resonance, &decay, &space, &motion, &output, &grainMix, &grainSize, &grainDensity, &grainTone, &dryWet, &resonatorMix, &synthMix, &character, &harmonics, &damping })
        if (slider->isVisible())
            g.drawText(slider->getName(), slider->getX(), slider->getY() - 17, slider->getWidth(), 18, juce::Justification::centred);
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
    dryWet.setBounds(520, 464, 155, 135);
    resonatorMix.setBounds(24, 506, 150, 125);
    synthMix.setBounds(188, 506, 150, 125);
    character.setBounds(32, 655, 180, 112);
    harmonics.setBounds(238, 655, 180, 112);
    damping.setBounds(444, 655, 180, 112);
    reverseButton.setBounds(42, 455, 170, 28);
    freezeButton.setBounds(238, 455, 170, 28);
}
