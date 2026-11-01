#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class WraithVisual final : public juce::Component,private juce::Timer {
public: explicit WraithVisual(WraithAudioProcessor&); void paint(juce::Graphics&) override; void mouseDown(const juce::MouseEvent&) override;
private: void timerCallback() override; WraithAudioProcessor& processor; float phase=0,energy=0,smoothCharacter=.5f; unsigned int lastTrigger=0; float pulsePhase=100,pulseX=0;
};
class WraithAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public: explicit WraithAudioProcessorEditor(WraithAudioProcessor&); ~WraithAudioProcessorEditor() override; void paint(juce::Graphics&) override; void resized() override;
private:
 struct ThinLook final : juce::LookAndFeel_V4 {void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;};
 ThinLook thin;
 void timerCallback() override {repaint(1110,630,48,85);}
 WraithAudioProcessor& processor; WraithVisual visual;
 juce::Slider character,harmonics,damping,decay,glide,position,root,tone,space,time,motion,cutoff,filterResonance,filterMode,filterGain,mix,output;
 using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::vector<std::unique_ptr<Attachment>> attachments;
 void rotary(juce::Slider&,const juce::String&,const char*);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessorEditor)
};
