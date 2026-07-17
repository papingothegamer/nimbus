#include "GainPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class GainPluginEditor : public juce::Component, private juce::Timer {
public:
    GainPluginEditor(GainPlugin& p) : plugin(p) {
        gainSlider = std::make_unique<NimbusRotaryDial>("Gain", -60.0, 24.0, plugin.getGainDecibels(), " dB", [this](float v) { plugin.setGainDecibels(v); });
        addAndMakeVisible(gainSlider.get());
        
        phaseButton.setButtonText(juce::CharPointer_UTF8("\xc3\x98")); // Phi symbol
        phaseButton.setToggleState(plugin.getPhaseInvert(), juce::dontSendNotification);
        phaseButton.onClick = [this]() {
            plugin.setPhaseInvert(phaseButton.getToggleState());
        };
        addAndMakeVisible(phaseButton);
        
        header = std::make_unique<PluginHeader>("Gain Utility");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(140, 180); // Embedded size
    }
    
    ~GainPluginEditor() override {
        stopTimer();
    }
    
    void timerCallback() override {
        gainSlider->setValue(plugin.getGainDecibels());
        if (plugin.getPhaseInvert() != phaseButton.getToggleState()) {
            phaseButton.setToggleState(plugin.getPhaseInvert(), juce::dontSendNotification);
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        auto bounds = getLocalBounds().reduced(10);
        
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(bounds.withTrimmedTop(25).toFloat(), 4.0f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        header->setBounds(bounds.removeFromTop(25));
        
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;
        fb.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        fb.alignItems = juce::FlexBox::AlignItems::center;
        
        fb.items.add(juce::FlexItem(*gainSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(phaseButton).withWidth(30).withHeight(20));
        
        fb.performLayout(bounds);
    }

private:
    GainPlugin& plugin;
    std::unique_ptr<NimbusRotaryDial> gainSlider;
    juce::ToggleButton phaseButton;
    std::unique_ptr<PluginHeader> header;
};


GainPlugin::GainPlugin() {
    updateDSP();
}

juce::Component* GainPlugin::createEditor() {
    return new GainPluginEditor(*this);
}

void GainPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2; // Assuming stereo for now

    {const juce::SpinLock::ScopedLockType sl(processLock);
    dspGain.prepare(spec);
    

    }


    updateDSP();
}

void GainPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspGain.reset();
}

void GainPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;

    const juce::SpinLock::ScopedLockType sl(processLock);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspGain.process(context);
}

void GainPlugin::updateDSP() {
    float linearGain = juce::Decibels::decibelsToGain(gainDb.load());
    if (phaseInvert.load()) {
        linearGain = -linearGain;
    }
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspGain.setGainLinear(linearGain);
}

} // namespace Nimbus
