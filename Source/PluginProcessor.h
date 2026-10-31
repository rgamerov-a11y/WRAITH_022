#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

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
    const juce::String getName() const override { return JucePlugin_IsSynth ? "WRAITH Instrument" : "WRAITH FX"; }
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

    static constexpr int grainTableSize = 4096;
    static constexpr int grainsPerVoice = 6;

    struct Grain
    {
        double position = 0.0;
        double readStart = 0.0;
        double readEnd = 0.0;
        float increment = 1.0f;
        float pan = 0.5f;
        bool reverse = false;
        bool capturedAudio = false;
        int age = 0;
        int length = 1;
        bool active = false;
    };

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
        float pluckEnvelope = 0.0f;
        double grainSpawnCountdown = 0.0;
        int nextGrainSlot = 0;
        uint32_t randomState = 0x6d2b79f5u;
        std::array<Grain, grainsPerVoice> grains {};
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
                     float resonance, float decay, float motion, float gain,
                     float grainMix, float grainSize, float grainDensity, float grainTone,
                     bool reverseGrains, bool freezeTexture, bool fxMode);
    void spawnGrain(Voice& voice, float grainSize, float grainTone, bool reverse, bool freeze);
    void createGrainSource();
    void captureInputAudio(juce::AudioBuffer<float>& buffer, bool freeze);
    float readCaptureSample(double position, int channel) const noexcept;

    std::array<Voice, maxVoices> voices {};
    uint64_t voiceAgeCounter = 0;
    bool sustainDown = false;
    double currentSampleRate = 44100.0;
    std::array<float, grainTableSize> grainSourceSoft {};
    std::array<float, grainTableSize> grainSourceBright {};
    std::array<std::vector<float>, 2> captureBuffer;
    int captureCapacity = 0;
    uint64_t captureWriteCounter = 0;
    uint64_t frozenEndCounter = 0;
    bool previousFreezeState = false;
    juce::AudioBuffer<float> dryBuffer;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WraithAudioProcessor)
};
