#include "DiskStream.h"

namespace Nimbus {

DiskStream::DiskStream(const juce::File& file, juce::AudioFormatManager& formatManager)
    : sourceFile(file)
{
    reader.reset(formatManager.createReaderFor(file));
    if (reader) {
        numChannels = reader->numChannels;
        sampleRate = reader->sampleRate;
        totalLength = reader->lengthInSamples;
        
        // 1-second read-ahead buffer
        ringBufferSize = static_cast<int>(sampleRate) * 2; 
        ringBuffer.setSize(numChannels, ringBufferSize);
        ringBuffer.clear();
    }
}

DiskStream::~DiskStream() {
}

void DiskStream::requestSeek(juce::int64 newPosition) {
    if (!reader) return;
    newPosition = juce::jlimit<juce::int64>(0, totalLength, newPosition);
    seekTarget.store(newPosition);
    seekRequested.store(true);
}

bool DiskStream::readSamples(juce::AudioBuffer<float>& destBuffer, int destStartSample, juce::int64 sourceStartSample, int numSamples) {
    if (!reader) return false;
    
    juce::int64 rPos = readPosition.load(std::memory_order_relaxed);
    juce::int64 wPos = writePosition.load(std::memory_order_relaxed);
    
    // If the transport asks for a sample that is far away from our current read head, seek.
    // 2048 is a reasonable threshold to prevent unnecessary seeks from tiny jitter
    if (std::abs(sourceStartSample - rPos) > 2048) {
        requestSeek(sourceStartSample);
        return false; // Silence for this block until seek finishes
    }
    
    // We adjust rPos to exactly what the caller wants to guarantee phase lock.
    rPos = sourceStartSample;
    
    int available = static_cast<int>(wPos - rPos);
    if (available < numSamples) {
        // Not enough data ready yet.
        return false;
    }
    
    int readIndex1 = static_cast<int>(rPos % ringBufferSize);
    int size1 = std::min(numSamples, ringBufferSize - readIndex1);
    int size2 = numSamples - size1;
    
    for (int ch = 0; ch < std::min(numChannels, destBuffer.getNumChannels()); ++ch) {
        destBuffer.copyFrom(ch, destStartSample, ringBuffer, ch, readIndex1, size1);
        if (size2 > 0) {
            destBuffer.copyFrom(ch, destStartSample + size1, ringBuffer, ch, 0, size2);
        }
    }
    
    readPosition.store(rPos + numSamples, std::memory_order_release);
    return true;
}

void DiskStream::processStream() {
    if (!reader) return;

    if (seekRequested.exchange(false)) {
        juce::int64 target = seekTarget.load();
        const juce::ScopedLock sl(readerLock);
        readPosition.store(target, std::memory_order_relaxed);
        writePosition.store(target, std::memory_order_relaxed);
    }
    
    juce::int64 rPos = readPosition.load(std::memory_order_acquire);
    juce::int64 wPos = writePosition.load(std::memory_order_relaxed);
    
    int availableSpace = ringBufferSize - static_cast<int>(wPos - rPos);
    int targetChunkSize = 8192; // Read in 8k chunks
    
    if (availableSpace >= targetChunkSize) {
        juce::int64 currentFilePos = wPos;
        int writeIndex1 = static_cast<int>(wPos % ringBufferSize);
        
        bool readOk = true;
        int actualSamplesRead = 0;
        
        {
            const juce::ScopedLock sl(readerLock);
            int maxToRead = static_cast<int>(std::min<juce::int64>(targetChunkSize, totalLength - currentFilePos));
            
            if (maxToRead > 0) {
                int size1 = std::min(maxToRead, ringBufferSize - writeIndex1);
                int size2 = maxToRead - size1;
                
                if (size1 > 0) {
                    readOk = reader->read(&ringBuffer, writeIndex1, size1, currentFilePos, true, true);
                    if (readOk) actualSamplesRead += size1;
                }
                if (readOk && size2 > 0) {
                    readOk = reader->read(&ringBuffer, 0, size2, currentFilePos + size1, true, true);
                    if (readOk) actualSamplesRead += size2;
                }
            }
        }
        
        if (actualSamplesRead > 0) {
            writePosition.store(wPos + actualSamplesRead, std::memory_order_release);
        }
    }
}

} // namespace Nimbus
