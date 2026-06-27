#include "AudioRecorder.h"

namespace Nimbus {

AudioRecorder::AudioRecorder(juce::TimeSliceThread& backgroundThread)
    : thread(backgroundThread)
{
    fifoBuffer.setSize(2, fifo.getTotalSize());
    fifoBuffer.clear();
}

AudioRecorder::~AudioRecorder() {
    stopRecording();
}

bool AudioRecorder::startRecording(const juce::File& outputFile, double sampleRate, int numChannels) {
    stopRecording();

    currentFile = outputFile;
    currentFile.deleteFile();

    if (auto outStream = std::unique_ptr<juce::FileOutputStream>(currentFile.createOutputStream())) {
        juce::WavAudioFormat wavFormat;
        if (auto writer = wavFormat.createWriterFor(outStream.get(), sampleRate, numChannels, 24, {}, 0)) {
            outStream.release(); // The writer takes ownership

            // Setup buffers
            fifo.reset();
            fifoBuffer.setSize(numChannels, fifo.getTotalSize());
            fifoBuffer.clear();
            numSamplesRecorded.store(0);

            ownedWriter.reset(writer);
            activeWriter.store(ownedWriter.get());
            
            thread.addTimeSliceClient(this);
            return true;
        }
    }
    return false;
}

void AudioRecorder::pushSamples(const juce::AudioBuffer<float>& buffer, int numSamples) {
    if (activeWriter.load() == nullptr)
        return;

    jassert(buffer.getNumChannels() == fifoBuffer.getNumChannels());

    int start1, size1, start2, size2;
    fifo.prepareToWrite(numSamples, start1, size1, start2, size2);

    if (size1 > 0) {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            fifoBuffer.copyFrom(ch, start1, buffer, ch, 0, size1);
        }
    }
    if (size2 > 0) {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            fifoBuffer.copyFrom(ch, start2, buffer, ch, size1, size2);
        }
    }

    fifo.finishedWrite(size1 + size2);
    numSamplesRecorded.fetch_add(size1 + size2);
}

juce::File AudioRecorder::stopRecording() {
    if (activeWriter.load() != nullptr) {
        activeWriter.store(nullptr);
        
        // Wait for the background thread to finish writing whatever is left
        thread.removeTimeSliceClient(this);
        
        // One final flush on the main thread (with lock to ensure safety)
        const juce::ScopedLock sl(writerLock);
        
        int start1, size1, start2, size2;
        fifo.prepareToRead(fifo.getNumReady(), start1, size1, start2, size2);
        
        if (ownedWriter != nullptr) {
            if (size1 > 0) {
                ownedWriter->writeFromAudioSampleBuffer(fifoBuffer, start1, size1);
            }
            if (size2 > 0) {
                ownedWriter->writeFromAudioSampleBuffer(fifoBuffer, start2, size2);
            }
        }
        fifo.finishedRead(size1 + size2);
        
        ownedWriter.reset();
    }
    
    return currentFile;
}

int AudioRecorder::useTimeSlice() {
    const juce::ScopedLock sl(writerLock);
    
    auto writer = activeWriter.load();
    if (writer == nullptr)
        return 50; // Sleep and check again

    int start1, size1, start2, size2;
    fifo.prepareToRead(fifo.getNumReady(), start1, size1, start2, size2);

    if (size1 > 0) {
        writer->writeFromAudioSampleBuffer(fifoBuffer, start1, size1);
    }
    if (size2 > 0) {
        writer->writeFromAudioSampleBuffer(fifoBuffer, start2, size2);
    }

    fifo.finishedRead(size1 + size2);

    // If there's more data, return 0 to run again immediately, else sleep for a bit
    return fifo.getNumReady() > 0 ? 0 : 50;
}

} // namespace Nimbus
