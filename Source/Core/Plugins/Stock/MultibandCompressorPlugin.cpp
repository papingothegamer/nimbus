#include "MultibandCompressorPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"
#include <array>

namespace Nimbus {

class MultibandCompressorPluginEditor : public juce::Component, private juce::Timer {
public:
    MultibandCompressorPluginEditor(MultibandCompressorPlugin& p) : plugin(p) {
        lowMidSlider = std::make_unique<PluginDial>("Low/Mid", 20.0, 1000.0, plugin.getLowMidFreq(), [this](float v) { plugin.setLowMidFreq(v); }, " Hz");
        midHighSlider = std::make_unique<PluginDial>("Mid/High", 500.0, 15000.0, plugin.getMidHighFreq(), [this](float v) { plugin.setMidHighFreq(v); }, " Hz");
        lowMidSlider->getSlider().setSkewFactorFromMidPoint(200.0);
        midHighSlider->getSlider().setSkewFactorFromMidPoint(2000.0);
        
        addAndMakeVisible(lowMidSlider.get());
        addAndMakeVisible(midHighSlider.get());
        
        for (int i = 0; i < 3; ++i) {
            auto& band = plugin.getBand(i);
            threshSliders[i] = std::make_unique<PluginDial>("Thresh", -60.0, 0.0, band.threshold.load(), [this, i](float v) { plugin.getBand(i).threshold.store(v); plugin.updateDSP(); }, " dB");
            ratioSliders[i] = std::make_unique<PluginDial>("Ratio", 1.0, 20.0, band.ratio.load(), [this, i](float v) { plugin.getBand(i).ratio.store(v); plugin.updateDSP(); }, ":1");
            gainSliders[i] = std::make_unique<PluginDial>("Gain", -24.0, 24.0, band.gainDb.load(), [this, i](float v) { plugin.getBand(i).gainDb.store(v); plugin.updateDSP(); }, " dB");
            
            addAndMakeVisible(threshSliders[i].get());
            addAndMakeVisible(ratioSliders[i].get());
            addAndMakeVisible(gainSliders[i].get());
        }
        
        header = std::make_unique<PluginHeader>("Multiband Compressor");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(420, 300);
    }
    
    ~MultibandCompressorPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        lowMidSlider->setValue(plugin.getLowMidFreq());
        midHighSlider->setValue(plugin.getMidHighFreq());
        for (int i = 0; i < 3; ++i) {
            auto& band = plugin.getBand(i);
            threshSliders[i]->setValue(band.threshold.load());
            ratioSliders[i]->setValue(band.ratio.load());
            gainSliders[i]->setValue(band.gainDb.load());
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        auto bounds = getLocalBounds().reduced(10);
        
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(bounds.withTrimmedTop(25).toFloat(), 4.0f);
        
        const char* titles[] = {"Low Band", "Mid Band", "High Band"};
        int w = bounds.getWidth() / 3;
        for (int i = 0; i < 3; ++i) {
            juce::Rectangle<int> col(bounds.getX() + i * w, bounds.getY() + 75, w, bounds.getHeight() - 75);
            g.setColour(DesignSystem::Colors::TextPrimary.withAlpha(0.6f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f).boldened());
            g.drawText(titles[i], col.removeFromTop(20), juce::Justification::centred, false);
        }
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        header->setBounds(bounds.removeFromTop(25));
        
        auto topRow = bounds.removeFromTop(50);
        lowMidSlider->setBounds(topRow.removeFromLeft(getWidth() / 2).withSizeKeepingCentre(80, 80));
        midHighSlider->setBounds(topRow.withSizeKeepingCentre(80, 80));
        
        int w = bounds.getWidth() / 3;
        bounds.removeFromTop(20); // space for titles
        for (int i = 0; i < 3; ++i) {
            auto col = bounds.removeFromLeft(w).reduced(2);
            juce::FlexBox fb;
            fb.flexDirection = juce::FlexBox::Direction::column;
            fb.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            fb.alignItems = juce::FlexBox::AlignItems::center;
            
            fb.items.add(juce::FlexItem(*threshSliders[i]).withWidth(60).withHeight(60));
            fb.items.add(juce::FlexItem(*ratioSliders[i]).withWidth(60).withHeight(60));
            fb.items.add(juce::FlexItem(*gainSliders[i]).withWidth(60).withHeight(60));
            
            fb.performLayout(col);
        }
    }

private:
    MultibandCompressorPlugin& plugin;
    std::unique_ptr<PluginDial> lowMidSlider, midHighSlider;
    std::array<std::unique_ptr<PluginDial>, 3> threshSliders;
    std::array<std::unique_ptr<PluginDial>, 3> ratioSliders;
    std::array<std::unique_ptr<PluginDial>, 3> gainSliders;
    std::unique_ptr<PluginHeader> header;
};

MultibandCompressorPlugin::MultibandCompressorPlugin() { updateDSP(); }

juce::Component* MultibandCompressorPlugin::createEditor() { return new MultibandCompressorPluginEditor(*this); }

void MultibandCompressorPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    currentSampleRate = sampleRate;

    {
        const juce::SpinLock::ScopedLockType sl(processLock);
        lp1.prepare(spec); hp1.prepare(spec);
        lp2.prepare(spec); hp2.prepare(spec);
        lp1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        hp1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        lp2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        hp2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        
        for (auto& comp : dspComps) comp.prepare(spec);
    }
    updateDSP();
}

void MultibandCompressorPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    lp1.reset(); hp1.reset(); lp2.reset(); hp2.reset();
    for (auto& comp : dspComps) comp.reset();
}

void MultibandCompressorPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    juce::AudioBuffer<float> lowBuffer, midBuffer, highBuffer;
    lowBuffer.makeCopyOf(buffer, true);
    midBuffer.makeCopyOf(buffer, true);
    highBuffer.makeCopyOf(buffer, true);
    
    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    juce::dsp::AudioBlock<float> midBlock(midBuffer);
    juce::dsp::AudioBlock<float> highBlock(highBuffer);
    
    juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
    juce::dsp::ProcessContextReplacing<float> midContext(midBlock);
    juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
    
    // Split into Low and High1 (Mid + High)
    lp1.process(lowContext);
    hp1.process(highContext); // Temp hold Mid+High in highBuffer
    
    // Split High1 into Mid and High
    midBuffer.copyFrom(0, 0, highBuffer, 0, 0, highBuffer.getNumSamples());
    midBuffer.copyFrom(1, 0, highBuffer, 1, 0, highBuffer.getNumSamples());
    
    lp2.process(midContext);
    hp2.process(highContext);
    
    // Compress each band
    dspComps[0].process(lowContext);
    dspComps[1].process(midContext);
    dspComps[2].process(highContext);
    
    // Apply make-up gains
    lowBuffer.applyGain(juce::Decibels::decibelsToGain(bands[0].gainDb.load()));
    midBuffer.applyGain(juce::Decibels::decibelsToGain(bands[1].gainDb.load()));
    highBuffer.applyGain(juce::Decibels::decibelsToGain(bands[2].gainDb.load()));
    
    // Sum
    buffer.clear();
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.addFrom(ch, 0, lowBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, midBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, highBuffer, ch, 0, buffer.getNumSamples());
    }
}

void MultibandCompressorPlugin::updateDSP() {
    if (currentSampleRate <= 0.0) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    lp1.setCutoffFrequency(lowMidFreq.load());
    hp1.setCutoffFrequency(lowMidFreq.load());
    
    lp2.setCutoffFrequency(midHighFreq.load());
    hp2.setCutoffFrequency(midHighFreq.load());
    
    for (size_t i = 0; i < 3; ++i) {
        dspComps[i].setThreshold(bands[i].threshold.load());
        dspComps[i].setRatio(bands[i].ratio.load());
        dspComps[i].setAttack(bands[i].attack.load());
        dspComps[i].setRelease(bands[i].release.load());
    }
}

} // namespace Nimbus
