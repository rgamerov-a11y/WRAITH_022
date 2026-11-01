#include "PluginEditor.h"
#include <cmath>
namespace { const auto bg=juce::Colour(0xff0b0c0e); const auto white=juce::Colour(0xffededed); const auto muted=juce::Colour(0xffa5a5a9); }
void WraithAudioProcessorEditor::ThinLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&){
 const float cx=x+w*.5f,cy=y+h*.5f,r=juce::jmin(w,h)*.33f;
 juce::Path path;path.addCentredArc(cx,cy,r,r,0,start,end,true);
 g.setColour(white.withAlpha(.16f));g.strokePath(path,juce::PathStrokeType(1.0f));
 juce::Path active;active.addCentredArc(cx,cy,r,r,0,start,start+pos*(end-start),true);
 g.setColour(white.withAlpha(.92f));g.strokePath(active,juce::PathStrokeType(1.45f));
 const float a=start+pos*(end-start);
 g.drawLine(cx+std::sin(a)*(r-5),cy-std::cos(a)*(r-5),cx+std::sin(a)*(r+2),cy-std::cos(a)*(r+2),1.0f);
}
WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(940,510);
 rotary(reverbPlacement,"reverbPlacement");
 rotary(harmonics,"harmonics");
 rotary(damping,"damping");
 rotary(decay,"decay");
 rotary(glide,"glide");
 rotary(position,"position");
 rotary(root,"root");
 rotary(tone,"tone");
 rotary(space,"space");
 rotary(time,"time");
 rotary(motion,"motion");
 rotary(utilityLP,"utilityLP");
 rotary(utilityHP,"utilityHP");
 rotary(utilityCompression,"utilityCompression");
 rotary(mix,"dryWet");
 rotary(output,"output");
 character.setSliderStyle(juce::Slider::LinearHorizontal);character.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 character.setColour(juce::Slider::trackColourId,white.withAlpha(.6f));character.setColour(juce::Slider::thumbColourId,white);
 character.setColour(juce::Slider::backgroundColourId,white.withAlpha(.16f));addAndMakeVisible(character);
 attachments.push_back(std::make_unique<Attachment>(p.parameters,"character",character));
 resetFilter.setColour(juce::TextButton::buttonColourId,bg);
 resetFilter.setColour(juce::TextButton::textColourOffId,white);
 resetFilter.onClick=[this]{resetUtility();};
 addAndMakeVisible(resetFilter);
 startTimerHz(15);
}
void WraithAudioProcessorEditor::rotary(juce::Slider& s,const char* id){
 s.setLookAndFeel(&thin);s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);addAndMakeVisible(s);
 attachments.push_back(std::make_unique<Attachment>(processor.parameters,id,s));
}
void WraithAudioProcessorEditor::paint(juce::Graphics& g){
 g.fillAll(bg);g.setColour(white);g.setFont(juce::Font(juce::FontOptions(24.0f)));
 g.drawText("W  R  A  I  T  H",32,16,460,37,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(11.0f);g.drawText("1.5.1  /  SOUND LAB  /  MIDI RESONANT FX",35,54,510,20,juce::Justification::centredLeft);
 g.setColour(white.withAlpha(.18f));g.drawLine(30.0f,94.0f,910.0f,94.0f,.8f);
 g.setColour(white);g.setFont(13.0f);g.drawText("CHARACTER MORPH",30,105,200,22,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(10.0f);
 g.drawText("STRINGS",300,147,90,18,juce::Justification::centredLeft);
 g.drawText("METAL",505,147,90,18,juce::Justification::centred);
 g.drawText("CLOUD",730,147,90,18,juce::Justification::centredRight);
 g.setColour(white.withAlpha(.16f));g.drawLine(30.0f,184.0f,910.0f,184.0f,.8f);
 g.setColour(white.withAlpha(.85f));g.setFont(11.0f);
 g.drawText("RESONATOR",30,195,230,20,juce::Justification::centredLeft);
 g.drawText("TEXTURE / SPACE / REVERB",30,291,300,20,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(10.0f);g.drawText("REVERB: PRE (-1)  /  OFF (0)  /  POST (+1)",320,398,430,20,juce::Justification::centredLeft);
 g.drawText("MIX & UTILITY",555,195,180,20,juce::Justification::centredLeft);
 g.drawText("OUTPUT",730,291,180,20,juce::Justification::centredLeft);
 auto label=[&](const char* n,juce::Slider& s){const auto b=s.getBounds();g.setColour(muted);g.setFont(9.5f);g.drawText(n,b.getX()-15,b.getBottom()+1,b.getWidth()+30,16,juce::Justification::centred);};
 label("HARMONICS",harmonics);
 label("DAMPING",damping);
 label("DECAY",decay);
 label("GLIDE",glide);
 label("POSITION",position);
 label("ROOT",root);
 label("TONE",tone);
 label("SPACE",space);
 label("TIME",time);
 label("REVERB",reverbPlacement);
 label("MOTION",motion);
 label("LOW-PASS",utilityLP);
 label("HIGH-PASS",utilityHP);
 label("COMPRESSION",utilityCompression);
 label("MIX",mix);
 label("OUT",output);
 const float level=juce::jlimit(0.0f,1.0f,processor.visualLevel.load(std::memory_order_relaxed));
 g.setColour(white.withAlpha(.2f));g.fillRect(905,318,3,55);
 g.setColour(white.withAlpha(.9f));const int bar=juce::roundToInt(level*55);g.fillRect(905,373-bar,3,bar);
}
void WraithAudioProcessorEditor::resized(){
 character.setBounds(295,108,525,34);
 const auto place=[](juce::Slider& s,int x,int y){s.setBounds(x,y,55,55);};
 place(harmonics,30,219);
 place(damping,113,219);
 place(decay,196,219);
 place(glide,279,219);
 place(position,362,219);
 place(root,445,219);
 place(tone,30,315);
 place(space,113,315);
 place(time,196,315);
 place(reverbPlacement,445,315);
 place(motion,279,315);
 place(utilityLP,555,219);
 place(utilityHP,640,219);
 place(utilityCompression,725,219);
 resetFilter.setBounds(817,233,76,28);
 place(mix,730,315);
 place(output,810,315);
}
WraithAudioProcessorEditor::~WraithAudioProcessorEditor(){
 for(auto* s:{&reverbPlacement,&harmonics,&damping,&decay,&glide,&position,&root,&tone,&space,&time,&motion,&utilityLP,&utilityHP,&utilityCompression,&mix,&output})s->setLookAndFeel(nullptr);
}

void WraithAudioProcessorEditor::resetUtility(){
 // Host-notified parameter updates preserve automation and use DSP smoothing.
 const struct { const char* id; float value; } targets[]={
   {"utilityLP",1.0f},{"utilityHP",0.0f},{"utilityCompression",0.0f}
 };
 for(const auto& target:targets){
   if(auto* param=processor.parameters.getParameter(target.id)){
     param->beginChangeGesture();
     param->setValueNotifyingHost(param->convertTo0to1(target.value));
     param->endChangeGesture();
   }
 }
}
