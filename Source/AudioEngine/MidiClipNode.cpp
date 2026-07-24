#include "MidiClipNode.h"

namespace Nimbus {

MidiClipNode::MidiClipNode(std::shared_ptr<MidiClip> clip, ITransport& t)
    : midiClip(std::move(clip)), transport(t) {
}

void MidiClipNode::prepareToPlay(double sampleRate, int /*maximumExpectedSamplesPerBlock*/) {
    sampleRate_ = sampleRate;
}

void MidiClipNode::releaseResources() {
    juce::MidiBuffer dummy;
    turnOffActiveNotes(dummy, 0);
}

void MidiClipNode::turnOffActiveNotes(juce::MidiBuffer& midiMessages, int sampleOffset) {
    for (int note : activeNotes) {
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, note, (juce::uint8)0), sampleOffset);
    }
    activeNotes.clear();
}

void MidiClipNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    int numSamples = buffer.getNumSamples();

    if (!transport.isPlaying() || !midiClip) {
        if (!activeNotes.isEmpty()) {
            turnOffActiveNotes(midiMessages, 0);
        }
        lastTransportPos = -1.0;
        return;
    }

    double currentPos = transport.getCurrentPosition();
    
    // Check if transport jumped/seeked
    if (lastTransportPos >= 0.0 && std::abs(currentPos - lastTransportPos) > numSamples + 10) {
        turnOffActiveNotes(midiMessages, 0);
    }
    
    double nextPos = currentPos + numSamples;
    lastTransportPos = nextPos;

    double clipStart = midiClip->startSample.get();
    double clipEnd = clipStart + midiClip->lengthSamples.get();

    // If we are completely outside the clip
    if (nextPos <= clipStart || currentPos >= clipEnd) {
        if (!activeNotes.isEmpty()) {
            turnOffActiveNotes(midiMessages, 0);
        }
        return;
    }

    auto& sequence = midiClip->getSequence();
    
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        if (event == nullptr)
            continue;

        auto& msg = event->message;
        double absoluteTime = clipStart + msg.getTimeStamp();

        // Standard event inside block
        if (absoluteTime >= currentPos && absoluteTime < nextPos && absoluteTime < clipEnd) {
            int sampleOffset = juce::roundToInt(absoluteTime - currentPos);
            sampleOffset = juce::jlimit(0, numSamples - 1, sampleOffset);
            
            midiMessages.addEvent(msg, sampleOffset);
            
            if (msg.isNoteOn()) {
                activeNotes.addIfNotAlreadyThere(msg.getNoteNumber());
            } else if (msg.isNoteOff()) {
                activeNotes.removeAllInstancesOf(msg.getNoteNumber());
            }
        }
    }
    
    // Handle clip end boundary for active notes
    if (nextPos >= clipEnd && currentPos < clipEnd) {
        int offset = juce::roundToInt(clipEnd - currentPos);
        offset = juce::jlimit(0, numSamples - 1, offset);
        turnOffActiveNotes(midiMessages, offset);
    }
}

} // namespace Nimbus
