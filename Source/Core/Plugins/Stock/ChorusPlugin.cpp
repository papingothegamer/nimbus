#include "ChorusPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class ChorusPluginEditor : public juce::Component, private juce::Timer {
public:
    ChorusPluginEditor(ChorusPlugin& p) : plugin(p) {
        rateSlider = std::make_unique<NimbusRotaryDial>("Rate", 0.1, 10.0, plugin.getRate(), " Hz", [this](float v) { plugin.setRate(v); });
        depthSlider = std::make_unique<NimbusRotaryDial>("Depth", 0.0, 1.0, plugin.getDepth(), "", [this](float v) { plugin.setDepth(v); });
        mixSlider = std::make_unique<NimbusRotaryDial>("Mix", 0.0, 1.0, plugin.getMix(), "", [this](float v) { plugin.setMix(v); });
        
        addAndMakeVisible(rateSlider.get());
        addAndMakeVisible(depthSlider.get());
        addAndMakeVisible(mixSlider.get());
        
        header = std::make_unique<PluginHeader>("Chorus");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(280, 150);
    }
    
    ~ChorusPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        rateSlider->setValue(plugin.getRate());
        depthSlider->setValue(plugin.getDepth());
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
        
        fb.items.add(juce::FlexItem(*rateSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(*depthSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(*mixSlider).withWidth(70).withHeight(90));
        
        fb.performLayout(bounds);
    }

private:
    ChorusPlugin& plugin;
    std::unique_ptr<NimbusRotaryDial> rateSlider;
    std::unique_ptr<NimbusRotaryDial> depthSlider;
    std::unique_ptr<NimbusRotaryDial> mixSlider;
    std::unique_ptr<PluginHeader> header;
};

ChorusPlugin::ChorusPlugin() { updateDSP(); }

juce::Component* ChorusPlugin::createEditor() { return new ChorusPluginEditor(*this); }

void ChorusPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    {const juce::SpinLock::ScopedLockType sl(processLock);
    dspChorus.prepare(spec);
    

    }


    updateDSP();
}

void ChorusPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspChorus.reset();
}

void ChorusPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspChorus.process(context);
}

void ChorusPlugin::updateDSP() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspChorus.setRate(rate.load());
    dspChorus.setDepth(depth.load());
    dspChorus.setMix(mix.load());
}

} // namespace Nimbus
