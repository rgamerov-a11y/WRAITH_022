#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class WraithVisual final : public juce::Component,private juce::Timer {
public:
 explicit WraithVisual(WraithAudioProcessor&);
 void paint(juce::Graphics&) override;
 void mouseDown(const juce::MouseEvent&) override;
private:
 void timerCallback() override;
 WraithAudioProcessor& processor;
 float phase=0,energy=0,smoothCharacter=.5f;
 unsigned int lastTrigger=0;
 float pulsePhase=100,pulseX=0;
};
class WraithAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit WraithAudioProcessorEditor(WraithAudioProcessor&);
 ~WraithAudioProcessorEditor() override=default;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 void timerCallback() override {repaint(960,660,60,130);}
 WraithAudioProcessor& processor;
 WraithVisual visual;
 juce::Slider character,harmonics,damping,glide,filter,mix,output;
 using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::unique_ptr<Attachment> characterA,harmonicsA,dampingA,glideA,filterA,mixA,outputA;
 void rotary(juce::Slider&,const juce::String&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessorEditor)
};
