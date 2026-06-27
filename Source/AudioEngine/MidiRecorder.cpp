#include "MidiRecorder.h"

namespace Nimbus {

MidiRecorder::MidiRecorder() {
}

void MidiRecorder::startRecording(double startSamplePosition) {
    std::lock_guard<std::mutex> lock(mutex);
    sequence.clear();
    clipStartSample = startSamplePosition;
    isRecording = true;
}

void MidiRecorder::pushEvents(const juce::MidiBuffer& buffer, int /*numSamples*/, double blockStartSamplePos) {
    if (!isRecording) return;
    
    // We use a spinlock/mutex here. In a true hard real-time system, we'd use a lock-free queue.
    // For now, this suffices since the UI thread only locks when stopping recording.
    if (mutex.try_lock()) {
        for (const auto metadata : buffer) {
            auto msg = metadata.getMessage();
            double absoluteSamplePos = blockStartSamplePos + metadata.samplePosition;
            double relativeToClipStart = absoluteSamplePos - clipStartSample;
            
            if (relativeToClipStart >= 0) {
                msg.setTimeStamp(relativeToClipStart);
                sequence.addEvent(msg);
            }
        }
        mutex.unlock();
    }
}

std::shared_ptr<MidiClip> MidiRecorder::stopRecordingAndGetClip(double endSamplePosition) {
    std::lock_guard<std::mutex> lock(mutex);
    isRecording = false;
    
    double lengthSamples = endSamplePosition - clipStartSample;
    if (lengthSamples <= 0) return nullptr;
    
    auto clip = std::make_shared<MidiClip>(clipStartSample, lengthSamples);
    sequence.updateMatchedPairs();
    clip->getSequence() = sequence;
    
    return clip;
}

} // namespace Nimbus
