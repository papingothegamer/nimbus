#include "MultibandCompressorPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"
#include <array>

namespace Nimbus {

class GRMeter : public juce::Component {
public:
    void setGR(float grDb) {
        currentGR = juce::jlimit(-30.0f, 0.0f, grDb);
        repaint();
    }
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(DesignSystem::Colors::PanelBackground.darker(0.2f));
        g.fillRoundedRectangle(bounds, 2.0f);
        float width = juce::jmap(currentGR, 0.0f, -30.0f, 0.0f, bounds.getWidth());
        if (width > 0) {
            juce::Rectangle<float> grBounds(bounds.getWidth() - width, 0, width, bounds.getHeight());
            g.setColour(juce::Colour::fromString("#4CAF50")); // Green for OTT
            g.fillRoundedRectangle(grBounds, 2.0f);
        }
    }
private:
    float currentGR = 0.0f;
};

class MultibandCompressorPluginEditor : public juce::Component, private juce::Timer {
public:
    MultibandCompressorPluginEditor(MultibandCompressorPlugin& p) : plugin(p) {
        lowMidSlider = std::make_unique<NimbusRotaryDial>("Low/Mid", 20.0, 1000.0, plugin.getLowMidFreq(), " Hz", [this](float v) { plugin.setLowMidFreq(v); });
        midHighSlider = std::make_unique<NimbusRotaryDial>("Mid/High", 500.0, 15000.0, plugin.getMidHighFreq(), " Hz", [this](float v) { plugin.setMidHighFreq(v); });
        lowMidSlider->setDefaultValue(150.0);
        midHighSlider->setDefaultValue(2500.0);
        lowMidSlider->getSlider().setSkewFactorFromMidPoint(200.0);
        midHighSlider->getSlider().setSkewFactorFromMidPoint(2000.0);
        lowMidSlider->getSlider().setNumDecimalPlacesToDisplay(0);
        midHighSlider->getSlider().setNumDecimalPlacesToDisplay(0);
        
        addAndMakeVisible(lowMidSlider.get());
        addAndMakeVisible(midHighSlider.get());
        
        for (int i = 0; i < 3; ++i) {
            auto& band = plugin.getBand(i);
            threshSliders[i] = std::make_unique<NimbusRotaryDial>("Thresh", -60.0, 0.0, band.threshold.load(), " dB", [this, i](float v) { plugin.getBand(i).threshold.store(v); plugin.updateDSP(); });
            ratioSliders[i] = std::make_unique<NimbusRotaryDial>("Ratio", 1.0, 20.0, band.ratio.load(), ":1", [this, i](float v) { plugin.getBand(i).ratio.store(v); plugin.updateDSP(); });
            gainSliders[i] = std::make_unique<NimbusRotaryDial>("Gain", -24.0, 24.0, band.gainDb.load(), " dB", [this, i](float v) { plugin.getBand(i).gainDb.store(v); plugin.updateDSP(); });
            
            threshSliders[i]->setDefaultValue(-40.0);
            ratioSliders[i]->setDefaultValue(4.0);
            gainSliders[i]->setDefaultValue(0.0);
            
            grMeters[i] = std::make_unique<GRMeter>();
            
            addAndMakeVisible(threshSliders[i].get());
            addAndMakeVisible(ratioSliders[i].get());
            addAndMakeVisible(gainSliders[i].get());
            addAndMakeVisible(grMeters[i].get());
        }
        
        header = std::make_unique<PluginHeader>("Multiband Compressor");
        addAndMakeVisible(header.get());
        
        startTimerHz(30);
        setSize(480, 320);
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
            grMeters[i]->setGR(band.currentGR.load());
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        auto bounds = getLocalBounds().reduced(10);
        
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(bounds.withTrimmedTop(25).toFloat(), 4.0f);
        
        const char* titles[] = {"Low Band", "Mid Band", "High Band"};
        int h = (bounds.getHeight() - 25) / 3;
        for (int i = 0; i < 3; ++i) {
            juce::Rectangle<int> row(bounds.getX(), bounds.getY() + 25 + i * h, bounds.getWidth(), h);
            g.setColour(DesignSystem::Colors::TextPrimary.withAlpha(0.6f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f).boldened());
            
            juce::Rectangle<int> titleArea = row.removeFromLeft(80);
            if (i == 0 || i == 2) {
                titleArea = titleArea.removeFromTop(20); // Make space for the crossover dial
            }
            g.drawText(titles[2 - i], titleArea, juce::Justification::centred, false);
            
            if (i > 0) {
                g.setColour(DesignSystem::Colors::Divider);
                g.fillRect(bounds.getX(), bounds.getY() + 25 + i * h, bounds.getWidth(), 1);
            }
        }
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        header->setBounds(bounds.removeFromTop(25));
        
        int h = bounds.getHeight() / 3;
        for (int i = 2; i >= 0; --i) { // 2=High on top, 1=Mid, 0=Low on bottom
            auto row = bounds.removeFromTop(h).reduced(5);
            
            auto leftCol = row.removeFromLeft(80);
            leftCol.removeFromTop(20); // offset for title
            if (i == 2) {
                midHighSlider->setBounds(leftCol.withSizeKeepingCentre(70, 70));
            } else if (i == 0) {
                lowMidSlider->setBounds(leftCol.withSizeKeepingCentre(70, 70));
            }
            
            auto rightDials = row.removeFromRight(180);
            threshSliders[i]->setBounds(rightDials.removeFromLeft(60).withSizeKeepingCentre(60, 60));
            ratioSliders[i]->setBounds(rightDials.removeFromLeft(60).withSizeKeepingCentre(60, 60));
            gainSliders[i]->setBounds(rightDials.removeFromLeft(60).withSizeKeepingCentre(60, 60));
            
            grMeters[i]->setBounds(row.reduced(10, 15)); // padding left/right and top/bottom
        }
    }

private:
    MultibandCompressorPlugin& plugin;
    std::unique_ptr<NimbusRotaryDial> lowMidSlider, midHighSlider;
    std::array<std::unique_ptr<NimbusRotaryDial>, 3> threshSliders;
    std::array<std::unique_ptr<NimbusRotaryDial>, 3> ratioSliders;
    std::array<std::unique_ptr<NimbusRotaryDial>, 3> gainSliders;
    std::array<std::unique_ptr<GRMeter>, 3> grMeters;
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
    for (int ch = 0; ch < midBuffer.getNumChannels(); ++ch) {
        if (ch < highBuffer.getNumChannels()) {
            midBuffer.copyFrom(ch, 0, highBuffer, ch, 0, highBuffer.getNumSamples());
        }
    }
    
    lp2.process(midContext);
    hp2.process(highContext);
    
    // Compress each band and measure GR
    auto measureGR = [&](juce::AudioBuffer<float>& buf, juce::dsp::Compressor<float>& comp, int bandIdx, juce::dsp::ProcessContextReplacing<float>& ctx) {
        float inPeak = 0.0f;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch) {
            inPeak = juce::jmax(inPeak, buf.getMagnitude(ch, 0, buf.getNumSamples()));
        }
        
        comp.process(ctx);
        
        float outPeak = 0.0f;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch) {
            outPeak = juce::jmax(outPeak, buf.getMagnitude(ch, 0, buf.getNumSamples()));
        }
        
        if (inPeak > 0.0001f && outPeak > 0.0001f) {
            float gr = juce::Decibels::gainToDecibels(outPeak / inPeak);
            float current = bands[bandIdx].currentGR.load();
            bands[bandIdx].currentGR.store(current * 0.8f + gr * 0.2f);
        } else {
            bands[bandIdx].currentGR.store(bands[bandIdx].currentGR.load() * 0.9f);
        }
    };
    
    measureGR(lowBuffer, dspComps[0], 0, lowContext);
    measureGR(midBuffer, dspComps[1], 1, midContext);
    measureGR(highBuffer, dspComps[2], 2, highContext);
    
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
