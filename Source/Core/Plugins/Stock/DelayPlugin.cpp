#include "DelayPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class DelayPluginEditor : public juce::Component, private juce::Timer {
public:
    DelayPluginEditor(DelayPlugin& p) : plugin(p) {
        timeSlider = std::make_unique<PluginDial>("Time", 1.0, 2000.0, plugin.getDelayTimeMs(), " ms", [this](float v) { plugin.setDelayTimeMs(v); });
        fbSlider = std::make_unique<PluginDial>("Feedback", 0.0, 0.95, plugin.getFeedback(), "", [this](float v) { plugin.setFeedback(v); });
        mixSlider = std::make_unique<PluginDial>("Mix", 0.0, 1.0, plugin.getMix(), "", [this](float v) { plugin.setMix(v); });
        
        addAndMakeVisible(timeSlider.get());
        addAndMakeVisible(fbSlider.get());
        addAndMakeVisible(mixSlider.get());
        
        header = std::make_unique<PluginHeader>("Delay");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(280, 150);
    }
    
    ~DelayPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        timeSlider->setValue(plugin.getDelayTimeMs());
        fbSlider->setValue(plugin.getFeedback());
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
        
        fb.items.add(juce::FlexItem(*timeSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(*fbSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(*mixSlider).withWidth(70).withHeight(90));
        
        fb.performLayout(bounds);
    }

private:
    DelayPlugin& plugin;
    std::unique_ptr<PluginDial> timeSlider;
    std::unique_ptr<PluginDial> fbSlider;
    std::unique_ptr<PluginDial> mixSlider;
    std::unique_ptr<PluginHeader> header;
};

DelayPlugin::DelayPlugin() { updateDSP(); }

juce::Component* DelayPlugin::createEditor() { return new DelayPluginEditor(*this); }

void DelayPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;
    
    currentSampleRate = sampleRate;

    {const juce::SpinLock::ScopedLockType sl(processLock);
    dspDelay.prepare(spec);
    

    }


    updateDSP();
}

void DelayPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspDelay.reset();
}

void DelayPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    float fb = feedback.load();
    float mix = wetMix.load();
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            float in = channelData[i];
            float delayed = dspDelay.popSample(channel);
            dspDelay.pushSample(channel, in + delayed * fb);
            channelData[i] = in * (1.0f - mix) + delayed * mix;
        }
    }
}

void DelayPlugin::updateDSP() {
    if (currentSampleRate <= 0.0) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    float samples = (delayTimeMs.load() / 1000.0f) * currentSampleRate;
    dspDelay.setDelay(samples);
}

} // namespace Nimbus
