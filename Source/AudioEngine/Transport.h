#pragma once

#include "ITransport.h"
#include <atomic>

namespace Nimbus {

class Transport : public ITransport {
public:
    Transport();
    ~Transport() override = default;

    // ITransport
    void play() override;
    void stop() override;
    void record() override;
    void stopRecording();
    
    void setPosition(double samplePosition) override;
    double getCurrentPosition() const override;
    int getCurrentPositionSamples() const;
    
    bool isPlaying() const override;
    bool isRecording() const override;

    bool isLooping() const;
    void setLooping(bool shouldLoop);
    
    void setLoopRegion(double startSamples, double endSamples);
    double getLoopStartSamples() const;
    double getLoopEndSamples() const;

    double getSampleRate() const override;
    double getTempo() const override;
    void setTempo(double newTempo) override;

    int getTimeSignatureNumerator() const override;
    int getTimeSignatureDenominator() const override;
    void setTimeSignature(int numerator, int denominator) override;

    // Called by the Audio Engine to advance the clock
    void advancePosition(int numSamples);
    
    // Called by the Audio Engine on sample rate change
    void setSampleRate(double newSampleRate);

private:
    std::atomic<bool> playing{false};
    std::atomic<bool> recording{false};
    std::atomic<double> currentPosition{0.0};
    std::atomic<double> sampleRate{44100.0};
    std::atomic<double> tempo{120.0};
    std::atomic<int> timeSigNumerator{4};
    std::atomic<int> timeSigDenominator{4};
    
    std::atomic<bool> looping{false};
    std::atomic<double> loopStartSamples{0.0};
    std::atomic<double> loopEndSamples{0.0};
};

} // namespace Nimbus
