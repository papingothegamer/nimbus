#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class DelayPlugin : public IStockPlugin {
public:
    DelayPlugin();
    ~DelayPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Delay"; }
    juce::String getCategory() const override { return "Delay & Loop"; }
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
    void setDelayTimeMs(float ms) { delayTimeMs.store(ms); updateDSP(); }
    float getDelayTimeMs() const { return delayTimeMs.load(); }
    
    void setFeedback(float fb) { feedback.store(fb); }
    float getFeedback() const { return feedback.load(); }
    
    void setMix(float mix) { wetMix.store(mix); }
    float getMix() const { return wetMix.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> delayTimeMs{250.0f};
    std::atomic<float> feedback{0.3f};
    std::atomic<float> wetMix{0.5f};
    
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> dspDelay{96000};
    
    double currentSampleRate = 44100.0;
    juce::SpinLock processLock;
};

} // namespace Nimbus
