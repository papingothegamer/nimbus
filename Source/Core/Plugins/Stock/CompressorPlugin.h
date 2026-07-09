#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class CompressorPlugin : public IStockPlugin {
public:
    CompressorPlugin();
    ~CompressorPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Compressor"; }
    juce::String getCategory() const override { return "Dynamics"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Parameters
    void setThreshold(float db) { threshold.store(db); updateDSP(); }
    float getThreshold() const { return threshold.load(); }
    
    void setRatio(float r) { ratio.store(r); updateDSP(); }
    float getRatio() const { return ratio.load(); }
    
    void setAttack(float ms) { attack.store(ms); updateDSP(); }
    float getAttack() const { return attack.load(); }
    
    void setRelease(float ms) { release.store(ms); updateDSP(); }
    float getRelease() const { return release.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> threshold{-20.0f};
    std::atomic<float> ratio{2.0f};
    std::atomic<float> attack{10.0f};
    std::atomic<float> release{100.0f};
    
    juce::dsp::Compressor<float> dspComp;
    
    juce::SpinLock processLock;
};

} // namespace Nimbus
