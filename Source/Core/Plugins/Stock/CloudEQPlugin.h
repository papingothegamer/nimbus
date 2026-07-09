#pragma once

#include "Core/Plugins/IStockPlugin.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <vector>

namespace Nimbus {

class CloudEQPlugin : public IStockPlugin {
public:
    CloudEQPlugin();
    ~CloudEQPlugin() override = default;

    // IStockPlugin
    juce::String getName() const override { return "Cloud EQ"; }
    juce::String getCategory() const override { return "EQ & Filters"; }
    juce::Component* createEditor() override;
    
    bool isBypassed() const override { return bypassed.load(); }
    void setBypassed(bool b) override { bypassed.store(b); }

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Parameters
    enum class FilterType {
        LowCut,
        LowShelf,
        Bell,
        HighShelf,
        HighCut
    };

    struct BandParams {
        std::atomic<FilterType> type;
        std::atomic<float> freq;
        std::atomic<float> q;
        std::atomic<float> gainDb;
        std::atomic<bool> enabled;
    };

    BandParams& getBand(int index) { return bands[index]; }
    void updateDSP();

    // FFT Data for Live Spectrum
    static constexpr int fftOrder = 11; // 2048 size
    static constexpr int fftSize = 1 << fftOrder;
    void copyFFTData(float* dest) {
        for (int i = 0; i < fftSize * 2; ++i) {
            dest[i] = scopeData[i];
        }
    }
    bool isNextFFTBlockReady() const { return nextFFTBlockReady; }
    void setNextFFTBlockReady(bool b) { nextFFTBlockReady = b; }

    juce::dsp::FFT forwardFFT{fftOrder};
    juce::dsp::WindowingFunction<float> window{fftSize, juce::dsp::WindowingFunction<float>::hann};

private:
    void pushNextSampleIntoFifo(float sample) noexcept;

    std::atomic<bool> bypassed{false};
    std::array<BandParams, 8> bands;
    
    // We explicitly manage left and right filters to avoid ProcessorDuplicator issues
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    std::array<IIRFilter, 8> leftFilters;
    std::array<IIRFilter, 8> rightFilters;
    
    double currentSampleRate = 44100.0;
    juce::SpinLock processLock;
    
    std::vector<float> fifo;
    std::vector<float> scopeData;
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady{false};
};

} // namespace Nimbus
