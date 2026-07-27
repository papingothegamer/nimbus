#pragma once

#include "AudioEngine/IAudioNode.h"
#include "DataModel/AudioClip.h"
#include "DiskStreaming/DiskStreamer.h"
#include "Transport.h"
#include <JuceHeader.h>
#include <memory>
#include "GranularTimeStretcher.h"
#include <atomic>

namespace Nimbus {

struct AudioClipRenderState {
    std::atomic<int> startSample{0};
    std::atomic<int> lengthSamples{0};
    std::atomic<int> sourceOffsetSamples{0};
    std::atomic<double> speedMultiplier{1.0};
    std::atomic<double> pitchShiftSemitones{0.0};
    std::atomic<float> gain{1.0f};
    std::atomic<int> fadeInSamples{0};
    std::atomic<int> fadeOutSamples{0};
    std::atomic<float> fadeInCurve{1.0f};
    std::atomic<float> fadeOutCurve{1.0f};
    std::atomic<bool> isCrossfadingIn{false};
    std::atomic<bool> isCrossfadingOut{false};
    std::atomic<bool> preservePitch{false};
    std::atomic<bool> matchDawTempo{false};
    std::atomic<double> originalBpm{120.0};
};

/**
 * An audio node that renders an AudioClip by reading from a DiskStreamer.
 * Position-aware via the global Transport.
 */
class AudioClipNode : public IAudioNode, private juce::Timer {
public:
    AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStreamer> streamer, Transport& transport);
    ~AudioClipNode() override;

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Called periodically by the UI/Model thread to push state down safely
    void syncStateFromModel();
    
    // Timer
    void timerCallback() override { syncStateFromModel(); }

private:
    std::shared_ptr<AudioClip> clipModel;
    std::shared_ptr<DiskStreamer> diskStreamer;
    Transport& globalTransport;

    AudioClipRenderState renderState;

    int lastProcessedTransportPos = -1;

    juce::LagrangeInterpolator interpolatorLeft;
    juce::LagrangeInterpolator interpolatorRight;
    GranularTimeStretcher granularStretcher;
    juce::AudioBuffer<float> readBuffer;
};

} // namespace Nimbus
