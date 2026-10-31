#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
class WraithAudioProcessor final : public juce::AudioProcessor
{
public:
    WraithAudioProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "WRAITH FX"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 18.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<int> visualNote {60};
    std::atomic<float> visualVelocity {0.0f};
    std::atomic<unsigned int> visualTrigger {0};
    std::atomic<float> visualLevel {0.0f};
private:
    struct ModeState { float y1=0.0f, y2=0.0f; };
    struct Voice {
        std::array<std::array<ModeState,4>,2> modes {};
        int note=-1, channel=1;
        float targetHz=0, currentHz=0, velocity=0, envelope=0, releaseStep=.00001f;
        uint64_t age=0;
        bool active=false, keyDown=false, sustained=false;
    };
    static constexpr int maxVoices=16;
    std::array<Voice,maxVoices> voices {};
    uint64_t voiceAgeCounter=0;
    bool sustainDown=false;
    double currentSampleRate=44100.0;
    float lastPlayedHz=0.0f;
    juce::AudioBuffer<float> dryBuffer;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    std::array<juce::dsp::StateVariableTPTFilter<float>,2> filter;
    juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> filterMorph;
    juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> wetSmooth;
    juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> outputSmooth;
    void noteOn(int,int,float);
    void noteOff(int,int);
    void releaseVoice(Voice&);
    void allNotesOff(bool);
    void renderRange(juce::AudioBuffer<float>&,int,int,float,float,float,float,float,float);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessor)
};
