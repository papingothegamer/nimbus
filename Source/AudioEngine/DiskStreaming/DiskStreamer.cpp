#include "DiskStreamer.h"

namespace Nimbus {

// 1 second of buffering at 48kHz
static constexpr int RING_BUFFER_SIZE = 48000;
// Read in chunks of 4096
static constexpr int READ_CHUNK_SIZE = 4096;

DiskStreamer::DiskStreamer(const juce::File& file, juce::AudioFormatManager& formatManager)
    : juce::Thread("DiskStreamerThread") {
    
    reader.reset(formatManager.createReaderFor(file));
    if (reader) {
        int channels = reader->numChannels;
        if (channels == 0) channels = 2; // fallback

        ringBuffer = std::make_unique<AudioRingBuffer>(channels, RING_BUFFER_SIZE);
        tempReadBuffer.setSize(channels, READ_CHUNK_SIZE);
    }
}

DiskStreamer::~DiskStreamer() {
    stopStreaming();
}

void DiskStreamer::startStreaming() {
    if (reader) {
        startThread(juce::Thread::Priority::normal);
    }
}

void DiskStreamer::stopStreaming() {
    stopThread(2000); // Wait up to 2s
}

void DiskStreamer::processBlock(juce::AudioBuffer<float>& buffer, int startSampleInFile, int numSamples) {
    if (!ringBuffer) {
        buffer.clear();
        return;
    }

    // Expected position vs actual read position
    // If the transport jumped, we must request a seek
    int expectedPosition = startSampleInFile;
    int currentReadPos = readPosition.load(std::memory_order_relaxed);

    if (currentReadPos != expectedPosition) {
        // We are out of sync. Request a seek.
        // We only request if we haven't already requested this exact position recently.
        int currentSeek = seekRequest.load(std::memory_order_relaxed);
        if (currentSeek != expectedPosition) {
            seekRequest.store(expectedPosition, std::memory_order_release);
            notify(); // Wake up the disk thread
        }
        buffer.clear();
        return; // Underrun / waiting for seek
    }

    // Normal continuous read
    int read = ringBuffer->read(buffer, numSamples);
    if (read < numSamples) {
        // Underrun! We read what we could, but the buffer zeroed the rest.
        // The background thread might be falling behind.
        notify(); // Kick the thread
    }
    
    readPosition.store(currentReadPos + numSamples, std::memory_order_relaxed);
    notify(); // Kick the thread to read more
}

void DiskStreamer::requestSeek(int newSamplePosition) {
    seekRequest.store(newSamplePosition, std::memory_order_release);
    notify();
}

void DiskStreamer::run() {
    int filePosition = 0;

    while (!threadShouldExit()) {
        int seekTo = seekRequest.exchange(-1, std::memory_order_acquire);
        
        if (seekTo >= 0) {
            // Perform the seek
            filePosition = seekTo;
            ringBuffer->reset();
            readPosition.store(filePosition, std::memory_order_relaxed);
            // We don't read immediately, we'll let the rest of the loop handle it
        }

        int freeSpace = ringBuffer->getFreeSpace();
        
        if (freeSpace >= READ_CHUNK_SIZE) {
            // Read a chunk from disk
            tempReadBuffer.clear();
            bool success = reader->read(&tempReadBuffer, 0, READ_CHUNK_SIZE, filePosition, true, true);
            
            if (success) {
                ringBuffer->write(tempReadBuffer, READ_CHUNK_SIZE);
                filePosition += READ_CHUNK_SIZE;
            } else {
                // EOF or error, wait a bit longer to prevent spinning
                wait(10);
            }
        } else {
            // Buffer is full enough, go to sleep until woken up by the audio thread
            wait(5);
        }
    }
}

} // namespace Nimbus
