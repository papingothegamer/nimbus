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
    
    void setPosition(double samplePosition) override;
    double getCurrentPosition() const override;
    
    bool isPlaying() const override;
    bool isRecording() const override;

    double getSampleRate() const override;
    double getTempo() const override;

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
};

} // namespace Nimbus
