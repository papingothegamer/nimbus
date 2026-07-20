#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class FilterPlugin : public IStockPlugin {
public:
    FilterPlugin();
    ~FilterPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Filter"; }
    juce::String getCategory() const override { return "EQ & Filters"; }
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
    void setCutoffFrequency(float freq) { cutoff.store(freq); updateDSP(); }
    float getCutoffFrequency() const { return cutoff.load(); }
    
    void setResonance(float res) { resonance.store(res); updateDSP(); }
    float getResonance() const { return resonance.load(); }
    
    void setFilterType(int type) { filterType.store(type); updateDSP(); }
    int getFilterType() const { return filterType.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> cutoff{1000.0f};
    std::atomic<float> resonance{0.707f};
    std::atomic<int> filterType{0}; // 0 = Lowpass, 1 = Highpass, 2 = Bandpass
    
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> dspFilters;
    
    double currentSampleRate = 44100.0;
    juce::SpinLock processLock;
};

} // namespace Nimbus
