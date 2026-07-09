#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>

namespace Nimbus {

class MultibandCompressorPlugin : public IStockPlugin {
public:
    MultibandCompressorPlugin();
    ~MultibandCompressorPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Multiband Comp"; }
    juce::String getCategory() const override { return "Dynamics"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    struct BandParams {
        std::atomic<float> threshold{-40.0f};
        std::atomic<float> ratio{4.0f};
        std::atomic<float> attack{10.0f};
        std::atomic<float> release{100.0f};
        std::atomic<float> gainDb{0.0f};
        std::atomic<float> currentGR{0.0f};
    };

    BandParams& getBand(int index) { return bands[index]; } // 0=Low, 1=Mid, 2=High
    
    void setLowMidFreq(float freq) { lowMidFreq.store(freq); updateDSP(); }
    float getLowMidFreq() const { return lowMidFreq.load(); }
    
    void setMidHighFreq(float freq) { midHighFreq.store(freq); updateDSP(); }
    float getMidHighFreq() const { return midHighFreq.load(); }
    
    void updateDSP();

private:
    std::atomic<bool> bypassed{false};
    std::array<BandParams, 3> bands;
    std::atomic<float> lowMidFreq{150.0f};
    std::atomic<float> midHighFreq{2500.0f};
    
    juce::dsp::LinkwitzRileyFilter<float> lp1, hp1, lp2, hp2;
    std::array<juce::dsp::Compressor<float>, 3> dspComps;
    
    double currentSampleRate = 44100.0;
    juce::SpinLock processLock;
};

} // namespace Nimbus
