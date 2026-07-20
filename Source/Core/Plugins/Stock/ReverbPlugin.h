#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace Nimbus {

class ReverbPlugin : public IStockPlugin {
public:
    ReverbPlugin();
    ~ReverbPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Reverb"; }
    juce::String getCategory() const override { return "Reverb"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameters (Audacity matching)
    void setRoomSize(float v) { roomSize.store(v); updateDSP(); }
    float getRoomSize() const { return roomSize.load(); }

    void setStereoWidth(float v) { stereoWidth.store(v); updateDSP(); }
    float getStereoWidth() const { return stereoWidth.load(); }

    void setPreDelay(float v) { preDelay.store(v); updateDSP(); }
    float getPreDelay() const { return preDelay.load(); }

    void setDamping(float v) { damping.store(v); updateDSP(); }
    float getDamping() const { return damping.load(); }

    void setReverberance(float v) { reverberance.store(v); updateDSP(); }
    float getReverberance() const { return reverberance.load(); }

    void setLowTone(float v) { lowTone.store(v); updateDSP(); }
    float getLowTone() const { return lowTone.load(); }

    void setHighTone(float v) { highTone.store(v); updateDSP(); }
    float getHighTone() const { return highTone.load(); }

    void setWetGain(float v) { wetGain.store(v); updateDSP(); }
    float getWetGain() const { return wetGain.load(); }

    void setDryGain(float v) { dryGain.store(v); updateDSP(); }
    float getDryGain() const { return dryGain.load(); }

    void setWetOnly(bool v) { wetOnly.store(v); updateDSP(); }
    bool getWetOnly() const { return wetOnly.load(); }

    void loadPreset(int index);

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    
    std::atomic<float> roomSize{75.0f}; // 0-100
    std::atomic<float> stereoWidth{100.0f}; // 0-100
    std::atomic<float> preDelay{10.0f}; // 0-200ms
    std::atomic<float> damping{50.0f}; // 0-100
    std::atomic<float> reverberance{50.0f}; // 0-100
    std::atomic<float> lowTone{100.0f}; // 0-100
    std::atomic<float> highTone{100.0f}; // 0-100
    std::atomic<float> wetGain{-1.0f}; // -20 to 10 dB
    std::atomic<float> dryGain{-1.0f}; // -20 to 10 dB
    std::atomic<bool> wetOnly{false};

    double currentSampleRate = 44100.0;

    juce::dsp::DelayLine<float> preDelayLine{192000}; // max 1 second at 192kHz
    juce::dsp::Reverb dspReverb;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowShelf;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highShelf;

    juce::SpinLock processLock;
};

} // namespace Nimbus
