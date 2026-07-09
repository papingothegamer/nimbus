#include "ReverbPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class ReverbPluginEditor : public juce::Component, private juce::Timer {
public:
    ReverbPluginEditor(ReverbPlugin& p) : plugin(p) {
        sizeSlider = std::make_unique<PluginDial>("Size", 0.0, 1.0, plugin.getRoomSize(), [this](float v) { plugin.setRoomSize(v); });
        dampSlider = std::make_unique<PluginDial>("Damp", 0.0, 1.0, plugin.getDamping(), [this](float v) { plugin.setDamping(v); });
        widthSlider = std::make_unique<PluginDial>("Width", 0.0, 1.0, plugin.getWidth(), [this](float v) { plugin.setWidth(v); });
        mixSlider = std::make_unique<PluginDial>("Mix", 0.0, 1.0, plugin.getMix(), [this](float v) { plugin.setMix(v); });
        
        addAndMakeVisible(sizeSlider.get());
        addAndMakeVisible(dampSlider.get());
        addAndMakeVisible(widthSlider.get());
        addAndMakeVisible(mixSlider.get());
        
        header = std::make_unique<PluginHeader>("Reverb");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(320, 150);
    }
    
    ~ReverbPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        sizeSlider->setValue(plugin.getRoomSize());
        dampSlider->setValue(plugin.getDamping());
        widthSlider->setValue(plugin.getWidth());
        mixSlider->setValue(plugin.getMix());
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
        fb.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        fb.alignItems = juce::FlexBox::AlignItems::center;
        
        fb.items.add(juce::FlexItem(*sizeSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*dampSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*widthSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*mixSlider).withWidth(65).withHeight(90));
        
        fb.performLayout(bounds);
    }

private:
    ReverbPlugin& plugin;
    std::unique_ptr<PluginDial> sizeSlider;
    std::unique_ptr<PluginDial> dampSlider;
    std::unique_ptr<PluginDial> widthSlider;
    std::unique_ptr<PluginDial> mixSlider;
    std::unique_ptr<PluginHeader> header;
};

ReverbPlugin::ReverbPlugin() { updateDSP(); }

juce::Component* ReverbPlugin::createEditor() { return new ReverbPluginEditor(*this); }

void ReverbPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    {const juce::SpinLock::ScopedLockType sl(processLock);
    dspReverb.prepare(spec);
    

    }


    updateDSP();
}

void ReverbPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspReverb.reset();
}

void ReverbPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspReverb.process(context);
}

void ReverbPlugin::updateDSP() {
    juce::dsp::Reverb::Parameters params;
    params.roomSize = roomSize.load();
    params.damping = damping.load();
    params.wetLevel = mix.load();
    params.dryLevel = 1.0f - mix.load();
    params.width = width.load();
    params.freezeMode = 0.0f;
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspReverb.setParameters(params);
}

} // namespace Nimbus
