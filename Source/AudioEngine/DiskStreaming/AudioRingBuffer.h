#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace Nimbus {

/**
 * A lock-free ring buffer designed for streaming multi-channel audio data.
 * Safe for one reader (audio thread) and one writer (disk thread).
 */
class AudioRingBuffer {
public:
    AudioRingBuffer(int numChannels, int sizeInSamples)
        : fifo(sizeInSamples), channels(numChannels), bufferSize(sizeInSamples) {
        
        buffer.setSize(numChannels, sizeInSamples);
        buffer.clear();
    }

    ~AudioRingBuffer() = default;

    /**
     * Push audio data into the ring buffer. (Called by disk thread).
     * Returns the number of samples successfully written.
     */
    int write(const juce::AudioBuffer<float>& sourceBuffer, int numSamplesToWrite) {
        jassert(sourceBuffer.getNumChannels() == channels);

        int start1, size1, start2, size2;
        fifo.prepareToWrite(numSamplesToWrite, start1, size1, start2, size2);

        if (size1 > 0) {
            for (int ch = 0; ch < channels; ++ch) {
                buffer.copyFrom(ch, start1, sourceBuffer, ch, 0, size1);
            }
        }
        if (size2 > 0) {
            for (int ch = 0; ch < channels; ++ch) {
                buffer.copyFrom(ch, start2, sourceBuffer, ch, size1, size2);
            }
        }

        fifo.finishedWrite(size1 + size2);
        return size1 + size2;
    }

    /**
     * Pop audio data out of the ring buffer. (Called by audio thread).
     * If there isn't enough data, the remaining destBuffer will be zeroed (underrun).
     * Returns the number of samples successfully read.
     */
    int read(juce::AudioBuffer<float>& destBuffer, int numSamplesToRead) {
        int start1, size1, start2, size2;
        fifo.prepareToRead(numSamplesToRead, start1, size1, start2, size2);

        int destChannels = std::min(destBuffer.getNumChannels(), channels);

        if (size1 > 0) {
            for (int ch = 0; ch < destChannels; ++ch) {
                destBuffer.copyFrom(ch, 0, buffer, ch, start1, size1);
            }
        }
        if (size2 > 0) {
            for (int ch = 0; ch < destChannels; ++ch) {
                destBuffer.copyFrom(ch, size1, buffer, ch, start2, size2);
            }
        }

        fifo.finishedRead(size1 + size2);

        int totalRead = size1 + size2;
        if (totalRead < numSamplesToRead) {
            // Buffer underrun, zero out the rest
            for (int ch = 0; ch < destBuffer.getNumChannels(); ++ch) {
                destBuffer.clear(ch, totalRead, numSamplesToRead - totalRead);
            }
        }

        return totalRead;
    }

    int getNumReady() const {
        return fifo.getNumReady();
    }

    int getFreeSpace() const {
        return fifo.getFreeSpace();
    }

    void reset() {
        fifo.reset();
        buffer.clear();
    }

private:
    juce::AbstractFifo fifo;
    juce::AudioBuffer<float> buffer;
    int channels;
    int bufferSize;
};

} // namespace Nimbus
