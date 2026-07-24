#pragma once
#include "../IStockPlugin.h"
#include <vector>

namespace Nimbus {

class MidiScalePlugin : public IStockPlugin {
public:
    MidiScalePlugin();
    ~MidiScalePlugin() override = default;

    juce::String getName() const override { return "Scale"; }
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
    
    // 12-element array representing active notes in the scale (C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
    std::vector<bool> scaleMask = {
        true, false, true, false, true, true, false, true, false, true, false, true
    }; // Default to C Major
};

} // namespace Nimbus
