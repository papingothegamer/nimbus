#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <mutex>
#include <memory>

namespace Nimbus {

class DiskStream {
public:
    DiskStream(const juce::File& file, juce::AudioFormatManager& formatManager);
    ~DiskStream();

    bool isReady() const { return reader != nullptr; }
    int getNumChannels() const { return numChannels; }
    double getSampleRate() const { return sampleRate; }
    juce::int64 getTotalLength() const { return totalLength; }
    const juce::File& getFile() const { return sourceFile; }

    // Called by audio thread
    bool readSamples(juce::AudioBuffer<float>& destBuffer, int destStartSample, juce::int64 sourceStartSample, int numSamples);
    void requestSeek(juce::int64 newPosition);

    // Called by background manager thread
    void processStream();

private:
    juce::File sourceFile;
    std::unique_ptr<juce::AudioFormatReader> reader;
    int numChannels = 0;
    double sampleRate = 0;
    juce::int64 totalLength = 0;

    juce::AudioBuffer<float> ringBuffer;
    int ringBufferSize = 0;

    std::atomic<juce::int64> readPosition { 0 };
    std::atomic<juce::int64> writePosition { 0 };

    std::atomic<bool> seekRequested { false };
    std::atomic<juce::int64> seekTarget { 0 };
    
    juce::CriticalSection readerLock;
};

} // namespace Nimbus
