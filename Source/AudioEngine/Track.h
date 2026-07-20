#pragma once

#include "IAudioNode.h"
#include "AudioGraph.h"
#include "DSP/GainNode.h"
#include "DSP/LevelMeter.h"
#include "DSP/DelayLine.h"
#include <memory>
#include <atomic>
#include <JuceHeader.h>
#include "DataModel/Models.h"

namespace Nimbus {

class Transport;
class AudioRecorder;
class MidiRecorder;

/**
 * Represents a single channel strip in the Mixer.
 * Owns an internal AudioGraph for its insert plugins and a GainNode for its volume/pan fader.
 */
class Track : public IAudioNode {
public:
    Track(TrackID id, bool isStereo, Transport* t = nullptr);
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
    IAudioNode* getSourceNode() const { return source.get(); }

    /**
     * Add an insert plugin to this track's chain.
     */
    void addInsertPlugin(std::unique_ptr<IAudioNode> pluginNode);
    void removeInsertPlugin(IAudioNode* pluginNode);

    void setInstrumentPlugin(std::unique_ptr<IAudioNode> instrumentNode);
    IAudioNode* getInstrumentPlugin() const { return instrument.get(); }
    
    TrackID getId() const { return id_; }
    
    const AudioGraph& getInsertGraph() const { return insertGraph; }

    void setInputBuffer(const juce::AudioBuffer<float>* inBuf) { inputBufferPtr = inBuf; }
    void setRecorder(AudioRecorder* recorder) { recorder_ = recorder; }
    void setMidiRecorder(MidiRecorder* recorder) { midiRecorder_ = recorder; }

    juce::MidiMessageCollector uiMidiCollector;
    
    // Live MIDI from UI or hardware
    void addLiveMidiMessage(const juce::MidiMessage& msg) {
        uiMidiCollector.addMessageToQueue(msg);
    }

    // Fader Controls
    void setVolume(float gainLinear);
    void setPan(float panValue);

    // Mute / Solo / Arm
    void setMuted(bool muted);
    void setSoloed(bool soloed);
    void setArmed(bool shouldBeArmed);
    
    // Grouping
    void setIsGroup(bool isGroup) { isGroup_ = isGroup; }
    bool isGroup() const { return isGroup_; }
    void setParentGroupId(const TrackID& parentId) { parentGroupId_ = parentId; }
    const TrackID& getParentGroupId() const { return parentGroupId_; }
    void clearGroupBuffer();
    juce::AudioBuffer<float>& getGroupBuffer() { return groupBuffer_; }

    void setInputChannelIndex(int index) { inputChannelIndex_ = index; }
    
    bool isMuted() const { return muted_.load(); }
    bool isSoloed() const { return soloed_.load(); }
    void setSilencedBySolo(bool silenced) { silencedBySolo_.store(silenced); }
    bool isSilencedBySolo() const { return silencedBySolo_.load(); }
    bool isArmed() const { return armed_.load(); }

    // Level Metering (Called by UI thread)
    float getPeakLevel() const { return meter.getPeakLevel(); }
    float getRMSLevel() const { return meter.getRMSLevel(); }

    // PDC (Plugin Delay Compensation)
    void setCompensationDelay(int samples);

private:
    TrackID id_;
    std::unique_ptr<IAudioNode> source;
    std::unique_ptr<IAudioNode> instrument;  // Synth for MIDI tracks
    AudioGraph insertGraph;
    GainNode fader;
    LevelMeter meter;
    DelayLine outputDelayLine;
    
    // Intermediate buffer to hold this track's isolated audio
    juce::AudioBuffer<float> tempBuffer;
    juce::AudioBuffer<float> groupBuffer_;
    juce::AudioBuffer<float> trackBuffer;
    juce::AudioBuffer<float> stereoPanBuffer;
    juce::MidiBuffer trackMidiBuffer;
    const juce::AudioBuffer<float>* inputBufferPtr = nullptr;
    AudioRecorder* recorder_ = nullptr;
    MidiRecorder* midiRecorder_ = nullptr;
    Transport* transport = nullptr;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    TrackID id;
    bool isStereo;
    bool isGroup_ = false;
    TrackID parentGroupId_;
    
    std::atomic<bool> muted_{false};
    std::atomic<bool> soloed_{false};
    std::atomic<bool> silencedBySolo_{false};
    std::atomic<bool> armed_{false};
    bool isStereo_ = true;
    std::atomic<int> inputChannelIndex_{-1};
    juce::SpinLock processLock;
};

} // namespace Nimbus
