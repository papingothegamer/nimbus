#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include "../DataModel/MidiClip.h"
#include <memory>
#include <mutex>

namespace Nimbus {

class MidiRecorder {
public:
    MidiRecorder();
    ~MidiRecorder() = default;

    void startRecording(double startSamplePosition);
    
    // Called by the audio thread
    void pushEvents(const juce::MidiBuffer& buffer, int numSamples, double blockStartSamplePos);
    
    // Called when recording finishes, returns the generated clip
    std::shared_ptr<MidiClip> stopRecordingAndGetClip(double endSamplePosition);

private:
    std::mutex mutex;
    juce::MidiMessageSequence sequence;
    double clipStartSample = 0.0;
    bool isRecording = false;
};

} // namespace Nimbus
