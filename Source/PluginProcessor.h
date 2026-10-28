#pragma once
#include <JuceHeader.h>
#include <array>

class WraithAudioProcessor final : public juce::AudioProcessor
{
public:
    WraithAudioProcessor();
    ~WraithAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "WRAITH"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 30.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    static constexpr int maxVoices = 16;

    struct Voice
    {
        int note = -1;
        int channel = 1;
        float phase = 0.0f;
        float phaseB = 0.0f;
        float phaseC = 0.0f;
        float envelope = 0.0f;
        float velocity = 0.0f;
        float releaseStep = 0.00001f;
        uint64_t age = 0;
        bool active = false;
        bool keyDown = false;
        bool sustained = false;
    };

    void noteOn(int note, int channel, float velocity);
    void noteOff(int note, int channel);
    void releaseVoice(Voice& voice);
    void allNotesOff(bool immediate);
    void renderRange(juce::AudioBuffer<float>& buffer, int start, int count,
                     float resonance, float decay, float motion, float gain);

    std::array<Voice, maxVoices> voices {};
    uint64_t voiceAgeCounter = 0;
    bool sustainDown = false;
    double currentSampleRate = 44100.0;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessor)
};
