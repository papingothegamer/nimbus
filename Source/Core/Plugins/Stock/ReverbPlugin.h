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

    // Parameters
    void setRoomSize(float s) { roomSize.store(s); updateDSP(); }
    float getRoomSize() const { return roomSize.load(); }
    
    void setDamping(float d) { damping.store(d); updateDSP(); }
    float getDamping() const { return damping.load(); }
    
    void setWidth(float w) { width.store(w); updateDSP(); }
    float getWidth() const { return width.load(); }
    
    void setMix(float m) { mix.store(m); updateDSP(); }
    float getMix() const { return mix.load(); }

private:
    void updateDSP();

    std::atomic<bool> bypassed{false};
    std::atomic<float> roomSize{0.5f};
    std::atomic<float> damping{0.5f};
    std::atomic<float> width{1.0f};
    std::atomic<float> mix{0.33f};
    
    juce::dsp::Reverb dspReverb;
    
    juce::SpinLock processLock;
};

} // namespace Nimbus
