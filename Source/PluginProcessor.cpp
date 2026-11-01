#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace {
constexpr float twoPi=juce::MathConstants<float>::twoPi;
float midiToHz(int note) { return 440.0f*std::pow(2.0f,(static_cast<float>(note)-69.0f)/12.0f); }
}
WraithAudioProcessor::WraithAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 parameters(*this,nullptr,"WRAITH_STATE",createParameterLayout()) {}
juce::AudioProcessorValueTreeState::ParameterLayout WraithAudioProcessor::createParameterLayout() {
 using F=juce::AudioParameterFloat; juce::AudioProcessorValueTreeState::ParameterLayout p;
 p.add(std::make_unique<F>("character","Character",juce::NormalisableRange<float>(0,1,.001f),.5f));
 p.add(std::make_unique<F>("harmonics","Harmonics",juce::NormalisableRange<float>(0,1,.001f),.65f));
 p.add(std::make_unique<F>("damping","Damping",juce::NormalisableRange<float>(0,1,.001f),.4f));
 p.add(std::make_unique<F>("glide","Glide (ms)",juce::NormalisableRange<float>(0,500,1),0.0f));
 // Keep legacy filter parameter for backwards-compatible preset loading; it is not used by v1.3.
 p.add(std::make_unique<F>("filter","Legacy Filter",juce::NormalisableRange<float>(0,1,.001f),.5f));
 // Legacy Sparks parameters retained for preset compatibility, no longer in the audio path.
 p.add(std::make_unique<F>("cutoff","Legacy Cutoff",juce::NormalisableRange<float>(20.0f,20000.0f,0.0f,0.25f),2400.0f));
 p.add(std::make_unique<F>("filterResonance","Legacy Filter Resonance",juce::NormalisableRange<float>(0.0f,1.0f,.001f),.3f));
 p.add(std::make_unique<F>("filterMode","Legacy Filter Mode",juce::NormalisableRange<float>(0.0f,1.0f,.001f),.5f));
 p.add(std::make_unique<F>("filterGain","Legacy Filter Gain",juce::NormalisableRange<float>(0.0f,24.0f,.1f),0.0f));
 p.add(std::make_unique<F>("utilityLP","Low-Pass",juce::NormalisableRange<float>(0.0f,1.0f,.001f),1.0f));
 p.add(std::make_unique<F>("utilityHP","High-Pass",juce::NormalisableRange<float>(0.0f,1.0f,.001f),0.0f));
 p.add(std::make_unique<F>("utilityCompression","Compression",juce::NormalisableRange<float>(0.0f,1.0f,.001f),0.0f));
 p.add(std::make_unique<F>("dryWet","Mix",juce::NormalisableRange<float>(0,1,.001f),.72f));
 p.add(std::make_unique<F>("output","Output (dB)",juce::NormalisableRange<float>(-24,12,.1f),0.0f));
 p.add(std::make_unique<F>("resonance","Resonance",juce::NormalisableRange<float>(0,1,.001f),.75f));
 p.add(std::make_unique<F>("decay","Decay",juce::NormalisableRange<float>(.15f,18.0f,.01f),6.0f));
 p.add(std::make_unique<F>("space","Space",juce::NormalisableRange<float>(0,1,.001f),.42f));
 p.add(std::make_unique<F>("position","Position",juce::NormalisableRange<float>(0,1,.001f),.38f));
 p.add(std::make_unique<F>("tone","Tone",juce::NormalisableRange<float>(0,1,.001f),.65f));
 p.add(std::make_unique<F>("motion","Motion",juce::NormalisableRange<float>(0,1,.001f),.32f));
 p.add(std::make_unique<F>("reverbPlacement","Reverb PRE / POST",juce::NormalisableRange<float>(-1.0f,1.0f,.001f),0.0f));
 p.add(std::make_unique<F>("time","Time",juce::NormalisableRange<float>(0,1,.001f),.55f));
 p.add(std::make_unique<F>("root","Root (semitones)",juce::NormalisableRange<float>(-12,12,1),0.0f));
 return p;
}
void WraithAudioProcessor::prepareToPlay(double sr,int block) {
 currentSampleRate=sr; dryBuffer.setSize(2,juce::jmax(1,block)); spaceBuffer.setSize(2,juce::jmax(1,block)); excitationBuffer.setSize(2,juce::jmax(1,block)); preBuffer.setSize(2,juce::jmax(1,block)); spaceHighpassState={0,0}; preHighpassState={0,0}; resonanceEnergy=0.0f;
 juce::dsp::ProcessSpec spec{sr,static_cast<juce::uint32>(juce::jmax(1,block)),2};
 reverb.reset(); reverb.prepare(spec); preReverb.reset(); preReverb.prepare(spec);
 preAmount.reset(sr,.04);postAmount.reset(sr,.04);preAmount.setCurrentAndTargetValue(0.0f);postAmount.setCurrentAndTargetValue(0.0f);
 reverbParams.roomSize=.92f; reverbParams.damping=.38f; reverbParams.width=1; reverbParams.wetLevel=.3f; reverbParams.dryLevel=.8f; reverb.setParameters(reverbParams);
 for(auto& f:filter){f.reset();f.prepare({sr,static_cast<juce::uint32>(juce::jmax(1,block)),1});f.setType(juce::dsp::StateVariableTPTFilterType::bandpass);f.setResonance(.707f);f.setCutoffFrequency(2400.0f);}
 for(auto& f:filterLow){f.reset();f.prepare({sr,static_cast<juce::uint32>(juce::jmax(1,block)),1});f.setType(juce::dsp::StateVariableTPTFilterType::lowpass);f.setResonance(.707f);f.setCutoffFrequency(2400.0f);}
 for(auto& f:filterHigh){f.reset();f.prepare({sr,static_cast<juce::uint32>(juce::jmax(1,block)),1});f.setType(juce::dsp::StateVariableTPTFilterType::highpass);f.setResonance(.707f);f.setCutoffFrequency(2400.0f);}
 lpSmooth.reset(sr,.08);lpSmooth.setCurrentAndTargetValue(1.0f);
 hpSmooth.reset(sr,.08);hpSmooth.setCurrentAndTargetValue(0.0f);
 compressionSmooth.reset(sr,.08);compressionSmooth.setCurrentAndTargetValue(0.0f);
 compressorEnvelope={0.0f,0.0f};
 resonanceSmooth.reset(sr,.03);resonanceSmooth.setCurrentAndTargetValue(.3f);
 gainSmooth.reset(sr,.03);gainSmooth.setCurrentAndTargetValue(0.0f);
 filterMorph.reset(sr,.025);filterMorph.setCurrentAndTargetValue(.5f);
 wetSmooth.reset(sr,.02);wetSmooth.setCurrentAndTargetValue(.72f);
 characterSmooth.reset(sr,.035);characterSmooth.setCurrentAndTargetValue(.5f);
 motionPhase={0,0};toneLP={0,0};
 outputSmooth.reset(sr,.02);outputSmooth.setCurrentAndTargetValue(1.0f);
 for(auto& v:voices)v=Voice{};
 voiceAgeCounter=0;sustainDown=false;lastPlayedHz=0;
}
void WraithAudioProcessor::releaseResources(){reverb.reset();preReverb.reset();}
bool WraithAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 const auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
 return in==out && (in==juce::AudioChannelSet::stereo()||in==juce::AudioChannelSet::mono());
}
void WraithAudioProcessor::noteOn(int note, int channel, float velocity)
{
    visualNote.store(note, std::memory_order_relaxed);
    visualVelocity.store(velocity, std::memory_order_relaxed);
    visualTrigger.fetch_add(1, std::memory_order_relaxed);
    // Retrigger the same held note on the same MIDI channel rather than stacking duplicates.
    for (auto& voice : voices)
    {
        if (voice.active && voice.note == note && voice.channel == channel && voice.keyDown)
        {
            voice.keyDown = false;
            releaseVoice(voice);
        }
    }

    int slot = -1;
    for (int i = 0; i < maxVoices; ++i)
        if (!voices[static_cast<size_t>(i)].active) { slot = i; break; }

    if (slot < 0)
    {
        // Prefer stealing the oldest released voice; otherwise steal the oldest voice.
        uint64_t oldest = UINT64_MAX;
        for (int i = 0; i < maxVoices; ++i)
        {
            const auto& v = voices[static_cast<size_t>(i)];
            if (!v.keyDown && v.age < oldest) { oldest = v.age; slot = i; }
        }
        if (slot < 0)
        {
            oldest = UINT64_MAX;
            for (int i = 0; i < maxVoices; ++i)
                if (voices[static_cast<size_t>(i)].age < oldest)
                {
                    oldest = voices[static_cast<size_t>(i)].age;
                    slot = i;
                }
        }
    }

    auto& v = voices[static_cast<size_t>(juce::jmax(0, slot))];
    v = Voice {};
    v.note = note;
    v.targetHz = midiToHz(juce::jlimit(0,127,note+juce::roundToInt(parameters.getRawParameterValue("root")->load())));
    v.currentHz = lastPlayedHz > 0.0f ? lastPlayedHz : v.targetHz;
    lastPlayedHz = v.targetHz;
    v.channel = channel;
    v.velocity = juce::jlimit(0.0f, 1.0f, velocity);
    v.envelope = 0.0f;
    v.active = true;
    v.keyDown = true;
    v.sustained = false;
    v.age = ++voiceAgeCounter;
}

void WraithAudioProcessor::releaseVoice(Voice& voice)
{
    voice.keyDown = false;
    voice.sustained = false;
    const float decay = parameters.getRawParameterValue("decay")->load();
    // Linear release step gives an approximate release duration controlled by Decay.
    voice.releaseStep = 1.0f / static_cast<float>(juce::jmax(1.0, currentSampleRate * decay));
}

void WraithAudioProcessor::noteOff(int note, int channel)
{
    for (auto& voice : voices)
    {
        if (!voice.active || voice.note != note || voice.channel != channel || !voice.keyDown)
            continue;

        voice.keyDown = false;
        if (sustainDown)
            voice.sustained = true;
        else
            releaseVoice(voice);
    }
}

void WraithAudioProcessor::allNotesOff(bool immediate)
{
    sustainDown = false;
    for (auto& voice : voices)
    {
        if (!voice.active) continue;
        voice.keyDown = false;
        voice.sustained = false;
        if (immediate)
        {
            voice.active = false;
            voice.envelope = 0.0f;
            voice.note = -1;
        }
        else
            releaseVoice(voice);
    }
}


void WraithAudioProcessor::renderRange(juce::AudioBuffer<float>& b,int start,int count,float resonance,float decay,float character,float harmonics,float damping,float glide,float position,float tone,float motion,float time) {
 const float sr=static_cast<float>(currentSampleRate);
 const float coeff=glide<=0?0.0f:std::exp(-1.0f/(.001f*glide*sr));
 constexpr float stringsRatio[6]={1,2,3,4,5,6};
 constexpr float metalRatio[6]={1,2.41f,3.87f,5.52f,7.13f,9.31f};
 constexpr float cloudRatio[6]={1.0f,2.002f,3.006f,4.012f,5.018f,6.026f};
 const float toneHz=350.0f*std::pow(40.0f,tone);
 const float toneA=1.0f-std::exp(-twoPi*toneHz/sr);
 const float spaceMotion=motion*motion;
 characterSmooth.setTargetValue(character);
 for(int s=start;s<start+count;++s){
   const float morph=characterSmooth.getNextValue();
   const float strings=juce::jmax(0.0f,1.0f-2.0f*morph);
   const float cloud=juce::jmax(0.0f,2.0f*morph-1.0f);
   const float metal=1.0f-strings-cloud;
   float l=0.0f,r=0.0f;
   float excitationSamples[2]={0,0};
   for(int ch=0;ch<b.getNumChannels();++ch){const float input=excitationBuffer.getSample(ch,s);toneLP[static_cast<size_t>(ch)]+=toneA*(input-toneLP[static_cast<size_t>(ch)]);excitationSamples[ch]=toneLP[static_cast<size_t>(ch)];}
   for(auto& v:voices){if(!v.active||v.note<0)continue;
     v.currentHz=coeff*v.currentHz+(1.0f-coeff)*v.targetHz;
     float accum[2]={0,0};
     for(int k=0;k<6;++k){
       const float partial=static_cast<float>(k)/5.0f;
       const float ratio=strings*stringsRatio[k]+metal*metalRatio[k]+cloud*cloudRatio[k];
       const float drift=1.0f+(.0002f+.0012f*cloud+.0015f*metal)*spaceMotion*std::sin(motionPhase[0]+k*1.47f);
       const float freq=v.currentHz*ratio*drift;
       if(freq>=sr*.44f||freq<12.0f)continue;
       const float omega=twoPi*freq/sr;
       const float modeDecay=juce::jmax(.09f,decay*(1.0f-.78f*damping*partial)*(1.0f+.20f*cloud-.22f*metal));
       const float fb=juce::jmin(.999995f,std::exp(-6.907755f/(modeDecay*sr)));
       // Excitation point controls the harmonic nodes of a virtual string/plate.
       const float posShape=.12f+.88f*std::abs(std::sin(juce::MathConstants<float>::pi*(k+1)*(.06f+.88f*position)));
       const float cloudAir=1.0f-cloud*(k==0?.25f:(.05f+.10f*partial));
       const float metalTame=1.0f-metal*(.12f+.48f*partial*partial);
       const float partialGain=std::pow(.42f+.56f*harmonics,static_cast<float>(k)) * posShape*(1.0f-.55f*damping*partial)*cloudAir*metalTame;
       for(int ch=0;ch<b.getNumChannels();++ch){
         auto& st=v.modes[static_cast<size_t>(ch)][static_cast<size_t>(k)];
         const float excitation=excitationSamples[ch];
         const float next=2.0f*fb*std::cos(omega)*st.y1-fb*fb*st.y2+excitation*(1.0f-fb)*3.5f;
         st.y2=st.y1;st.y1=next;
         accum[ch]+=next*partialGain;
       }
     }
     // Cloud uses lower excitation to avoid stacking 16 sustained resonators into the saturator.
     const float characterGain=1.5f*strings+1.65f*metal+1.65f*cloud;
     const float gain=resonance*v.velocity*v.envelope*characterGain;
     l+=accum[0]*gain;r+=accum[b.getNumChannels()>1?1:0]*gain;
     if(v.keyDown)v.envelope+=(1.0f-v.envelope)*.0025f;
     else if(!v.sustained){
       const float step=1.0f/juce::jmax(1.0f,decay*sr);
       v.envelope-=juce::jmax(step,v.releaseStep);
       if(v.envelope<=.00002f)v=Voice{};
     }
   }
   // Smooth energy-dependent gain reduction: prevents sustained chords from
   // driving the spatial stage into a dense, undifferentiated wash.
   const float power=.5f*(l*l+r*r);
   const float follow=power>resonanceEnergy ? .0035f : .00008f;
   resonanceEnergy+=(power-resonanceEnergy)*follow;
   const float dynamicGain=1.0f/std::sqrt(1.0f+resonanceEnergy*(2.0f+1.0f*cloud));
   l*=dynamicGain; r*=dynamicGain;
   // Soft saturation increases density without hard clipping.
   const float drive=1.15f+.12f*metal-.35f*cloud;
   const float softL=std::tanh(l*drive)*.83f;
   const float softR=std::tanh(r*drive)*.83f;
   // In Cloud retain more uncompressed dynamics, with bounded peaks.
   const float cloudL=std::tanh(l*1.10f)*.94f;
   const float cloudR=std::tanh(r*1.10f)*.94f;
   b.setSample(0,s,softL*(1.0f-cloud)+cloudL*cloud);
   if(b.getNumChannels()>1)b.setSample(1,s,softR*(1.0f-cloud)+cloudR*cloud);
   motionPhase[0]+=twoPi*(.07f+2.4f*spaceMotion)/sr;
   if(motionPhase[0]>twoPi)motionPhase[0]-=twoPi;
 }
}
void WraithAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;
 const int n=b.getNumSamples(),channels=b.getNumChannels();
 if(dryBuffer.getNumSamples()<n){jassertfalse;return;}
 for(int ch=0;ch<channels;++ch)dryBuffer.copyFrom(ch,0,b,ch,0,n);
 const float placement=parameters.getRawParameterValue("reverbPlacement")->load();
 // Center is dry; left blends PRE, right blends POST. Equal center routing
 // avoids changing the resonator timbre when the placement control is neutral.
 preAmount.setTargetValue(juce::jmax(0.0f,-placement));
 postAmount.setTargetValue(juce::jmax(0.0f,placement));
 const float spaceValue=parameters.getRawParameterValue("space")->load();
 const float timeValue=parameters.getRawParameterValue("time")->load();
 const float toneValue=parameters.getRawParameterValue("tone")->load();
 reverbParams.roomSize=juce::jlimit(.15f,.999f,.22f+.779f*timeValue);
 reverbParams.damping=juce::jlimit(0.0f,1.0f,.72f-.22f*toneValue);
 reverbParams.width=juce::jlimit(0.0f,1.0f,.5f+.5f*spaceValue);
 reverbParams.wetLevel=1.0f; reverbParams.dryLevel=0.0f;
 reverb.setParameters(reverbParams);preReverb.setParameters(reverbParams);
 const float hpA=1.0f-std::exp(-twoPi*(145.0f+125.0f*spaceValue)/static_cast<float>(currentSampleRate));
 // PRE: reverberate the input first; the resulting tail excites the resonators.
 // Process even at zero PRE amount so the internal reverb state stays continuous.
 preBuffer.clear();
 for(int ch=0;ch<channels;++ch)for(int i=0;i<n;++i){
   const float x=dryBuffer.getSample(ch,i);
   auto& low=preHighpassState[static_cast<size_t>(ch)];low+=hpA*(x-low);
   preBuffer.setSample(ch,i,x-low);
 }
 if(channels==2){
   juce::dsp::AudioBlock<float> block(preBuffer);
   juce::dsp::ProcessContextReplacing<float> context(block.getSubBlock(0,static_cast<size_t>(n)));
   preReverb.process(context);
 } else { preBuffer.clear(); }
 // Apply the same ramp to all channels at each sample, rather than each channel separately.
 for(int i=0;i<n;++i){
   const float a=preAmount.getNextValue();
   for(int ch=0;ch<channels;++ch)
     excitationBuffer.setSample(ch,i,dryBuffer.getSample(ch,i)+preBuffer.getSample(ch,i)*(.12f+.35f*spaceValue)*a);
 }

 const auto raw=[this](const char* id){return parameters.getRawParameterValue(id)->load();};
 const float resonance=raw("resonance"),decay=raw("decay"),character=raw("character"),harmonics=raw("harmonics"),damping=raw("damping"),glide=raw("glide"),position=raw("position"),tone=raw("tone"),motion=raw("motion"),time=raw("time");
 int rendered=0;
 for(const auto meta:midi){
   const auto msg=meta.getMessage();
   const int at=juce::jlimit(rendered,n,meta.samplePosition);
   if(at>rendered)renderRange(b,rendered,at-rendered,resonance,decay,character,harmonics,damping,glide,position,tone,motion,time);
   if(msg.isNoteOn())noteOn(msg.getNoteNumber(),msg.getChannel(),msg.getFloatVelocity());
   else if(msg.isNoteOff())noteOff(msg.getNoteNumber(),msg.getChannel());
   else if(msg.isController()){
     if(msg.getControllerNumber()==64){const bool down=msg.getControllerValue()>=64;
       if(sustainDown&&!down)for(auto& v:voices)if(v.active&&v.sustained&&!v.keyDown)releaseVoice(v);
       sustainDown=down;
     }else if(msg.getControllerNumber()==120)allNotesOff(true);
     else if(msg.getControllerNumber()==123)allNotesOff(false);
   }
   rendered=at;
 }
 if(rendered<n)renderRange(b,rendered,n-rendered,resonance,decay,character,harmonics,damping,glide,position,tone,motion,time);
 // POST: reverb receives the completed resonator output. PRE and POST
 // use independent state so changing placement never feeds back into the other path.
 if(channels==2){
   for(int ch=0;ch<channels;++ch)for(int i=0;i<n;++i){
     const float x=b.getSample(ch,i);
     auto& low=spaceHighpassState[static_cast<size_t>(ch)];low+=hpA*(x-low);
     spaceBuffer.setSample(ch,i,x-low);
   }
   juce::dsp::AudioBlock<float> block(spaceBuffer);
   juce::dsp::ProcessContextReplacing<float> context(block.getSubBlock(0,static_cast<size_t>(n)));
   reverb.process(context);
   for(int i=0;i<n;++i){
     const float a=postAmount.getNextValue()*(.12f+.35f*spaceValue);
     for(int ch=0;ch<channels;++ch)b.addSample(ch,i,spaceBuffer.getSample(ch,i)*a);
   }
 }
 lpSmooth.setTargetValue(raw("utilityLP"));
 hpSmooth.setTargetValue(raw("utilityHP"));
 compressionSmooth.setTargetValue(raw("utilityCompression"));
 wetSmooth.setTargetValue(raw("dryWet"));
 outputSmooth.setTargetValue(juce::Decibels::decibelsToGain(raw("output")));
 float peak=0.0f;
 const float fs=static_cast<float>(currentSampleRate);
 const float attack=std::exp(-1.0f/(0.012f*fs));
 const float release=std::exp(-1.0f/(0.180f*fs));
 for(int i=0;i<n;++i){
   const float lpValue=lpSmooth.getNextValue();
   const float hpValue=hpSmooth.getNextValue();
   const float amount=compressionSmooth.getNextValue();
   const float mix=wetSmooth.getNextValue();
   const float outGain=outputSmooth.getNextValue();
   // Logarithmic ranges. At the open end, each filter is smoothly bypassed.
   const float lpHz=40.0f*std::pow(500.0f,lpValue);
   const float hpHz=20.0f*std::pow(500.0f,hpValue);
   const float lpBlend=juce::jlimit(0.0f,1.0f,(1.0f-lpValue)*20.0f);
   const float hpBlend=juce::jlimit(0.0f,1.0f,hpValue*20.0f);
   for(int ch=0;ch<channels;++ch){
     const size_t idx=static_cast<size_t>(juce::jmin(ch,1));
     auto& lp=filterLow[idx];auto& hp=filterHigh[idx];
     lp.setCutoffFrequency(juce::jlimit(20.0f,fs*.45f,lpHz));
     hp.setCutoffFrequency(juce::jlimit(20.0f,fs*.45f,hpHz));
     const float wet=b.getSample(ch,i);
     const float low=lp.processSample(0,wet);
     const float afterLP=wet+lpBlend*(low-wet);
     const float high=hp.processSample(0,afterLP);
     const float filtered=afterLP+hpBlend*(high-afterLP);
     // Feed-forward, soft-knee-like one-knob dynamics. Unity when amount is zero.
     const float magnitude=std::abs(filtered);
     float& env=compressorEnvelope[idx];
     const float coeff=magnitude>env?attack:release;
     env=coeff*env+(1.0f-coeff)*magnitude;
     const float threshold=juce::jmap(amount,0.0f,1.0f,0.75f,0.10f);
     const float ratio=1.0f+amount*5.0f;
     const float gainReduction=env>threshold ? std::pow(juce::jmax(env/threshold,1.0f),1.0f/ratio-1.0f) : 1.0f;
     const float compressed=filtered*gainReduction*(1.0f+0.18f*amount);
     const float out=(compressed*mix+dryBuffer.getSample(ch,i)*(1.0f-mix))*outGain;
     b.setSample(ch,i,out);
     peak=juce::jmax(peak,std::abs(out));
   }
 }
 visualLevel.store(peak,std::memory_order_relaxed);
}
void WraithAudioProcessor::getStateInformation(juce::MemoryBlock& data){
 auto state=parameters.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());copyXmlToBinary(*xml,data);
}
void WraithAudioProcessor::setStateInformation(const void* data,int bytes){
 std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,bytes));
 if(xml&&xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* WraithAudioProcessor::createEditor(){return new WraithAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WraithAudioProcessor();}
