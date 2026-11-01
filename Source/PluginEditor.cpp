#include "PluginEditor.h"
#include <cmath>
namespace {const auto bg=juce::Colour(0xff0b0b0d); const auto white=juce::Colour(0xffededed);const auto muted=juce::Colour(0xff9c9ca2);}
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
    const float motion = processor.parameters.getRawParameterValue("motion")->load();
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



void WraithAudioProcessorEditor::ThinLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&){
 const float cx=x+w*.5f,cy=y+h*.49f,r=juce::jmin(w,h)*.32f;
 juce::Path bgArc;bgArc.addCentredArc(cx,cy,r,r,0,start,end,true);
 g.setColour(juce::Colours::white.withAlpha(.18f));g.strokePath(bgArc,juce::PathStrokeType(1.2f));
 const float a=start+pos*(end-start);juce::Path active;active.addCentredArc(cx,cy,r,r,0,start,a,true);
 g.setColour(juce::Colours::white.withAlpha(.9f));g.strokePath(active,juce::PathStrokeType(1.6f));
 g.drawLine(cx+std::sin(a)*(r-5),cy-std::cos(a)*(r-5),cx+std::sin(a)*(r+2),cy-std::cos(a)*(r+2),1.25f);
}
WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p):AudioProcessorEditor(&p),processor(p),visual(p){
 setSize(1200,760);addAndMakeVisible(visual);
 rotary(harmonics,"HARMONICS","harmonics");rotary(damping,"DAMPING","damping");rotary(decay,"DECAY","decay");
 rotary(glide,"GLIDE","glide");rotary(position,"POSITION","position");rotary(root,"ROOT","root");
 rotary(tone,"TONE","tone");rotary(space,"SPACE","space");rotary(time,"TIME","time");rotary(motion,"MOTION","motion");
 rotary(cutoff,"CUTOFF","cutoff");rotary(filterResonance,"RESONANCE","filterResonance");rotary(filterMode,"MODE","filterMode");rotary(filterGain,"GAIN","filterGain");rotary(mix,"MIX","dryWet");rotary(output,"OUT","output");
 character.setSliderStyle(juce::Slider::LinearHorizontal);character.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 character.setColour(juce::Slider::trackColourId,white.withAlpha(.55f));character.setColour(juce::Slider::thumbColourId,white);
 character.setColour(juce::Slider::backgroundColourId,white.withAlpha(.14f));addAndMakeVisible(character);
 attachments.push_back(std::make_unique<Attachment>(p.parameters,"character",character));startTimerHz(20);
}
void WraithAudioProcessorEditor::rotary(juce::Slider& s,const juce::String&,const char* id){
 s.setLookAndFeel(&thin);s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 addAndMakeVisible(s);attachments.push_back(std::make_unique<Attachment>(processor.parameters,id,s));
}
void WraithAudioProcessorEditor::paint(juce::Graphics& g){
 g.fillAll(bg);g.setColour(white);g.setFont(juce::Font(juce::FontOptions(27.0f)));
 g.drawText("W  R  A  I  T  H",46,20,440,44,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(11.0f)));
 g.drawText("E N H A N C E D   R E S O N A N C E    /    1.3",48,64,520,18,juce::Justification::centredLeft);
 g.drawText("M I D I   R E S O N A N T   F X",880,42,280,20,juce::Justification::centredRight);
 g.setColour(white);g.setFont(juce::Font(juce::FontOptions(15.0f)));
 g.drawText("C H A R A C T E R   M O R P H",340,446,520,25,juce::Justification::centred);
 g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(11.0f)));
 g.drawText("S T R I N G S",200,524,160,20,juce::Justification::centred);
 g.drawText("M E T A L",520,524,160,20,juce::Justification::centred);
 g.drawText("C L O U D",840,524,160,20,juce::Justification::centred);
 g.setColour(white.withAlpha(.16f));g.drawLine(35.0f,566.0f,1165.0f,566.0f,.8f);
 for(float x:{485.0f,765.0f,1035.0f})g.drawLine(x,588.0f,x,730.0f,.75f);
 g.setColour(white.withAlpha(.85f));g.setFont(juce::Font(juce::FontOptions(12.0f)));
 g.drawText("R E S O N A T O R",42,576,245,20,juce::Justification::centredLeft);
 g.drawText("T E X T U R E",495,576,220,20,juce::Justification::centredLeft);
 g.drawText("F I L T E R",775,576,190,20,juce::Justification::centredLeft);
 g.drawText("O U T P U T",1045,576,150,20,juce::Justification::centredLeft);
 auto label=[&](const char* name,juce::Slider& s){auto b=s.getBounds();g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(10.0f)));g.drawText(name,b.getX()-12,b.getBottom()+1,b.getWidth()+24,15,juce::Justification::centred);};
 label("HARMONICS",harmonics);label("DAMPING",damping);label("DECAY",decay);label("GLIDE",glide);label("POSITION",position);
 label("ROOT",root);label("TONE",tone);label("SPACE",space);label("TIME",time);label("MOTION",motion);
 label("CUTOFF",cutoff);label("RESONANCE",filterResonance);label("MODE",filterMode);label("GAIN",filterGain);label("MIX",mix);label("OUT",output);
 const float level=juce::jlimit(0.0f,1.0f,processor.visualLevel.load(std::memory_order_relaxed));
 g.setColour(white.withAlpha(.14f));g.fillRect(1160,635,3,61);
 g.setColour(white.withAlpha(.85f));const int bar=juce::roundToInt(level*61);g.fillRect(1160,696-bar,3,bar);
}
void WraithAudioProcessorEditor::resized(){
 visual.setBounds(95,89,1010,345);character.setBounds(235,478,730,34);
 const auto place=[](juce::Slider& s,int x,int y){s.setBounds(x,y,65,65);};
 place(harmonics,30,612);place(damping,104,612);place(decay,178,612);place(glide,252,612);place(position,326,612);place(root,400,612);
 place(tone,488,612);place(space,555,612);place(time,622,612);place(motion,689,612);
 place(cutoff,765,612);place(filterResonance,832,612);place(filterMode,899,612);place(filterGain,966,612);place(mix,1041,612);place(output,1110,612);
}

WraithAudioProcessorEditor::~WraithAudioProcessorEditor(){for(auto* s:{&harmonics,&damping,&decay,&glide,&position,&root,&tone,&space,&time,&motion,&cutoff,&filterResonance,&filterMode,&filterGain,&mix,&output})s->setLookAndFeel(nullptr);}
