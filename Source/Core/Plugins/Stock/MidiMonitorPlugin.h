#pragma once
#include "../IStockPlugin.h"

namespace Nimbus {

class MidiMonitorPlugin : public IStockPlugin {
public:
    MidiMonitorPlugin();
    ~MidiMonitorPlugin() override = default;

    juce::String getName() const override { return "MIDI Monitor"; }
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
};

} // namespace Nimbus
