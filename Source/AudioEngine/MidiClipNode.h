#pragma once

#include "AudioEngine/IAudioNode.h"
#include "AudioEngine/ITransport.h"
#include "DataModel/MidiClip.h"
#include <memory>

namespace Nimbus {

/**
 * Reads a MidiClip, outputs MIDI buffers, and optionally bounces to audio.
 */
class MidiClipNode : public IAudioNode {
public:
    MidiClipNode(std::shared_ptr<MidiClip> clip, ITransport& transport);
    ~MidiClipNode() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    std::shared_ptr<MidiClip> midiClip;
    ITransport& transport;
    double sampleRate_{44100.0};
    float currentPeak{0.0f};

    // Framework for bouncing:
    // If a synth plugin is attached to this node or track, the MIDI buffer
    // is passed to the synth. To bounce to audio seamlessly, we can render 
    // the synth output offline and replace this node with an AudioClipNode.
};

} // namespace Nimbus
