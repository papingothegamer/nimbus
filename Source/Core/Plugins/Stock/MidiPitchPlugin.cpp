#include "MidiPitchPlugin.h"
#include "../../../UI/DesignSystem/Colors.h"
#include "../../../UI/DesignSystem/Typography.h"

namespace Nimbus {

class MidiPitchPluginEditor : public juce::Component {
public:
    MidiPitchPluginEditor(MidiPitchPlugin& p) : plugin(p) {
        setSize(250, 150);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::PanelBackground);
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
        g.drawText("Pitch Editor (Coming Soon)", getLocalBounds(), juce::Justification::centred, false);
    }

private:
    MidiPitchPlugin& plugin;
};

MidiPitchPlugin::MidiPitchPlugin() {
}

juce::Component* MidiPitchPlugin::createEditor() {
    return new MidiPitchPluginEditor(*this);
}

void MidiPitchPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
}

void MidiPitchPlugin::releaseResources() {
}

void MidiPitchPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (bypassed || pitchShift == 0) return;
    
    juce::MidiBuffer processedBuffer;
    for (const auto meta : midiMessages) {
        auto message = meta.getMessage();
        if (message.isNoteOn() || message.isNoteOff()) {
            int newNote = juce::jlimit(0, 127, message.getNoteNumber() + pitchShift);
            message.setNoteNumber(newNote);
        }
        processedBuffer.addEvent(message, meta.samplePosition);
    }
    midiMessages.swapWith(processedBuffer);
}

void MidiPitchPlugin::getStateInformation(juce::MemoryBlock& destData) {
}

void MidiPitchPlugin::setStateInformation(const void* data, int sizeInBytes) {
}

} // namespace Nimbus
