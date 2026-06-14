#pragma once

#include "AudioEngine/IAudioNode.h"
#include "DataModel/AudioClip.h"
#include "DiskStreaming/DiskStreamer.h"
#include "Transport.h"
#include <memory>

namespace Nimbus {

/**
 * An audio node that renders an AudioClip by reading from a DiskStreamer.
 * Position-aware via the global Transport.
 */
class AudioClipNode : public IAudioNode {
public:
    AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStreamer> streamer, Transport& transport);
    ~AudioClipNode() override = default;

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    std::shared_ptr<AudioClip> clipModel;
    std::shared_ptr<DiskStreamer> diskStreamer;
    Transport& globalTransport;

    int lastProcessedTransportPos = -1;
};

} // namespace Nimbus
