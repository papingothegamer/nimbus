#pragma once

#include "Nodes/Node.h"
#include "DataModel/AudioClip.h"
#include "DiskStreaming/DiskStreamer.h"
#include "DiskStreaming/TimeStretchReader.h"
#include "Transport.h"
#include "DataModel/TimelineProject.h"
#include <JuceHeader.h>
#include <memory>

namespace Nimbus {

/**
 * An audio node that renders an AudioClip by reading from a DiskStreamer.
 * Position-aware via the global Transport.
 */
class AudioClipNode : public Node {
public:
    AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStreamer> streamer, Transport& transport);
    ~AudioClipNode() override = default;

    // Node
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources();
    void process(const ProcessContext& context) override;

private:
    std::shared_ptr<AudioClip> clipModel;
    std::shared_ptr<DiskStreamer> diskStreamer;
    std::unique_ptr<TimeStretchReader> timeStretchReader;

    int lastProcessedTransportPos = -1;

    Transport& globalTransport;
    
    juce::LagrangeInterpolator interpolatorLeft;
    juce::LagrangeInterpolator interpolatorRight;
    juce::AudioBuffer<float> readBuffer;
};

} // namespace Nimbus
