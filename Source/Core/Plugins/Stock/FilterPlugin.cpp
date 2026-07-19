#include "FilterPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class FilterPluginEditor : public juce::Component, private juce::Timer {
public:
    FilterPluginEditor(FilterPlugin& p) : plugin(p) {
        typeCombo.addItem("Lowpass", 1);
        typeCombo.addItem("Highpass", 2);
        typeCombo.addItem("Bandpass", 3);

        typeCombo.setSelectedId(plugin.getFilterType() + 1, juce::dontSendNotification);
        typeCombo.onChange = [this]() {
            plugin.setFilterType(typeCombo.getSelectedId() - 1);
        };
        addAndMakeVisible(typeCombo);
        
        cutoffSlider = std::make_unique<PluginDial>("Freq", 20.0, 20000.0, plugin.getCutoffFrequency(), " Hz", [this](float v) { plugin.setCutoffFrequency(v); });
        addAndMakeVisible(cutoffSlider.get());
        
        resSlider = std::make_unique<PluginDial>("Res", 0.1, 10.0, plugin.getResonance(), "", [this](float v) { plugin.setResonance(v); });
        addAndMakeVisible(resSlider.get());
        
        header = std::make_unique<PluginHeader>("Filter");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(240, 180); // Embedded size
    }
    
    ~FilterPluginEditor() override {
        stopTimer();
    }
    
    void timerCallback() override {
        cutoffSlider->setValue(plugin.getCutoffFrequency());
        resSlider->setValue(plugin.getResonance());
        if (plugin.getFilterType() != typeCombo.getSelectedId() - 1) {
            typeCombo.setSelectedId(plugin.getFilterType() + 1, juce::dontSendNotification);
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
        
        typeCombo.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(120, 24));
        
        juce::FlexBox fb;
        fb.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        fb.alignItems = juce::FlexBox::AlignItems::center;
        
        fb.items.add(juce::FlexItem(*cutoffSlider).withWidth(70).withHeight(90));
        fb.items.add(juce::FlexItem(*resSlider).withWidth(70).withHeight(90));
        
        fb.performLayout(bounds);
    }

private:
    FilterPlugin& plugin;
    juce::ComboBox typeCombo;
    std::unique_ptr<PluginDial> cutoffSlider;
    std::unique_ptr<PluginDial> resSlider;
    std::unique_ptr<PluginHeader> header;
};


FilterPlugin::FilterPlugin() {
    updateDSP();
}

juce::Component* FilterPlugin::createEditor() {
    return new FilterPluginEditor(*this);
}

void FilterPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    currentSampleRate = sampleRate;

    {const juce::SpinLock::ScopedLockType sl(processLock);
    for (auto& filter : dspFilters) filter.prepare(spec);
    

    }


    updateDSP();
}

void FilterPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    for (auto& filter : dspFilters) filter.reset();
}

void FilterPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;

    const juce::SpinLock::ScopedLockType sl(processLock);
    
    int numChannels = juce::jmin((int)buffer.getNumChannels(), 2);
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            data[i] = dspFilters[ch].processSample(ch, data[i]);
        }
    }
}

void FilterPlugin::updateDSP() {
    if (currentSampleRate <= 0.0) return;
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    for (auto& filter : dspFilters) {
        filter.setType(static_cast<juce::dsp::StateVariableTPTFilterType>(filterType.load()));
        filter.setCutoffFrequency(cutoff.load());
        filter.setResonance(resonance.load());
    }
}

} // namespace Nimbus
