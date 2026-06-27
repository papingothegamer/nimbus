#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <memory>

namespace Nimbus {

class AudioRecorder : private juce::TimeSliceClient {
public:
    AudioRecorder(juce::TimeSliceThread& backgroundThread);
    ~AudioRecorder() override;

    // Call from UI thread before recording starts
    bool startRecording(const juce::File& outputFile, double sampleRate, int numChannels);
    
    // Call from real-time audio thread — lock-free, allocation-free
    void pushSamples(const juce::AudioBuffer<float>& buffer, int numSamples);
    
    // Call from UI thread when recording stops
    juce::File stopRecording();
    
    bool isRecording() const { return activeWriter.load() != nullptr; }
    int64_t getNumSamplesRecorded() const { return numSamplesRecorded.load(); }

private:
    int useTimeSlice() override; // Background disk write
    
    juce::TimeSliceThread& thread;
    std::atomic<juce::AudioFormatWriter*> activeWriter { nullptr };
    juce::CriticalSection writerLock; // Only used on non-RT thread for cleanup
    
    // Lock-free FIFO for RT -> background transfer
    juce::AbstractFifo fifo { 48000 * 2 }; // ~2 seconds buffer at 48kHz
    juce::AudioBuffer<float> fifoBuffer;
    
    std::atomic<int64_t> numSamplesRecorded { 0 };
    std::unique_ptr<juce::AudioFormatWriter> ownedWriter;
    juce::File currentFile;
};

} // namespace Nimbus
