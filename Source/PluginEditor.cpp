#include "PluginEditor.h"
#include <cmath>

namespace
{
    const auto background = juce::Colour(0xff0b0d13);
    const auto panel = juce::Colour(0xff151923);
    const auto muted = juce::Colour(0xff9094a8);
    const auto ivory = juce::Colour(0xfff0e8e0);
    const auto lilac = juce::Colour(0xffb9a3ff);
    void section(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& label)
    {
        g.setColour(panel); g.fillRoundedRectangle(bounds, 14.0f);
        g.setColour(juce::Colour(0xff282b3a)); g.drawRoundedRectangle(bounds, 14.0f, 1.0f);
        g.setColour(muted); g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(label, bounds.toNearestInt().reduced(18).removeFromTop(28), juce::Justification::centredLeft);
    }
}

WraithVisual::WraithVisual(WraithAudioProcessor& p) : processor(p)
{
    setOpaque(true);
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
    g.fillAll(background);
    const auto w = static_cast<float>(getWidth()), h = static_cast<float>(getHeight());
    juce::ColourGradient shade(juce::Colour(0xff19152b), w*.5f, h*.45f, background, w*.5f, h, true);
    g.setGradientFill(shade); g.fillRect(getLocalBounds());
    const float t = smoothCharacter;
    const float strings = juce::jmax(0.0f, 1.0f - std::abs(t) * 2.0f);
    const float metal = juce::jmax(0.0f, 1.0f - std::abs(t - .5f) * 2.0f);
    const float cloud = juce::jmax(0.0f, 1.0f - std::abs(t - 1.0f) * 2.0f);
    const auto cream = juce::Colour(0xfff1d6ac), purple = juce::Colour(0xffbd8eff), aqua = juce::Colour(0xff72e1d2);
    const auto tint = t < .5f ? cream.interpolatedWith(purple, t*2.0f) : purple.interpolatedWith(aqua, (t-.5f)*2.0f);
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
            const auto colour = tint.interpolatedWith(aqua, juce::jlimit(0.0f,1.0f,(u+v+2.0f)*.075f));
            g.setColour(colour.withAlpha(juce::jlimit(0.05f, .92f, .25f + env*.57f + energy*.10f)));
            const float radius = .55f + env*.65f;
            g.fillEllipse(px-radius,py-radius,radius*2,radius*2);
        }
    g.setColour(ivory.withAlpha(.85f)); g.setFont(juce::Font(juce::FontOptions(10.0f,juce::Font::bold)));
    g.drawText("RESONANCE FIELD", 20, 16, 220, 18, juce::Justification::centredLeft);
    g.setColour(muted); g.setFont(10.0f);
    g.drawText("STRINGS", 20, getHeight()-27, 100, 17, juce::Justification::centredLeft);
    g.drawText("METAL", getWidth()/2-45, getHeight()-27, 90, 17, juce::Justification::centred);
    g.drawText("CLOUD", getWidth()-115, getHeight()-27, 95, 17, juce::Justification::centredRight);
}

WraithAudioProcessorEditor::WraithAudioProcessorEditor(WraithAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), visual(p)
{
    setSize(1020, 810);
    addAndMakeVisible(visual);
    for (auto pair : { std::pair<juce::Slider*, const char*>{&resonance,"RESONANCE"}, {&decay,"DECAY"}, {&space,"SPACE"}, {&motion,"MOTION"},
                       {&grainMix,"GRAIN MIX"}, {&grainSize,"GRAIN SIZE"}, {&grainDensity,"DENSITY"}, {&grainTone,"TONE"},
                       {&output,"OUTPUT"}, {&dryWet,"DRY / WET"}, {&resonatorMix,"RESONATOR MIX"}, {&synthMix,"SYNTH MIX"},
                       {&character,"CHARACTER"}, {&harmonics,"HARMONICS"}, {&damping,"DAMPING"}, {&glide,"GLIDE"} })
        styleSlider(*pair.first, pair.second);
    auto& s = processor.parameters;
    resonanceAttachment = std::make_unique<Attachment>(s,"resonance",resonance);
    decayAttachment = std::make_unique<Attachment>(s,"decay",decay);
    spaceAttachment = std::make_unique<Attachment>(s,"space",space);
    motionAttachment = std::make_unique<Attachment>(s,"motion",motion);
    grainMixAttachment = std::make_unique<Attachment>(s,"grainMix",grainMix);
    grainSizeAttachment = std::make_unique<Attachment>(s,"grainSize",grainSize);
    grainDensityAttachment = std::make_unique<Attachment>(s,"grainDensity",grainDensity);
    grainToneAttachment = std::make_unique<Attachment>(s,"grainTone",grainTone);
    outputAttachment = std::make_unique<Attachment>(s,"output",output);
    dryWetAttachment = std::make_unique<Attachment>(s,"dryWet",dryWet);
    resonatorMixAttachment = std::make_unique<Attachment>(s,"resonatorMix",resonatorMix);
    synthMixAttachment = std::make_unique<Attachment>(s,"synthMix",synthMix);
    characterAttachment = std::make_unique<Attachment>(s,"character",character);
    harmonicsAttachment = std::make_unique<Attachment>(s,"harmonics",harmonics);
    dampingAttachment = std::make_unique<Attachment>(s,"damping",damping);
    glideAttachment = std::make_unique<Attachment>(s,"glide",glide);
    for (auto* b : {&reverseButton,&freezeButton})
    {
        b->setColour(juce::ToggleButton::textColourId, ivory);
        b->setColour(juce::ToggleButton::tickColourId, lilac);
        addAndMakeVisible(*b);
    }
    reverseAttachment = std::make_unique<ButtonAttachment>(s,"reverse",reverseButton);
    freezeAttachment = std::make_unique<ButtonAttachment>(s,"freeze",freezeButton);
    if (JucePlugin_IsSynth) { dryWet.setVisible(false); resonatorMix.setVisible(false); synthMix.setVisible(false); }
}
void WraithAudioProcessorEditor::styleSlider(juce::Slider& s, const juce::String& name)
{
    s.setName(name);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,20);
    s.setColour(juce::Slider::rotarySliderFillColourId,lilac);
    s.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff303343));
    s.setColour(juce::Slider::thumbColourId,ivory);
    s.setColour(juce::Slider::textBoxTextColourId,ivory);
    s.setColour(juce::Slider::textBoxBackgroundColourId,panel);
    s.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    addAndMakeVisible(s);
}
void WraithAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(ivory); g.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));
    g.drawText("W R A I T H",24,16,430,43,juce::Justification::centredLeft);
    g.setColour(muted); g.setFont(11.0f);
    g.drawText(JucePlugin_IsSynth ? "POLYPHONIC INSTRUMENT  /  1.0" : "MIDI RESONATOR FX  /  1.0",26,56,400,22,juce::Justification::centredLeft);
    section(g,{18,94,244,415},"RESONATOR");
    section(g,{758,94,244,415},"GRANULAR");
    section(g,{18,524,984,266},"MIX  /  SPACE  /  OUTPUT");
    g.setColour(muted); g.setFont(10.0f);
    for (auto* s : {&resonance,&decay,&space,&motion,&grainMix,&grainSize,&grainDensity,&grainTone,&output,&dryWet,&resonatorMix,&synthMix,&character,&harmonics,&damping,&glide})
        if (s->isVisible()) g.drawText(s->getName(),s->getX(),s->getY()-14,s->getWidth(),16,juce::Justification::centred);
    g.setColour(lilac); g.setFont(11.0f);
    g.drawText("CHARACTER MORPH",285,440,450,18,juce::Justification::centred);
    g.setColour(muted); g.setFont(10.0f);
    g.drawText("MIDI-REACTIVE PARTICLE FIELD  /  STRINGS — METAL — CLOUD",286,478,450,20,juce::Justification::centred);
}
void WraithAudioProcessorEditor::resized()
{
    visual.setBounds(279,94,462,336);
    auto place = [](juce::Slider& s,int x,int y,int w=105,int h=96){s.setBounds(x,y,w,h);};
    place(resonance,30,144); place(decay,148,144);
    place(harmonics,30,265); place(damping,148,265);
    place(glide,30,389); place(motion,148,389);
    place(grainMix,771,144); place(grainSize,889,144);
    place(grainDensity,771,265); place(grainTone,889,265);
    reverseButton.setBounds(782,399,108,30); freezeButton.setBounds(782,441,178,30);
    place(character,451,434,118,92);
    place(resonatorMix,40,585,125,112); place(synthMix,174,585,125,112);
    place(dryWet,310,585,125,112); place(space,446,585,125,112);
    place(output,582,585,125,112);
}
