#pragma once
#include "../IStockPlugin.h"
#include <vector>

namespace Nimbus {

class MidiChordPlugin : public IStockPlugin {
public:
    MidiChordPlugin();
    ~MidiChordPlugin() override = default;

    juce::String getName() const override { return "Chord"; }
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

private:
    bool bypassed = false;
    double currentSampleRate = 44100.0;
    std::vector<int> chordIntervals = { 4, 7 }; // Default to Major triad (+4, +7)
};

} // namespace Nimbus
