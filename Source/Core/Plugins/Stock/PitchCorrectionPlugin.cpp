#include "PitchCorrectionPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "StockPluginUI.h"

namespace Nimbus {

class PitchCorrectionPluginEditor : public juce::Component, private juce::Timer {
public:
    PitchCorrectionPluginEditor(PitchCorrectionPlugin& p) : plugin(p) {
        keyCombo.addItem("C", 1);
        keyCombo.addItem("C#", 2);
        keyCombo.addItem("D", 3);
        keyCombo.addItem("D#", 4);
        keyCombo.addItem("E", 5);
        keyCombo.addItem("F", 6);
        keyCombo.addItem("F#", 7);
        keyCombo.addItem("G", 8);
        keyCombo.addItem("G#", 9);
        keyCombo.addItem("A", 10);
        keyCombo.addItem("A#", 11);
        keyCombo.addItem("B", 12);
        keyCombo.setSelectedId(plugin.getKey() + 1, juce::dontSendNotification);
        keyCombo.onChange = [this]() { plugin.setKey(keyCombo.getSelectedId() - 1); };
        addAndMakeVisible(keyCombo);
        
        scaleCombo.addItem("Major", 1);
        scaleCombo.addItem("Minor", 2);
        scaleCombo.addItem("Chromatic", 3);
        scaleCombo.setSelectedId(plugin.getScale() + 1, juce::dontSendNotification);
        scaleCombo.onChange = [this]() { plugin.setScale(scaleCombo.getSelectedId() - 1); };
        addAndMakeVisible(scaleCombo);
        
        speedSlider = std::make_unique<PluginDial>("Speed", 0.0, 100.0, plugin.getSpeed(), [this](float v) { plugin.setSpeed(v); });
        addAndMakeVisible(speedSlider.get());
        
        header = std::make_unique<PluginHeader>("Pitch Correction");
        addAndMakeVisible(header.get());
        
        startTimerHz(20);
        setSize(320, 180);
    }
    
    ~PitchCorrectionPluginEditor() override { stopTimer(); }
    
    void timerCallback() override {
        if (keyCombo.getSelectedId() != plugin.getKey() + 1) keyCombo.setSelectedId(plugin.getKey() + 1, juce::dontSendNotification);
        if (scaleCombo.getSelectedId() != plugin.getScale() + 1) scaleCombo.setSelectedId(plugin.getScale() + 1, juce::dontSendNotification);
        speedSlider->setValue(plugin.getSpeed());
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
        
        auto comboCol = juce::FlexItem().withWidth(120).withHeight(60).withMargin(juce::FlexItem::Margin(10));
        
        juce::FlexBox fbCol;
        fbCol.flexDirection = juce::FlexBox::Direction::column;
        fbCol.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
        fbCol.items.add(juce::FlexItem(keyCombo).withHeight(25).withMargin(juce::FlexItem::Margin(0, 0, 10, 0)));
        fbCol.items.add(juce::FlexItem(scaleCombo).withHeight(25));
        
        // Use a dummy component to hold the combobox flexbox layout
        // For simplicity, just setBounds manually for the combos inside resized
        fb.items.add(juce::FlexItem(*speedSlider).withWidth(70).withHeight(90));
        
        // Actually it's easier to just do manual layout for this one
        auto comboArea = bounds.removeFromLeft(140).reduced(10);
        keyCombo.setBounds(comboArea.removeFromTop(25));
        comboArea.removeFromTop(10);
        scaleCombo.setBounds(comboArea.removeFromTop(25));
        
        speedSlider->setBounds(bounds.withSizeKeepingCentre(70, 90));
    }

private:
    PitchCorrectionPlugin& plugin;
    juce::ComboBox keyCombo;
    juce::ComboBox scaleCombo;
    std::unique_ptr<PluginDial> speedSlider;
    std::unique_ptr<PluginHeader> header;
};

PitchCorrectionPlugin::PitchCorrectionPlugin() {}

juce::Component* PitchCorrectionPlugin::createEditor() { return new PitchCorrectionPluginEditor(*this); }

void PitchCorrectionPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    currentSampleRate = sampleRate;

    const juce::SpinLock::ScopedLockType sl(processLock);
    delayLine.prepare(spec);
}

void PitchCorrectionPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    delayLine.reset();
}

void PitchCorrectionPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    
    // Note: Real-time pitch correction (autotune) requires complex DSP 
    // including pitch detection (YIN/autocorrelation), note quantization,
    // and phase vocoder or PSOLA pitch shifting. 
    // This is a dummy pass-through for the prototype UI.
}

} // namespace Nimbus
