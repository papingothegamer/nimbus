#pragma once

#include "AudioRingBuffer.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <memory>
#include <atomic>

namespace Nimbus {

/**
 * A background thread that reads a specific audio file from disk
 * and pushes it into an AudioRingBuffer lock-free.
 */
class DiskStreamer : public juce::Thread {
public:
    DiskStreamer(const juce::File& file, juce::AudioFormatManager& formatManager);
    ~DiskStreamer() override;

    /**
     * Start the background streaming thread.
     */
    void startStreaming();

    /**
     * Stop the thread safely.
     */
    void stopStreaming();

    /**
     * Pulls audio for the given transport position. Called on the AUDIO THREAD.
     * Lock-free.
     */
    void processBlock(juce::AudioBuffer<float>& buffer, int startSampleInFile, int numSamples);

    /**
     * Notifies the disk thread that the playhead has jumped, 
     * requiring a buffer flush and a seek. Called on the AUDIO THREAD.
     * Lock-free.
     */
    void requestSeek(int newSamplePosition);

    bool isReady() const { return reader != nullptr; }

private:
    void run() override;

    std::unique_ptr<juce::AudioFormatReader> reader;
    std::unique_ptr<AudioRingBuffer> ringBuffer;

    std::atomic<int> readPosition{0};
    std::atomic<int> seekRequest{-1};
    
    juce::AudioBuffer<float> tempReadBuffer;
};

} // namespace Nimbus
