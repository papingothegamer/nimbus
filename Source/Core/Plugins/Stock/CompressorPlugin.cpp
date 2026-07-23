#include "CompressorPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Colors.h"
#include "StockPluginUI.h"

namespace Nimbus {

struct CompressorPreset {
    juce::String name;
    float threshold;
    float ratio;
    float attack;
    float release;
    float makeupGain;
};

static std::vector<CompressorPreset> compressorPresets = {
    // General
    {"Modern", -14.0f, 4.0f, 0.2f, 210.0f, 0.0f},
    {"Glue Compressor", -22.0f, 1.2f, 20.0f, 1000.0f, 2.5f},
    {"Gentle", -18.0f, 1.5f, 1.0f, 100.0f, 0.0f},
    {"Beat Booster", -18.0f, 4.0f, 14.0f, 9.0f, 3.0f},
    
    // Mastering
    {"Deep Dive Master", -23.5f, 1.2f, 52.2f, 12.2f, 1.6f},
    {"Beefy Master", -16.8f, 1.2f, 49.6f, 17.9f, 2.5f},
    {"Make It Right Master", -6.5f, 1.4f, 1.0f, 1.0f, 1.6f},
    {"Brick Wall Master", -10.0f, 100.0f, 0.1f, 2.0f, 3.0f},
    
    // Vocal
    {"Lead Vocals", -14.0f, 5.2f, 1.0f, 60.0f, 0.0f},
    {"Fat Vocals", -32.0f, 1.7f, 86.9f, 15.2f, 2.5f},
    {"Power Vocals", -16.8f, 1.5f, 2.8f, 356.3f, 3.0f},
    {"Vocal Control", -15.0f, 3.0f, 0.1f, 196.0f, 4.5f},
    {"Vocal Touch-Up", -22.0f, 1.5f, 2.0f, 450.0f, 3.6f},
    {"Voice Memos Balancer", -22.3f, 10.1f, 6.5f, 3.6f, 4.5f},
    {"Podcast/Radio", -15.0f, 3.0f, 15.0f, 40.0f, 1.0f},
    
    // Instrument
    {"Piano", -16.0f, 2.0f, 0.2f, 150.0f, 1.0f},
    {"Acoustic Guitar", -15.0f, 2.5f, 15.0f, 225.0f, 1.5f},
    {"Bass Guitar", -13.0f, 3.0f, 1.0f, 50.0f, 0.0f},
    {"Strings", -15.0f, 1.8f, 30.0f, 400.0f, 2.5f},
    {"Kick Drums", -14.0f, 4.0f, 30.0f, 120.0f, 2.0f},
    
    // New additions
    {"Snare Drum", -18.0f, 4.0f, 5.0f, 150.0f, 2.0f},
    {"Drum Bus", -20.0f, 2.5f, 30.0f, 100.0f, 1.5f},
    {"Electric Guitar", -20.0f, 3.0f, 10.0f, 80.0f, 1.5f}
};

void CompressorPlugin::loadPreset(int index) {
    if (index >= 0 && index < compressorPresets.size()) {
        const auto& p = compressorPresets[index];
        threshold.store(p.threshold);
        ratio.store(p.ratio);
        attack.store(p.attack);
        release.store(p.release);
        makeupGain.store(p.makeupGain);
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
        btnNext.onClick = [this] { if (currentIdx < (int)compressorPresets.size() - 1) select(currentIdx + 1); };
        
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
        currentIdx = juce::jlimit(0, (int)compressorPresets.size() - 1, i);
        label.setText(compressorPresets[currentIdx].name, juce::dontSendNotification);
        if (onSelect) onSelect(currentIdx);
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
            juce::PopupMenu m;
            for (int i = 0; i < compressorPresets.size(); ++i) {
                m.addItem(i + 1, compressorPresets[i].name);
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

class CompressionGraphComponent : public juce::Component {
public:
    CompressionGraphComponent(CompressorPlugin& p) : plugin(p) {}

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(DesignSystem::Colors::AppBackground.darker(0.2f));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Grid
        g.setColour(DesignSystem::Colors::ComponentBorder.withAlpha(0.3f));
        const float minDb = -60.0f;
        const float maxDb = 0.0f;
        const float dbRange = maxDb - minDb;
        
        for (float db = minDb; db <= maxDb; db += 10.0f) {
            float norm = (db - minDb) / dbRange;
            float x = bounds.getX() + norm * bounds.getWidth();
            float y = bounds.getBottom() - norm * bounds.getHeight();
            g.drawHorizontalLine(juce::roundToInt(y), bounds.getX(), bounds.getRight());
            g.drawVerticalLine(juce::roundToInt(x), bounds.getY(), bounds.getBottom());
        }

        float threshold = plugin.getThreshold();
        float ratio = plugin.getRatio();
        float makeup = plugin.getMakeupGain();
        
        auto getPointForDb = [&](float inDb) -> juce::Point<float> {
            float outDb = inDb;
            if (inDb > threshold) {
                outDb = threshold + (inDb - threshold) / ratio;
            }
            outDb += makeup;
            
            float normX = juce::jlimit(0.0f, 1.0f, (inDb - minDb) / dbRange);
            float normY = juce::jlimit(0.0f, 1.0f, (outDb - minDb) / dbRange);
            
            return {
                bounds.getX() + normX * bounds.getWidth(),
                bounds.getBottom() - normY * bounds.getHeight()
            };
        };

        // Draw Transfer Curve
        juce::Path curve;
        curve.startNewSubPath(getPointForDb(minDb));
        curve.lineTo(getPointForDb(threshold));
        curve.lineTo(getPointForDb(maxDb));

        g.setColour(DesignSystem::Colors::PrimaryAction);
        g.strokePath(curve, juce::PathStrokeType(2.0f));

        // Live Dot
        float liveIn = plugin.getCurrentInputLevel();
        if (liveIn > minDb) {
            auto livePt = getPointForDb(liveIn);
            g.setColour(juce::Colours::white);
            g.fillEllipse(livePt.x - 3.0f, livePt.y - 3.0f, 6.0f, 6.0f);
        }
    }
private:
    CompressorPlugin& plugin;
};

class CompressorPluginEditor : public juce::Component, private juce::Timer {
public:
    CompressorPluginEditor(CompressorPlugin& p) : plugin(p), graph(p) {
        carousel.onSelect = [this](int idx) {
            plugin.loadPreset(idx);
            updateUI();
        };
        carousel.onReset = [this]() {
            plugin.loadPreset(0);
            carousel.select(0);
            updateUI();
        };
        addAndMakeVisible(carousel);

        threshSlider = std::make_unique<PluginDial>("Thresh", -60.0, 0.0, plugin.getThreshold(), " dB", [this](float v) { plugin.setThreshold(v); });
        ratioSlider = std::make_unique<PluginDial>("Ratio", 1.0, 20.0, plugin.getRatio(), ":1", [this](float v) { plugin.setRatio(v); });
        attackSlider = std::make_unique<PluginDial>("Attack", 0.1, 500.0, plugin.getAttack(), " ms", [this](float v) { plugin.setAttack(v); });
        releaseSlider = std::make_unique<PluginDial>("Release", 10.0, 2000.0, plugin.getRelease(), " ms", [this](float v) { plugin.setRelease(v); });
        makeupSlider = std::make_unique<PluginDial>("Make-up", -20.0, 20.0, plugin.getMakeupGain(), " dB", [this](float v) { plugin.setMakeupGain(v); });
        
        addAndMakeVisible(threshSlider.get());
        addAndMakeVisible(ratioSlider.get());
        addAndMakeVisible(attackSlider.get());
        addAndMakeVisible(releaseSlider.get());
        addAndMakeVisible(makeupSlider.get());
        addAndMakeVisible(graph);

        carousel.select(0);
        
        startTimerHz(30);
        setSize(520, 280);
    }
    
    ~CompressorPluginEditor() override { stopTimer(); }
    
    void updateUI() {
        threshSlider->setValue(plugin.getThreshold());
        ratioSlider->setValue(plugin.getRatio());
        attackSlider->setValue(plugin.getAttack());
        releaseSlider->setValue(plugin.getRelease());
        makeupSlider->setValue(plugin.getMakeupGain());
        graph.repaint();
    }

    void timerCallback() override {
        updateUI();
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        auto bounds = getLocalBounds().reduced(8);
        
        bounds.removeFromTop(32); // carousel area
        
        auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.5f);
        bounds.removeFromLeft(8);
        
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(leftPanel.toFloat(), 4.0f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(8);
        
        carousel.setBounds(bounds.removeFromTop(32).reduced(0, 4));
        
        auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.5f);
        bounds.removeFromLeft(8);
        
        auto graphBounds = bounds;
        graph.setBounds(graphBounds);

        int dialW = leftPanel.getWidth() / 3;
        int dialH = leftPanel.getHeight() / 2;
        
        threshSlider->setBounds(leftPanel.getX(), leftPanel.getY(), dialW, dialH);
        ratioSlider->setBounds(leftPanel.getX() + dialW, leftPanel.getY(), dialW, dialH);
        attackSlider->setBounds(leftPanel.getX() + dialW * 2, leftPanel.getY(), dialW, dialH);
        
        // Center the two bottom knobs
        int offset = dialW / 2;
        releaseSlider->setBounds(leftPanel.getX() + offset, leftPanel.getY() + dialH, dialW, dialH);
        makeupSlider->setBounds(leftPanel.getX() + offset + dialW, leftPanel.getY() + dialH, dialW, dialH);
    }

private:
    CompressorPlugin& plugin;
    PresetCarousel carousel;
    std::unique_ptr<PluginDial> threshSlider;
    std::unique_ptr<PluginDial> ratioSlider;
    std::unique_ptr<PluginDial> attackSlider;
    std::unique_ptr<PluginDial> releaseSlider;
    std::unique_ptr<PluginDial> makeupSlider;
    CompressionGraphComponent graph;
};

CompressorPlugin::CompressorPlugin() { updateDSP(); }

juce::Component* CompressorPlugin::createEditor() { return new CompressorPluginEditor(*this); }

void CompressorPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 2;

    {
        const juce::SpinLock::ScopedLockType sl(processLock);
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
    
    float maxIn = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        maxIn = juce::jmax(maxIn, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    float inDb = juce::Decibels::gainToDecibels(maxIn, -100.0f);
    currentInputLevel.store(inDb);

    {
        const juce::SpinLock::ScopedLockType sl(processLock);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        dspComp.process(context);
    }

    float maxOut = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        maxOut = juce::jmax(maxOut, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    }
    float outDb = juce::Decibels::gainToDecibels(maxOut, -100.0f);
    currentGainReduction.store(inDb - outDb);

    float makeupLinear = juce::Decibels::decibelsToGain(makeupGain.load());
    if (makeupLinear != 1.0f) {
        buffer.applyGain(makeupLinear);
    }
}

void CompressorPlugin::updateDSP() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    dspComp.setThreshold(threshold.load());
    dspComp.setRatio(ratio.load());
    dspComp.setAttack(attack.load());
    dspComp.setRelease(release.load());
}

void CompressorPlugin::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream stream(destData, true);
    stream.writeFloat(threshold.load());
    stream.writeFloat(ratio.load());
    stream.writeFloat(attack.load());
    stream.writeFloat(release.load());
    stream.writeFloat(makeupGain.load());
    stream.writeBool(bypassed.load());
}

void CompressorPlugin::setStateInformation(const void* data, int sizeInBytes) {
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    if (!stream.isExhausted()) threshold.store(stream.readFloat());
    if (!stream.isExhausted()) ratio.store(stream.readFloat());
    if (!stream.isExhausted()) attack.store(stream.readFloat());
    if (!stream.isExhausted()) release.store(stream.readFloat());
    if (!stream.isExhausted()) makeupGain.store(stream.readFloat());
    if (!stream.isExhausted()) bypassed.store(stream.readBool());
    updateDSP();
}

} // namespace Nimbus
