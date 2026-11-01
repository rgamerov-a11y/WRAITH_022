#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class WraithAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit WraithAudioProcessorEditor(WraithAudioProcessor&);
 ~WraithAudioProcessorEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 struct ThinLook final : juce::LookAndFeel_V4 {
  void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
 } thin;
 void timerCallback() override { repaint(890,310,20,95); }
 WraithAudioProcessor& processor;
 juce::Slider reverbPlacement,character,harmonics,damping,decay,glide,position,root,tone,space,time,motion,utilityLP,utilityHP,utilityCompression,mix,output;
 juce::TextButton resetFilter {"RESET"};
 void resetUtility();
 using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::vector<std::unique_ptr<Attachment>> attachments;
 void rotary(juce::Slider&,const char*);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessorEditor)
};
