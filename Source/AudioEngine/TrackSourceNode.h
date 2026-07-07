#pragma once

#include "AudioEngine/IAudioNode.h"
#include "AudioEngine/AudioClipNode.h"
#include "AudioEngine/MidiClipNode.h"
#include "DataModel/TimelineProject.h"
#include "AudioEngine/Transport.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <memory>
#include <juce_core/juce_core.h>

namespace Nimbus {

class TrackSourceNode : public IAudioNode {
public:
    TrackSourceNode(Transport& t, juce::AudioFormatManager& fm);
    ~TrackSourceNode() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    void updateClips(const std::vector<AnyClipPtr>& newClips);

private:
    Transport& transport;
    juce::AudioFormatManager& formatManager;

    std::vector<std::unique_ptr<IAudioNode>> clipNodes;
    juce::SpinLock processLock;

    double currentSampleRate = 0.0;
    int currentBlockSize = 0;
};

} // namespace Nimbus
