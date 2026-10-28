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
    layout.add(std::make_unique<juce::AudioParameterFloat>("grainMix", "Grain Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("grainSize", "Grain Size",
        juce::NormalisableRange<float>(25.0f, 280.0f, 1.0f, 0.55f), 115.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("grainDensity", "Grain Density",
        juce::NormalisableRange<float>(2.0f, 32.0f, 0.1f), 13.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("grainTone", "Grain Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.48f));
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
    createGrainSource();
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
    v.pluckEnvelope = 1.0f;
    v.grainSpawnCountdown = 0.0;
    v.nextGrainSlot = 0;
    v.randomState = 0x9e3779b9u ^ (static_cast<uint32_t>(note) * 0x85ebca6bu)
                    ^ static_cast<uint32_t>(voiceAgeCounter + 1);
    if (v.randomState == 0) v.randomState = 0x6d2b79f5u;
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

void WraithAudioProcessor::createGrainSource()
{
    // A deterministic, internally generated excitation texture; no external samples.
    juce::Random random(0x57415254);
    float low = 0.0f;
    for (int i = 0; i < grainTableSize; ++i)
    {
        const float white = random.nextFloat() * 2.0f - 1.0f;
        low += (white - low) * 0.075f;
        const float cycle = static_cast<float>(i) / static_cast<float>(grainTableSize);
        const float pitched = 0.30f * std::sin(twoPi * cycle * 17.0f)
                            + 0.18f * std::sin(twoPi * cycle * 43.0f + 0.7f)
                            + 0.10f * std::sin(twoPi * cycle * 91.0f + 1.9f);
        grainSourceSoft[static_cast<size_t>(i)] = juce::jlimit(-1.0f, 1.0f, low * 0.85f + pitched);
        grainSourceBright[static_cast<size_t>(i)] = juce::jlimit(-1.0f, 1.0f, white * 0.72f + pitched * 0.75f);
    }
}

void WraithAudioProcessor::spawnGrain(Voice& voice, float grainSize, float grainTone)
{
    auto& grain = voice.grains[static_cast<size_t>(voice.nextGrainSlot)];
    voice.nextGrainSlot = (voice.nextGrainSlot + 1) % grainsPerVoice;

    auto nextRandom = [&voice]()
    {
        // Per-voice xorshift RNG avoids locks or shared RNG state on the audio thread.
        uint32_t x = voice.randomState;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        voice.randomState = x;
        return static_cast<float>(x & 0x00ffffffu) / 16777215.0f;
    };
    const float start = nextRandom();
    const float noteRate = std::pow(2.0f, (static_cast<float>(voice.note) - 60.0f) / 12.0f);
    const float jitter = 0.82f + nextRandom() * 0.36f;
    grain.position = start * static_cast<float>(grainTableSize);
    grain.increment = juce::jlimit(0.25f, 4.0f, noteRate * jitter);
    grain.pan = juce::jlimit(0.0f, 1.0f, 0.5f + (nextRandom() - 0.5f) * (0.25f + grainTone * 0.7f));
    grain.age = 0;
    grain.length = juce::jmax(16, static_cast<int>(currentSampleRate * (grainSize / 1000.0f)));
    grain.active = true;
}

void WraithAudioProcessor::renderRange(juce::AudioBuffer<float>& buffer, int start, int count,
                                       float resonance, float decay, float motion, float gain,
                                       float grainMix, float grainSize, float grainDensity, float grainTone)
{
    const int channels = buffer.getNumChannels();
    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : nullptr;
    const double grainInterval = currentSampleRate / static_cast<double>(juce::jmax(0.1f, grainDensity));
    const float softWeight = 1.0f - grainTone;

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

            const float body = std::sin(twoPi * v.phase);
            const float metallic = std::sin(twoPi * v.phaseB);
            const float upper = std::sin(twoPi * v.phaseC);
            // The pluck transient is voice state, not block state: MIDI event splits
            // can no longer restart the attack transient mid-note.
            const float pluck = v.pluckEnvelope;
            v.pluckEnvelope *= std::exp(-1.0f / static_cast<float>(juce::jmax(1.0, currentSampleRate * 0.025)));
            const float resonant = (body * (0.62f - resonance * 0.12f)
                                  + metallic * (0.12f + resonance * 0.32f)
                                  + upper * (0.06f + resonance * 0.18f)
                                  + pluck * 0.08f) * v.envelope * v.velocity;
            const float pan = 0.5f + 0.42f * std::sin(twoPi * v.phase * (0.07f + motion * 0.18f));
            float grainLeft = 0.0f;
            float grainRight = 0.0f;

            if (grainMix > 0.0001f && v.envelope > 0.0001f)
            {
                v.grainSpawnCountdown -= 1.0;
                if (v.grainSpawnCountdown <= 0.0)
                {
                    spawnGrain(v, grainSize, grainTone);
                    v.grainSpawnCountdown += grainInterval;
                }

                for (auto& grain : v.grains)
                {
                    if (!grain.active) continue;
                    const float norm = static_cast<float>(grain.age) / static_cast<float>(grain.length);
                    if (norm >= 1.0f)
                    {
                        grain.active = false;
                        continue;
                    }
                    const int index0 = static_cast<int>(grain.position) & (grainTableSize - 1);
                    const int index1 = (index0 + 1) & (grainTableSize - 1);
                    const float frac = grain.position - std::floor(grain.position);
                    const float soft = grainSourceSoft[static_cast<size_t>(index0)] * (1.0f - frac)
                                     + grainSourceSoft[static_cast<size_t>(index1)] * frac;
                    const float bright = grainSourceBright[static_cast<size_t>(index0)] * (1.0f - frac)
                                       + grainSourceBright[static_cast<size_t>(index1)] * frac;
                    const float window = 0.5f - 0.5f * std::cos(twoPi * norm);
                    const float sample = (soft * softWeight + bright * grainTone) * window;
                    grainLeft += sample * std::sqrt(1.0f - grain.pan);
                    grainRight += sample * std::sqrt(grain.pan);
                    grain.position += grain.increment;
                    while (grain.position >= static_cast<float>(grainTableSize)) grain.position -= static_cast<float>(grainTableSize);
                    ++grain.age;
                }
            }

            const float grainLevel = grainMix * 0.16f * v.envelope * v.velocity;
            l += resonant * std::sqrt(1.0f - pan) * (1.0f - grainMix * 0.35f) + grainLeft * grainLevel;
            r += resonant * std::sqrt(pan) * (1.0f - grainMix * 0.35f) + grainRight * grainLevel;

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
                    for (auto& grain : v.grains) grain.active = false;
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
    const float grainMix = parameters.getRawParameterValue("grainMix")->load();
    const float grainSize = parameters.getRawParameterValue("grainSize")->load();
    const float grainDensity = parameters.getRawParameterValue("grainDensity")->load();
    const float grainTone = parameters.getRawParameterValue("grainTone")->load();
    const float gain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());

    int renderedUntil = 0;
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int eventSample = juce::jlimit(renderedUntil, buffer.getNumSamples(),
                                             metadata.samplePosition);
        if (eventSample > renderedUntil)
            renderRange(buffer, renderedUntil, eventSample - renderedUntil,
                        resonance, decay, motion, gain, grainMix, grainSize, grainDensity, grainTone);

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
                    resonance, decay, motion, gain, grainMix, grainSize, grainDensity, grainTone);

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
