#pragma once

#include "DiskStreamer.h"
#include "signalsmith-stretch.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <vector>

namespace Nimbus {

class TimeStretchReader {
public:
    TimeStretchReader(std::shared_ptr<DiskStreamer> sourceStreamer);
    ~TimeStretchReader() = default;

    void prepare(double sampleRate, int samplesPerBlock);

    void setPitchShift(double semitones);
    void setTimeStretchRatio(double ratio); // 1.0 = normal, 2.0 = half speed
    
    // Reads a block of audio. The startSample is in the timeline's domain.
    // The reader will pull the necessary amount of audio from the underlying DiskStreamer.
    void readBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, bool shouldLoop, double loopStart, double loopEnd);

private:
    std::shared_ptr<DiskStreamer> streamer;
    signalsmith::stretch::SignalsmithStretch<float> stretcher;
    
    double currentSampleRate = 44100.0;
    double pitchShiftSemitones = 0.0;
    double timeRatio = 1.0;
    
    juce::AudioBuffer<float> inputBuffer;
    std::vector<const float*> inputChannels;
    std::vector<float*> outputChannels;
    
    double currentSourcePosition = 0.0;
    bool isFirstRead = true;
};

} // namespace Nimbus
