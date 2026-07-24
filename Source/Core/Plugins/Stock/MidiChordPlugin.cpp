#include "MidiChordPlugin.h"
#include "../../../UI/DesignSystem/Colors.h"
#include "../../../UI/DesignSystem/Typography.h"

namespace Nimbus {

class MidiChordPluginEditor : public juce::Component {
public:
    MidiChordPluginEditor(MidiChordPlugin& p) : plugin(p) {
        setSize(250, 150);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::PanelBackground);
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
        g.drawText("Chord Effect (Major Triad)", getLocalBounds(), juce::Justification::centred, false);
    }

private:
    MidiChordPlugin& plugin;
};

MidiChordPlugin::MidiChordPlugin() {
}

juce::Component* MidiChordPlugin::createEditor() {
    return new MidiChordPluginEditor(*this);
}

void MidiChordPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
}

void MidiChordPlugin::releaseResources() {
}

void MidiChordPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (bypassed || chordIntervals.empty()) return;
    
    juce::MidiBuffer processedBuffer;
    for (const auto meta : midiMessages) {
        auto message = meta.getMessage();
        
        // Always pass through the original message
        processedBuffer.addEvent(message, meta.samplePosition);
        
        // Add chord notes
        if (message.isNoteOn() || message.isNoteOff()) {
            for (int interval : chordIntervals) {
                if (interval == 0) continue;
                int newNote = juce::jlimit(0, 127, message.getNoteNumber() + interval);
                auto newMessage = message;
                newMessage.setNoteNumber(newNote);
                processedBuffer.addEvent(newMessage, meta.samplePosition);
            }
        }
    }
    midiMessages.swapWith(processedBuffer);
}

void MidiChordPlugin::getStateInformation(juce::MemoryBlock& destData) {
}

void MidiChordPlugin::setStateInformation(const void* data, int sizeInBytes) {
}

} // namespace Nimbus
