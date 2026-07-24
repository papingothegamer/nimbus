#pragma once
#include "../IStockPlugin.h"

namespace Nimbus {

class MidiPitchPlugin : public IStockPlugin {
public:
    MidiPitchPlugin();
    ~MidiPitchPlugin() override = default;

    juce::String getName() const override { return "Pitch"; }
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
    int pitchShift = 12; // Default to +1 octave

private:
    double currentSampleRate = 44100.0;
};

} // namespace Nimbus
