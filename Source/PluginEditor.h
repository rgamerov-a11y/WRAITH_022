#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class WraithAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit WraithAudioProcessorEditor(WraithAudioProcessor&);
    ~WraithAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    WraithAudioProcessor& processor;
    juce::Slider resonance, decay, space, motion, grainMix, grainSize, grainDensity, grainTone, output;
    juce::Label title, subtitle, footer;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> resonanceAttachment, decayAttachment, spaceAttachment,
        motionAttachment, grainMixAttachment, grainSizeAttachment, grainDensityAttachment,
        grainToneAttachment, outputAttachment;

    void styleSlider(juce::Slider&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessorEditor)
};
