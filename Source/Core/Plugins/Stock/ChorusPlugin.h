#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class ChorusPlugin : public IStockPlugin {
public:
    ChorusPlugin();
    ~ChorusPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Chorus"; }
    juce::String getCategory() const override { return "Modulation"; }
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
    void setRate(float rate) { this->rate.store(rate); updateDSP(); }
    float getRate() const { return rate.load(); }
    
    void setDepth(float d) { depth.store(d); updateDSP(); }
    float getDepth() const { return depth.load(); }
    
    void setMix(float m) { mix.store(m); updateDSP(); }
    float getMix() const { return mix.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> rate{1.0f};
    std::atomic<float> depth{0.25f};
    std::atomic<float> mix{0.5f};
    
    juce::dsp::Chorus<float> dspChorus;
    
    juce::SpinLock processLock;
};

} // namespace Nimbus
