#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr float twoPi = juce::MathConstants<float>::twoPi;
    float midiToHz(int note)
    {
        return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    }
}

WraithAudioProcessor::WraithAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "WRAITH_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout WraithAudioProcessor::createParameterLayout()
{
    using P = juce::AudioProcessorValueTreeState;
    P::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.62f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay",
        juce::NormalisableRange<float>(0.15f, 18.0f, 0.01f, 0.45f), 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("space", "Space",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.42f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("motion", "Motion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.28f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("output", "Output",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f), -8.0f));
    return layout;
}

void WraithAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(juce::jmax(1, samplesPerBlock)),
                                  static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    reverb.reset();
    reverbParams.roomSize = 0.92f;
    reverbParams.damping = 0.38f;
    reverbParams.wetLevel = 0.35f;
    reverbParams.dryLevel = 0.80f;
    reverbParams.width = 1.0f;
    reverb.setParameters(reverbParams);
    reverb.prepare(spec);

    for (auto& voice : voices)
        voice = Voice {};
    sustainDown = false;
    voiceAgeCounter = 0;
}

void WraithAudioProcessor::releaseResources()
{
    reverb.reset();
}

bool WraithAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::stereo() || output == juce::AudioChannelSet::mono();
}

void WraithAudioProcessor::noteOn(int note, int channel, float velocity)
{
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

void WraithAudioProcessor::renderRange(juce::AudioBuffer<float>& buffer, int start, int count,
                                       float resonance, float decay, float motion, float gain)
{
    const int channels = buffer.getNumChannels();
    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int s = start; s < start + count; ++s)
    {
        float l = 0.0f;
        float r = 0.0f;

        for (auto& v : voices)
        {
            if (!v.active || v.note < 0) continue;

            const float hz = midiToHz(v.note);
            const float increment = hz / static_cast<float>(currentSampleRate);
            v.phase += increment;
            v.phaseB += increment * (2.006f + motion * 0.018f);
            v.phaseC += increment * 3.997f;
            if (v.phase >= 1.0f) v.phase -= std::floor(v.phase);
            if (v.phaseB >= 1.0f) v.phaseB -= std::floor(v.phaseB);
            if (v.phaseC >= 1.0f) v.phaseC -= std::floor(v.phaseC);

            // Lightweight resonant voice: fundamental plus slightly inharmonic partials.
            // This is intentionally an original synthesis model, not a sample or a clone.
            const float body = std::sin(twoPi * v.phase);
            const float metallic = std::sin(twoPi * v.phaseB);
            const float upper = std::sin(twoPi * v.phaseC);
            const float pluck = std::exp(-static_cast<float>(s - start) /
                                          static_cast<float>(juce::jmax(1.0, currentSampleRate * 0.025)));
            const float tone = (body * (0.62f - resonance * 0.12f)
                              + metallic * (0.12f + resonance * 0.32f)
                              + upper * (0.06f + resonance * 0.18f)
                              + pluck * 0.08f) * v.envelope * v.velocity;

            const float pan = 0.5f + 0.42f * std::sin(twoPi * v.phase * (0.07f + motion * 0.18f));
            l += tone * std::sqrt(1.0f - pan);
            r += tone * std::sqrt(pan);

            if (v.keyDown)
                v.envelope += (1.0f - v.envelope) * 0.0025f;
            else if (!v.sustained)
            {
                const float step = 1.0f / static_cast<float>(juce::jmax(1.0, currentSampleRate * decay));
                v.envelope -= juce::jmax(step, v.releaseStep);
                if (v.envelope <= 0.00002f)
                {
                    v.envelope = 0.0f;
                    v.active = false;
                    v.note = -1;
                }
            }
        }

        left[s] += l * gain;
        if (right != nullptr) right[s] += r * gain;
    }
}

void WraithAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const float resonance = parameters.getRawParameterValue("resonance")->load();
    const float decay = parameters.getRawParameterValue("decay")->load();
    const float space = parameters.getRawParameterValue("space")->load();
    const float motion = parameters.getRawParameterValue("motion")->load();
    const float gain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());

    int renderedUntil = 0;
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int eventSample = juce::jlimit(renderedUntil, buffer.getNumSamples(),
                                             metadata.samplePosition);
        if (eventSample > renderedUntil)
            renderRange(buffer, renderedUntil, eventSample - renderedUntil,
                        resonance, decay, motion, gain);

        if (msg.isNoteOn())
            noteOn(msg.getNoteNumber(), msg.getChannel(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            noteOff(msg.getNoteNumber(), msg.getChannel());
        else if (msg.isController())
        {
            if (msg.getControllerNumber() == 64)
            {
                const bool newSustain = msg.getControllerValue() >= 64;
                if (sustainDown && !newSustain)
                {
                    for (auto& voice : voices)
                        if (voice.active && voice.sustained && !voice.keyDown)
                            releaseVoice(voice);
                }
                sustainDown = newSustain;
            }
            else if (msg.getControllerNumber() == 120)
                allNotesOff(true);
            else if (msg.getControllerNumber() == 123)
                allNotesOff(false);
        }

        renderedUntil = eventSample;
    }

    if (renderedUntil < buffer.getNumSamples())
        renderRange(buffer, renderedUntil, buffer.getNumSamples() - renderedUntil,
                    resonance, decay, motion, gain);

    if (buffer.getNumChannels() > 1)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        reverbParams.wetLevel = space * 0.78f;
        reverbParams.dryLevel = 1.0f - space * 0.32f;
        reverb.setParameters(reverbParams);
        reverb.process(context);
    }
}

juce::AudioProcessorEditor* WraithAudioProcessor::createEditor()
{
    return new WraithAudioProcessorEditor(*this);
}

void WraithAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WraithAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WraithAudioProcessor();
}
