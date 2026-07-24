#include "MidiMonitorPlugin.h"
#include "../../../UI/DesignSystem/Colors.h"
#include "../../../UI/DesignSystem/Typography.h"

namespace Nimbus {

class MidiMonitorPluginEditor : public juce::Component {
public:
    MidiMonitorPluginEditor(MidiMonitorPlugin& p) : plugin(p) {
        setSize(250, 150);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::PanelBackground);
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
        g.drawText("MIDI Monitor Editor (Coming Soon)", getLocalBounds(), juce::Justification::centred, false);
    }

private:
    MidiMonitorPlugin& plugin;
};

MidiMonitorPlugin::MidiMonitorPlugin() {
}

juce::Component* MidiMonitorPlugin::createEditor() {
    return new MidiMonitorPluginEditor(*this);
}

void MidiMonitorPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
}

void MidiMonitorPlugin::releaseResources() {
}

void MidiMonitorPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (bypassed) return;
    
    // In the future, copy messages to a thread-safe queue for the UI to display.
    // For now, it just passes through transparently.
}

void MidiMonitorPlugin::getStateInformation(juce::MemoryBlock& destData) {
}

void MidiMonitorPlugin::setStateInformation(const void* data, int sizeInBytes) {
}

} // namespace Nimbus
