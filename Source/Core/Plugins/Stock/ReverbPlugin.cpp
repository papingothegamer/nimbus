#include "ReverbPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Colors.h"
#include "StockPluginUI.h"

namespace Nimbus {

struct Preset {
    juce::String name;
    float roomSize, preDelay, reverberance, hfDamping, toneLow, toneHigh, wetGain, dryGain, stereoWidth;
    bool wetOnly;
};

static std::vector<Preset> factoryPresets = {
    {"Acoustic", 50, 10, 75, 100, 21, 100, -14, 0, 80, false},
    {"Ambience", 100, 55, 100, 50, 53, 38, 0, -10, 100, false},
    {"Artificial", 81, 99, 23, 62, 16, 19, -4, 0, 100, false},
    {"Clean", 50, 10, 75, 100, 55, 100, -18, 0, 75, false},
    {"Modern", 50, 10, 75, 100, 55, 100, -15, 0, 75, false},
    {"Vocal I", 70, 20, 40, 99, 100, 50, -12, 0, 70, false},
    {"Vocal II", 50, 0, 50, 99, 50, 100, -1, -1, 70, false},
    {"Dance Vocal", 90, 2, 60, 77, 30, 51, -10, 0, 100, false},
    {"Modern Vocal", 66, 27, 77, 8, 0, 51, -10, 0, 68, false},
    {"Voice Tail", 66, 27, 100, 8, 0, 51, -6, 0, 68, false},
    {"Bathroom", 16, 8, 80, 0, 0, 100, -6, 0, 100, false},
    {"Small Room Bright", 30, 10, 50, 50, 50, 100, -1, -1, 100, false},
    {"Small Room Dark", 30, 10, 50, 50, 100, 0, -1, -1, 100, false},
    {"Medium Room", 75, 10, 40, 50, 100, 70, -1, -1, 70, false},
    {"Large Room", 85, 10, 40, 50, 100, 80, 0, -6, 90, false},
    {"Church Hall", 90, 32, 60, 50, 100, 50, 0, -12, 100, false},
    {"Cathedral", 90, 16, 90, 50, 100, 0, 0, -20, 100, false},
    {"Big Cave", 100, 55, 100, 50, 53, 38, 5, -3, 100, false}
};

void ReverbPlugin::loadPreset(int index) {
    if (index >= 0 && index < factoryPresets.size()) {
        const auto& p = factoryPresets[index];
        roomSize.store(p.roomSize);
        preDelay.store(p.preDelay);
        reverberance.store(p.reverberance);
        damping.store(p.hfDamping);
        lowTone.store(p.toneLow);
        highTone.store(p.toneHigh);
        wetGain.store(p.wetGain);
        dryGain.store(p.dryGain);
        stereoWidth.store(p.stereoWidth);
        wetOnly.store(p.wetOnly);
        updateDSP();
    }
}

class PresetCarousel : public juce::Component {
public:
    PresetCarousel() {
        auto prevDrawable = juce::Drawable::createFromImageData(BinaryData::chevronleft_svg, BinaryData::chevronleft_svgSize);
        if (prevDrawable) prevDrawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        btnPrev.setImages(prevDrawable.get());
        btnPrev.onClick = [this] { if (currentIdx > 0) select(currentIdx - 1); };
        
        auto nextDrawable = juce::Drawable::createFromImageData(BinaryData::chevronright_svg, BinaryData::chevronright_svgSize);
        if (nextDrawable) nextDrawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        btnNext.setImages(nextDrawable.get());
        btnNext.onClick = [this] { if (currentIdx < (int)factoryPresets.size() - 1) select(currentIdx + 1); };
        
        auto resetDrawable = juce::Drawable::createFromImageData(BinaryData::resetdefault_svg, BinaryData::resetdefault_svgSize);
        if (resetDrawable) resetDrawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        btnReset.setImages(resetDrawable.get());
        btnReset.onClick = [this] { if (onReset) onReset(); };
        
        label.setJustificationType(juce::Justification::centred);
        label.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
        label.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
        
        addAndMakeVisible(btnPrev);
        addAndMakeVisible(label);
        addAndMakeVisible(btnNext);
        addAndMakeVisible(btnReset);
        
        label.addMouseListener(this, false);
    }
    
    void select(int i) {
        currentIdx = juce::jlimit(0, (int)factoryPresets.size() - 1, i);
        label.setText(factoryPresets[currentIdx].name, juce::dontSendNotification);
        if (onSelect) onSelect(currentIdx);
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
            juce::PopupMenu m;
            for (int i = 0; i < factoryPresets.size(); ++i) {
                m.addItem(i + 1, factoryPresets[i].name);
            }
            juce::PopupMenu::Options opts;
            opts = opts.withTargetComponent(&label).withParentComponent(getTopLevelComponent()).withStandardItemHeight(24);
            m.showMenuAsync(opts, [this](int res) {
                if (res > 0) select(res - 1);
            });
        }
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        btnReset.setBounds(bounds.removeFromRight(20).reduced(2));
        bounds.removeFromRight(4);
        
        int carouselW = 160;
        auto cBounds = bounds.withSizeKeepingCentre(carouselW, bounds.getHeight());
        btnPrev.setBounds(cBounds.removeFromLeft(20).reduced(0, 4));
        btnNext.setBounds(cBounds.removeFromRight(20).reduced(0, 4));
        label.setBounds(cBounds);
    }
    
    std::function<void(int)> onSelect;
    std::function<void()> onReset;

private:
    juce::DrawableButton btnPrev{"Prev", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnNext{"Next", juce::DrawableButton::ImageFitted};
    juce::Label label;
    juce::DrawableButton btnReset{"Reset", juce::DrawableButton::ImageFitted};
    int currentIdx = 0;
};

class ReverbPluginEditor : public juce::Component, private juce::Timer {
public:
    ReverbPluginEditor(ReverbPlugin& p) : plugin(p) {
        carousel.onSelect = [this](int idx) {
            plugin.loadPreset(idx);
            updateUI();
        };
        carousel.onReset = [this]() {
            plugin.loadPreset(0); // Load 'Acoustic' as default
            carousel.select(0);
            updateUI();
        };
        addAndMakeVisible(carousel);

        roomSizeDial = std::make_unique<NimbusRotaryDial>("Room size", 0.0f, 100.0f, plugin.getRoomSize(), "%", [this](float v) { plugin.setRoomSize(v); });
        widthDial = std::make_unique<NimbusRotaryDial>("Stereo width", 0.0f, 100.0f, plugin.getStereoWidth(), "%", [this](float v) { plugin.setStereoWidth(v); });
        preDelayDial = std::make_unique<NimbusRotaryDial>("Pre-delay", 0.0f, 200.0f, plugin.getPreDelay(), "ms", [this](float v) { plugin.setPreDelay(v); });
        
        dampDial = std::make_unique<NimbusRotaryDial>("Damping", 0.0f, 100.0f, plugin.getDamping(), "%", [this](float v) { plugin.setDamping(v); });
        reverbDial = std::make_unique<NimbusRotaryDial>("Reverberance", 0.0f, 100.0f, plugin.getReverberance(), "%", [this](float v) { plugin.setReverberance(v); });
        lowToneDial = std::make_unique<NimbusRotaryDial>("Low tone", 0.0f, 100.0f, plugin.getLowTone(), "%", [this](float v) { plugin.setLowTone(v); });
        highToneDial = std::make_unique<NimbusRotaryDial>("High tone", 0.0f, 100.0f, plugin.getHighTone(), "%", [this](float v) { plugin.setHighTone(v); });
        wetGainDial = std::make_unique<NimbusRotaryDial>("Wet gain", -20.0f, 10.0f, plugin.getWetGain(), "dB", [this](float v) { plugin.setWetGain(v); });
        dryGainDial = std::make_unique<NimbusRotaryDial>("Dry gain", -20.0f, 10.0f, plugin.getDryGain(), "dB", [this](float v) { plugin.setDryGain(v); });

        addAndMakeVisible(roomSizeDial.get());
        addAndMakeVisible(widthDial.get());
        addAndMakeVisible(preDelayDial.get());
        
        addAndMakeVisible(dampDial.get());
        addAndMakeVisible(reverbDial.get());
        addAndMakeVisible(lowToneDial.get());
        addAndMakeVisible(highToneDial.get());
        addAndMakeVisible(wetGainDial.get());
        addAndMakeVisible(dryGainDial.get());
        
        wetOnlyButton.setButtonText("Wet only");
        wetOnlyButton.setToggleState(plugin.getWetOnly(), juce::dontSendNotification);
        wetOnlyButton.onClick = [this]() { plugin.setWetOnly(wetOnlyButton.getToggleState()); };
        addAndMakeVisible(wetOnlyButton);

        carousel.select(0); // init label
        startTimerHz(20);
        
        // Slightly wider to fit responsive grid gracefully
        setSize(550, 240);
    }
    
    ~ReverbPluginEditor() override { stopTimer(); }
    
    void updateUI() {
        roomSizeDial->setValue(plugin.getRoomSize());
        widthDial->setValue(plugin.getStereoWidth());
        preDelayDial->setValue(plugin.getPreDelay());
        dampDial->setValue(plugin.getDamping());
        reverbDial->setValue(plugin.getReverberance());
        lowToneDial->setValue(plugin.getLowTone());
        highToneDial->setValue(plugin.getHighTone());
        wetGainDial->setValue(plugin.getWetGain());
        dryGainDial->setValue(plugin.getDryGain());
        wetOnlyButton.setToggleState(plugin.getWetOnly(), juce::dontSendNotification);
    }

    void timerCallback() override {
        updateUI();
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        auto bounds = getLocalBounds().reduced(8);
        
        bounds.removeFromTop(32); // carousel
        
        auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.4f);
        bounds.removeFromLeft(8);
        auto rightPanel = bounds;
        
        g.setColour(DesignSystem::Colors::ComponentBackground.brighter(0.05f)); 
        g.fillRoundedRectangle(leftPanel.toFloat(), 6.0f);
        
        g.setColour(DesignSystem::Colors::ComponentBackground.darker(0.05f)); 
        g.fillRoundedRectangle(rightPanel.toFloat(), 6.0f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(8);
        
        auto topRow = bounds.removeFromTop(24);
        wetOnlyButton.setBounds(topRow.removeFromLeft(100));
        carousel.setBounds(topRow);
        
        bounds.removeFromTop(8); // Gap after carousel
        
        auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.4f).reduced(8);
        bounds.removeFromLeft(8); // Spacing between panels
        auto rightPanel = bounds.reduced(8);
        
        // Dynamically compute dial sizes to fit in available space
        // Left Panel: 2 columns, 2 rows
        int dialHLeft = leftPanel.getHeight() / 2;
        int dialWLeft = leftPanel.getWidth() / 2;
        
        roomSizeDial->setBounds(leftPanel.getX(), leftPanel.getY(), dialWLeft, dialHLeft);
        widthDial->setBounds(leftPanel.getX() + dialWLeft, leftPanel.getY(), dialWLeft, dialHLeft);
        preDelayDial->setBounds(leftPanel.getX(), leftPanel.getY() + dialHLeft, dialWLeft, dialHLeft);
        
        // Right Panel: 3 columns, 2 rows
        int dialHRight = rightPanel.getHeight() / 2;
        int dialWRight = rightPanel.getWidth() / 3;
        
        dampDial->setBounds(rightPanel.getX(), rightPanel.getY(), dialWRight, dialHRight);
        reverbDial->setBounds(rightPanel.getX() + dialWRight, rightPanel.getY(), dialWRight, dialHRight);
        lowToneDial->setBounds(rightPanel.getX() + dialWRight * 2, rightPanel.getY(), dialWRight, dialHRight);
        
        highToneDial->setBounds(rightPanel.getX(), rightPanel.getY() + dialHRight, dialWRight, dialHRight);
        wetGainDial->setBounds(rightPanel.getX() + dialWRight, rightPanel.getY() + dialHRight, dialWRight, dialHRight);
        dryGainDial->setBounds(rightPanel.getX() + dialWRight * 2, rightPanel.getY() + dialHRight, dialWRight, dialHRight);
    }

private:
    ReverbPlugin& plugin;
    PresetCarousel carousel;

    std::unique_ptr<NimbusRotaryDial> roomSizeDial;
    std::unique_ptr<NimbusRotaryDial> widthDial;
    std::unique_ptr<NimbusRotaryDial> preDelayDial;
    
    std::unique_ptr<NimbusRotaryDial> dampDial;
    std::unique_ptr<NimbusRotaryDial> reverbDial;
    std::unique_ptr<NimbusRotaryDial> lowToneDial;
    std::unique_ptr<NimbusRotaryDial> highToneDial;
    std::unique_ptr<NimbusRotaryDial> wetGainDial;
    std::unique_ptr<NimbusRotaryDial> dryGainDial;
    
    juce::ToggleButton wetOnlyButton;
};

ReverbPlugin::ReverbPlugin() { updateDSP(); }

juce::Component* ReverbPlugin::createEditor() { return new ReverbPluginEditor(*this); }

void ReverbPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    {const juce::SpinLock::ScopedLockType sl(processLock);
        dspReverb.prepare(spec);
        preDelayLine.prepare(spec);
        lowShelf.prepare(spec);
        highShelf.prepare(spec);
    }

    updateDSP();
}

void ReverbPlugin::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspReverb.reset();
    preDelayLine.reset();
    lowShelf.reset();
    highShelf.reset();
}

void ReverbPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    preDelayLine.process(context);
    dspReverb.process(context);
    lowShelf.process(context);
    highShelf.process(context);
}

void ReverbPlugin::updateDSP() {
    juce::dsp::Reverb::Parameters params;
    params.roomSize = reverberance.load() / 100.0f;
    params.damping = damping.load() / 100.0f;
    params.width = stereoWidth.load() / 100.0f;
    
    float wetLvl = wetOnly.load() ? juce::Decibels::decibelsToGain(wetGain.load()) : juce::Decibels::decibelsToGain(wetGain.load());
    float dryLvl = wetOnly.load() ? 0.0f : juce::Decibels::decibelsToGain(dryGain.load());
    params.wetLevel = wetLvl;
    params.dryLevel = dryLvl;
    params.freezeMode = 0.0f;
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspReverb.setParameters(params);
    
    float ms = preDelay.load();
    if (currentSampleRate > 0) {
        float samples = (ms / 1000.0f) * currentSampleRate;
        preDelayLine.setDelay(juce::jlimit(0.0f, (float)preDelayLine.getMaximumDelayInSamples() - 1.0f, samples));
    }
    
    float lowGain = juce::jmax(0.01f, lowTone.load() / 100.0f);
    float highGain = juce::jmax(0.01f, highTone.load() / 100.0f);
    
    if (currentSampleRate > 0) {
        *lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, 200.0f, 0.707f, lowGain);
        *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 4000.0f, 0.707f, highGain);
    }
}

} // namespace Nimbus
