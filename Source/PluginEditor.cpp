#include "PluginEditor.h"
#include <cmath>
namespace {
const auto bg=juce::Colour(0xff0b0b0d);
const auto white=juce::Colour(0xffededed);
const auto muted=juce::Colour(0xff9c9ca2);
}
WraithVisual::WraithVisual(WraithAudioProcessor& p) : processor(p)
{
    setOpaque(false);
    startTimerHz(30);
}
void WraithVisual::mouseDown(const juce::MouseEvent& e)
{
    pulseX = getWidth() > 0 ? e.position.x / static_cast<float>(getWidth()) : 0.5f;
    pulsePhase = 0.0f; energy = 1.0f; repaint();
}
void WraithVisual::timerCallback()
{
    const auto count = processor.visualTrigger.load(std::memory_order_relaxed);
    if (count != lastTrigger)
    {
        lastTrigger = count;
        const int note = processor.visualNote.load(std::memory_order_relaxed);
        pulseX = juce::jlimit(0.05f, 0.95f, (note - 36.0f) / 60.0f);
        energy = juce::jlimit(0.0f, 1.0f, processor.visualVelocity.load(std::memory_order_relaxed));
        pulsePhase = 0.0f;
    }
    const auto* p = processor.parameters.getRawParameterValue("character");
    if (p != nullptr)
    {
        // Parameter is normalized in the DSP as 0..1 or 0..100 depending on the layout.
        const float raw = p->load();
        const float target = juce::jlimit(0.0f, 1.0f, raw > 1.0f ? raw / 100.0f : raw);
        smoothCharacter += (target - smoothCharacter) * 0.085f;
    }
    const float motion = 0.28f;
    phase += 0.025f + 0.018f * juce::jlimit(0.0f, 1.0f, motion);
    energy *= 0.958f;
    pulsePhase += 0.032f;
    repaint();
}
void WraithVisual::paint(juce::Graphics& g)
{
    
    const auto w = static_cast<float>(getWidth()), h = static_cast<float>(getHeight());

    const float t = smoothCharacter;
    const float strings = juce::jmax(0.0f, 1.0f - std::abs(t) * 2.0f);
    const float metal = juce::jmax(0.0f, 1.0f - std::abs(t - .5f) * 2.0f);
    const float cloud = juce::jmax(0.0f, 1.0f - std::abs(t - 1.0f) * 2.0f);
    const auto tint = juce::Colours::white;
    // Procedural field inspired by the supplied particle video, not the video itself.
    // All drawing stays on the message thread, never the audio thread.
    for (int z = 0; z < 48; ++z)
        for (int x = 0; x < 92; ++x)
        {
            const float u = (x / 91.0f - .5f) * 2.4f;
            const float v = (z / 47.0f - .5f) * 2.0f;
            const float env = std::exp(-1.3f * (u*u + v*v));
            const float a = std::sin(u*4.0f + phase)*std::cos(v*2.4f-phase*.35f);
            const float b = std::sin(u*11.0f+phase*1.3f)*std::cos(v*8.0f-phase*.7f);
            const float c = std::sin(u*2.7f+v*2.0f+phase*.55f)*std::cos(v*3.7f-u+phase*.3f);
            float y = (strings*a + metal*b + cloud*c) * env * .37f;
            if (pulsePhase < 1.6f)
            {
                const float dist = std::hypot(u-(pulseX-.5f)*2.4f, v);
                y += std::sin(dist*15.0f-pulsePhase*22.0f)*std::exp(-12.0f*std::pow(dist-pulsePhase*.75f, 2.0f))*energy*.25f;
            }
            const float px = w*.5f + u*w*.31f + v*w*.10f;
            const float py = h*.50f + v*h*.22f - y*h*.57f;
            const auto colour = tint;
            g.setColour(colour.withAlpha(juce::jlimit(0.05f, .92f, .25f + env*.57f + energy*.10f)));
            const float radius = .55f + env*.65f;
            g.fillEllipse(px-radius,py-radius,radius*2,radius*2);
        }
    g.setColour(juce::Colours::white.withAlpha(.85f)); g.setFont(juce::Font(juce::FontOptions(10.0f,juce::Font::bold)));
    g.drawText("RESONANCE FIELD", 20, 16, 220, 18, juce::Justification::centredLeft);
    g.setColour(juce::Colours::white.withAlpha(.48f)); g.setFont(10.0f);
    g.drawText("STRINGS", 20, getHeight()-27, 100, 17, juce::Justification::centredLeft);
    g.drawText("METAL", getWidth()/2-45, getHeight()-27, 90, 17, juce::Justification::centred);
    g.drawText("CLOUD", getWidth()-115, getHeight()-27, 95, 17, juce::Justification::centredRight);
}


WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p)
 : AudioProcessorEditor(&p),processor(p),visual(p){
 setSize(1100,720);addAndMakeVisible(visual);
 for(auto* s:{&harmonics,&damping,&glide,&filter,&mix,&output})rotary(*s,"");
 character.setSliderStyle(juce::Slider::LinearHorizontal);
 character.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 character.setColour(juce::Slider::trackColourId,white.withAlpha(.55f));
 character.setColour(juce::Slider::thumbColourId,white);
 character.setColour(juce::Slider::backgroundColourId,white.withAlpha(.14f));
 addAndMakeVisible(character);
 auto& s=p.parameters;
 characterA=std::make_unique<Attachment>(s,"character",character);
 harmonicsA=std::make_unique<Attachment>(s,"harmonics",harmonics);
 dampingA=std::make_unique<Attachment>(s,"damping",damping);
 glideA=std::make_unique<Attachment>(s,"glide",glide);
 filterA=std::make_unique<Attachment>(s,"filter",filter);
 mixA=std::make_unique<Attachment>(s,"dryWet",mix);
 outputA=std::make_unique<Attachment>(s,"output",output);
 startTimerHz(20);
}
void WraithAudioProcessorEditor::rotary(juce::Slider& s,const juce::String&){
 s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
 s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 s.setColour(juce::Slider::rotarySliderFillColourId,white);
 s.setColour(juce::Slider::rotarySliderOutlineColourId,white.withAlpha(.12f));
 s.setColour(juce::Slider::thumbColourId,white);
 addAndMakeVisible(s);
}
void WraithAudioProcessorEditor::paint(juce::Graphics& g){
 g.fillAll(bg);
 g.setColour(white);g.setFont(juce::Font(juce::FontOptions(27.0f)));
 g.drawText("W  R  A  I  T  H",46,22,440,44,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(11.0f)));
 g.drawText("R E S O N A N T   F X    /    1.1",48,65,400,18,juce::Justification::centredLeft);
 g.drawText("A U D I O   E F F E C T",840,44,220,20,juce::Justification::centredRight);
 g.setColour(white);g.setFont(juce::Font(juce::FontOptions(16.0f)));
 g.drawText("C H A R A C T E R   M O R P H",310,443,480,27,juce::Justification::centred);
 g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(12.0f)));
 g.drawText("S T R I N G S",180,519,180,20,juce::Justification::centred);
 g.drawText("M E T A L",463,519,174,20,juce::Justification::centred);
 g.drawText("C L O U D",745,519,180,20,juce::Justification::centred);
 g.setColour(white.withAlpha(.15f));
 g.drawLine(42.0f,565.0f,1058.0f,565.0f,.75f);
 g.drawLine(530.0f,591.0f,530.0f,696.0f,.75f);
 g.drawLine(780.0f,591.0f,780.0f,696.0f,.75f);
 g.setColour(white.withAlpha(.85f));g.setFont(juce::Font(juce::FontOptions(13.0f)));
 g.drawText("R E S O N A T O R",50,580,210,24,juce::Justification::centredLeft);
 g.drawText("F I L T E R",567,580,170,24,juce::Justification::centredLeft);
 g.drawText("M I X",816,580,100,24,juce::Justification::centredLeft);
 auto label=[&](const juce::String& title,juce::Slider& s,int x,int y,int w){
   g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(10.5f)));
   g.drawText(title,x,y,w,15,juce::Justification::centred);
   g.setColour(white.withAlpha(.8f));g.setFont(juce::Font(juce::FontOptions(11.0f)));
   g.drawText(s.getTextFromValue(s.getValue()),x,y+17,w,18,juce::Justification::centred);
 };
 label("HARMONICS",harmonics,202,654,102);
 label("DAMPING",damping,309,654,102);
 label("GLIDE",glide,415,654,102);
 label("DARK  ·  OPEN  ·  THIN",filter,554,654,218);
 label("MIX",mix,801,654,105);
 label("OUT",output,911,654,105);
 const float level=juce::jlimit(0.0f,1.0f,processor.visualLevel.load(std::memory_order_relaxed));
 g.setColour(white.withAlpha(.12f));g.fillRect(1044,614,3,60);g.fillRect(1052,614,3,60);
 g.setColour(white.withAlpha(.8f));g.fillRect(1044,674-juce::roundToInt(level*60),3,juce::roundToInt(level*60));
 g.fillRect(1052,674-juce::roundToInt(level*60),3,juce::roundToInt(level*60));
}
void WraithAudioProcessorEditor::resized(){
 visual.setBounds(70,88,960,345);
 character.setBounds(218,478,665,32);
 harmonics.setBounds(207,604,90,53);
 damping.setBounds(315,604,90,53);
 glide.setBounds(421,604,90,53);
 filter.setBounds(622,604,88,53);
 mix.setBounds(806,604,90,53);
 output.setBounds(917,604,90,53);
}
