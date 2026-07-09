#include "CompressorPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class CompressorPluginEditor : public juce::Component, private juce::Timer {
public:
    CompressorPluginEditor(CompressorPlugin& p) : plugin(p) {
        threshSlider = std::make_unique<PluginDial>("Thresh", -60.0, 0.0, plugin.getThreshold(), [this](float v) { plugin.setThreshold(v); }, " dB");
        ratioSlider = std::make_unique<PluginDial>("Ratio", 1.0, 20.0, plugin.getRatio(), [this](float v) { plugin.setRatio(v); }, ":1");
        attackSlider = std::make_unique<PluginDial>("Attack", 0.1, 500.0, plugin.getAttack(), [this](float v) { plugin.setAttack(v); }, " ms");
        releaseSlider = std::make_unique<PluginDial>("Release", 10.0, 2000.0, plugin.getRelease(), [this](float v) { plugin.setRelease(v); }, " ms");
        
        addAndMakeVisible(threshSlider.get());
        addAndMakeVisible(ratioSlider.get());
        addAndMakeVisible(attackSlider.get());
        addAndMakeVisible(releaseSlider.get());
        
        header = std::make_unique<PluginHeader>("Compressor");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(320, 150);
    }
    
    ~CompressorPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        threshSlider->setValue(plugin.getThreshold());
        ratioSlider->setValue(plugin.getRatio());
        attackSlider->setValue(plugin.getAttack());
        releaseSlider->setValue(plugin.getRelease());
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
        
        fb.items.add(juce::FlexItem(*threshSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*ratioSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*attackSlider).withWidth(65).withHeight(90));
        fb.items.add(juce::FlexItem(*releaseSlider).withWidth(65).withHeight(90));
        
        fb.performLayout(bounds);
    }

private:
    CompressorPlugin& plugin;
    std::unique_ptr<PluginDial> threshSlider;
    std::unique_ptr<PluginDial> ratioSlider;
    std::unique_ptr<PluginDial> attackSlider;
    std::unique_ptr<PluginDial> releaseSlider;
    std::unique_ptr<PluginHeader> header;
};

CompressorPlugin::CompressorPlugin() { updateDSP(); }

juce::Component* CompressorPlugin::createEditor() { return new CompressorPluginEditor(*this); }

void CompressorPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    {const juce::SpinLock::ScopedLockType sl(processLock);
    dspComp.prepare(spec);
    

    }


    updateDSP();
}

void CompressorPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspComp.reset();
}

void CompressorPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspComp.process(context);
}

void CompressorPlugin::updateDSP() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspComp.setThreshold(threshold.load());
    dspComp.setRatio(ratio.load());
    dspComp.setAttack(attack.load());
    dspComp.setRelease(release.load());
}

} // namespace Nimbus
