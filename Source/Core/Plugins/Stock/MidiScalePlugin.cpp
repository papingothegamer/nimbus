#include "MidiScalePlugin.h"
#include "../../../UI/DesignSystem/Colors.h"
#include "../../../UI/DesignSystem/Typography.h"

namespace Nimbus {

class MidiScalePluginEditor : public juce::Component {
public:
    MidiScalePluginEditor(MidiScalePlugin& p) : plugin(p) {
        setSize(250, 150);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::PanelBackground);
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
        g.drawText("Scale Editor (Coming Soon)", getLocalBounds(), juce::Justification::centred, false);
    }

private:
    MidiScalePlugin& plugin;
};

MidiScalePlugin::MidiScalePlugin() {
}

juce::Component* MidiScalePlugin::createEditor() {
    return new MidiScalePluginEditor(*this);
}

void MidiScalePlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
}

void MidiScalePlugin::releaseResources() {
}

void MidiScalePlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (bypassed || scaleMask.size() != 12) return;
    
    juce::MidiBuffer processedBuffer;
    for (const auto meta : midiMessages) {
        auto message = meta.getMessage();
        if (message.isNoteOn() || message.isNoteOff()) {
            int note = message.getNoteNumber();
            int pitchClass = note % 12;
            
            if (!scaleMask[pitchClass]) {
                // Find nearest note in scale
                int nearestNote = note;
                for (int i = 1; i <= 6; ++i) {
                    int down = (pitchClass - i + 12) % 12;
                    int up = (pitchClass + i) % 12;
                    if (scaleMask[down]) {
                        nearestNote = note - i;
                        break;
                    }
                    if (scaleMask[up]) {
                        nearestNote = note + i;
                        break;
                    }
                }
                message.setNoteNumber(juce::jlimit(0, 127, nearestNote));
            }
        }
        processedBuffer.addEvent(message, meta.samplePosition);
    }
    midiMessages.swapWith(processedBuffer);
}

void MidiScalePlugin::getStateInformation(juce::MemoryBlock& destData) {
}

void MidiScalePlugin::setStateInformation(const void* data, int sizeInBytes) {
}

} // namespace Nimbus
