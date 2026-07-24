#pragma once
#include "../IStockPlugin.h"
#include <vector>
#include <algorithm>

namespace Nimbus {

class MidiArpeggiatorPlugin : public IStockPlugin {
public:
    MidiArpeggiatorPlugin();
    ~MidiArpeggiatorPlugin() override = default;

    juce::String getName() const override { return "Arpeggiator"; }
    juce::String getCategory() const override { return "MIDI Effects"; }
    bool isMidiEffect() const override { return true; }
    
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed; }
    void setBypassed(bool b) override { bypassed = b; }

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    int getLatencySamples() const override { return 0; }

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool bypassed = false;
    double rateInSeconds = 0.25; // Default to 16th notes at 60BPM approx

private:
    double currentSampleRate = 44100.0;
    
    std::vector<int> heldNotes;
    int currentNoteIndex = 0;
    int lastPlayedNote = -1;
    double samplesSinceLastNote = 0.0;
};

} // namespace Nimbus
