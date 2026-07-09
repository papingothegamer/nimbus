#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class PitchCorrectionPlugin : public IStockPlugin {
public:
    PitchCorrectionPlugin();
    ~PitchCorrectionPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Pitch Correction"; }
    juce::String getCategory() const override { return "Modulation"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Parameters
    void setSpeed(float ms) { speed.store(ms); }
    float getSpeed() const { return speed.load(); }
    
    void setKey(int k) { key.store(k); } // 0=C, 1=C#, etc.
    int getKey() const { return key.load(); }
    
    void setScale(int s) { scale.store(s); } // 0=Major, 1=Minor, 2=Chromatic
    int getScale() const { return scale.load(); }

private:
    std::atomic<bool> bypassed{false};
    std::atomic<float> speed{20.0f}; // Retune speed
    std::atomic<int> key{0};
    std::atomic<int> scale{2}; // Chromatic by default
    
    // Very basic pitch shifting via delay lines (mocking the autotune effect)
    juce::dsp::DelayLine<float> delayLine{48000};
    float phase = 0.0f;
    
    double currentSampleRate = 44100.0;
    juce::SpinLock processLock;
};

} // namespace Nimbus
