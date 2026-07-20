#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class GainPlugin : public IStockPlugin {
public:
    GainPlugin();
    ~GainPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Gain"; }
    juce::String getCategory() const override { return "Utility"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameters
    void setGainDecibels(float db) { gainDb.store(db); updateDSP(); }
    float getGainDecibels() const { return gainDb.load(); }
    
    void setPhaseInvert(bool invert) { phaseInvert.store(invert); updateDSP(); }
    bool getPhaseInvert() const { return phaseInvert.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> gainDb{0.0f};
    std::atomic<bool> phaseInvert{false};
    
    juce::dsp::Gain<float> dspGain;
    
    juce::SpinLock processLock;
};

} // namespace Nimbus
