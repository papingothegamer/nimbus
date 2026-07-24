#include "MidiArpeggiatorPlugin.h"
#include "../../../UI/DesignSystem/Colors.h"
#include "../../../UI/DesignSystem/Typography.h"

namespace Nimbus {

class MidiArpeggiatorPluginEditor : public juce::Component {
public:
    MidiArpeggiatorPluginEditor(MidiArpeggiatorPlugin& p) : plugin(p) {
        setSize(250, 150);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::PanelBackground);
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
        g.drawText("Arpeggiator Editor (Coming Soon)", getLocalBounds(), juce::Justification::centred, false);
    }

private:
    MidiArpeggiatorPlugin& plugin;
};

MidiArpeggiatorPlugin::MidiArpeggiatorPlugin() {
}

juce::Component* MidiArpeggiatorPlugin::createEditor() {
    return new MidiArpeggiatorPluginEditor(*this);
}

void MidiArpeggiatorPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
}

void MidiArpeggiatorPlugin::releaseResources() {
}

void MidiArpeggiatorPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (bypassed) return;
    
    int numSamples = buffer.getNumSamples();
    juce::MidiBuffer processedBuffer;

    for (const auto meta : midiMessages) {
        auto message = meta.getMessage();
        if (message.isNoteOn()) {
            if (std::find(heldNotes.begin(), heldNotes.end(), message.getNoteNumber()) == heldNotes.end()) {
                heldNotes.push_back(message.getNoteNumber());
                std::sort(heldNotes.begin(), heldNotes.end());
            }
        } else if (message.isNoteOff()) {
            heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), message.getNoteNumber()), heldNotes.end());
            if (heldNotes.empty()) {
                if (lastPlayedNote != -1) {
                    processedBuffer.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), meta.samplePosition);
                    lastPlayedNote = -1;
                }
            }
        }
    }
    
    if (heldNotes.empty()) {
        midiMessages.swapWith(processedBuffer);
        return;
    }

    double samplesPerNote = currentSampleRate * rateInSeconds;
    
    for (int i = 0; i < numSamples; ++i) {
        samplesSinceLastNote++;
        if (samplesSinceLastNote >= samplesPerNote) {
            samplesSinceLastNote = 0.0;
            
            // Turn off last note
            if (lastPlayedNote != -1) {
                processedBuffer.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), i);
            }
            
            // Play next note
            if (currentNoteIndex >= heldNotes.size()) {
                currentNoteIndex = 0;
            }
            
            lastPlayedNote = heldNotes[currentNoteIndex];
            processedBuffer.addEvent(juce::MidiMessage::noteOn(1, lastPlayedNote, (juce::uint8)100), i);
            
            currentNoteIndex++;
        }
    }
    
    midiMessages.swapWith(processedBuffer);
}

void MidiArpeggiatorPlugin::getStateInformation(juce::MemoryBlock& destData) {
}

void MidiArpeggiatorPlugin::setStateInformation(const void* data, int sizeInBytes) {
}

} // namespace Nimbus
