#pragma once

#include "IAudioNode.h"
#include "AudioGraph.h"
#include "DSP/GainNode.h"
#include "DSP/LevelMeter.h"
#include <memory>

namespace Nimbus {

/**
 * Represents a single channel strip in the Mixer.
 * Owns an internal AudioGraph for its insert plugins and a GainNode for its volume/pan fader.
 */
class Track : public IAudioNode {
public:
    Track();
    ~Track() override = default;

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    int getLatencySamples() const override;

    /**
     * Set the sound source for this track (e.g. an AudioFileReaderNode, or TestToneNode).
     */
    void setSourceNode(std::unique_ptr<IAudioNode> sourceNode);

    /**
     * Add an insert plugin to this track's chain.
     */
    void addInsertPlugin(std::unique_ptr<IAudioNode> pluginNode);
    void removeInsertPlugin(IAudioNode* pluginNode);
    
    const AudioGraph& getInsertGraph() const { return insertGraph; }

    // Fader Controls
    void setVolume(float gainLinear);
    void setPan(float panValue);

    // Level Metering (Called by UI thread)
    float getPeakLevel() const { return meter.getPeakLevel(); }
    float getRMSLevel() const { return meter.getRMSLevel(); }

private:
    std::unique_ptr<IAudioNode> source;
    AudioGraph insertGraph;
    GainNode fader;
    LevelMeter meter;
    
    // Intermediate buffer to hold this track's isolated audio
    juce::AudioBuffer<float> trackBuffer;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};

} // namespace Nimbus
