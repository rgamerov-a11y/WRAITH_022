#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class WraithVisual final : public juce::Component, private juce::Timer
{
public:
    explicit WraithVisual(WraithAudioProcessor&);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    WraithAudioProcessor& processor;
    float phase = 0.0f, energy = 0.0f, smoothCharacter = 0.5f;
    unsigned int lastTrigger = 0;
    float pulsePhase = 100.0f, pulseX = 0.0f;
};

class WraithAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit WraithAudioProcessorEditor(WraithAudioProcessor&);
    ~WraithAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    WraithAudioProcessor& processor;
    WraithVisual visual;
    juce::Slider resonance, decay, space, motion, grainMix, grainSize, grainDensity, grainTone, output, dryWet, resonatorMix, synthMix, character, harmonics, damping, glide;
    juce::ToggleButton reverseButton { "REVERSE" }, freezeButton { "GRANULAR FREEZE" };
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> resonanceAttachment, decayAttachment, spaceAttachment,
        motionAttachment, grainMixAttachment, grainSizeAttachment, grainDensityAttachment,
        grainToneAttachment, outputAttachment, dryWetAttachment, resonatorMixAttachment,
        synthMixAttachment, characterAttachment, harmonicsAttachment, dampingAttachment, glideAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> reverseAttachment, freezeAttachment;
    void styleSlider(juce::Slider&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessorEditor)
};
