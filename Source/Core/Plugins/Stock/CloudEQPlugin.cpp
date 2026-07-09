#include "CloudEQPlugin.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Colors.h"
#include "StockPluginUI.h"
#include <algorithm>
#include <cmath>

namespace Nimbus {

static juce::Colour getBandColor(int index) {
    switch (index) {
        case 0: return juce::Colour::fromString("#FF5252"); // Red
        case 1: return juce::Colour::fromString("#FF9800"); // Orange
        case 2: return juce::Colour::fromString("#FFEB3B"); // Yellow
        case 3: return juce::Colour::fromString("#4CAF50"); // Green
        case 4: return juce::Colour::fromString("#00BCD4"); // Cyan
        case 5: return juce::Colour::fromString("#2196F3"); // Blue
        case 6: return juce::Colour::fromString("#9C27B0"); // Purple
        case 7: return juce::Colour::fromString("#E91E63"); // Pink
        default: return DesignSystem::Colors::PrimaryAction;
    }
}

class CloudEQGraph : public juce::Component, private juce::Timer {
public:
    CloudEQGraph(CloudEQPlugin& p, std::function<void(int)> onBandSelected) 
        : plugin(p), bandSelectedCallback(onBandSelected) {
        fftData.resize(CloudEQPlugin::fftSize * 2, 0.0f);
        startTimerHz(30);
    }
    ~CloudEQGraph() override { stopTimer(); }

    void timerCallback() override {
        if (plugin.isNextFFTBlockReady()) {
            plugin.copyFFTData(fftData.data());
            plugin.setNextFFTBlockReady(false);
            plugin.forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
        }
        repaint();
    }

    
    void mouseDown(const juce::MouseEvent& e) override {
        int closestBand = -1;
        float minDistance = 30.0f;
        for (int i = 0; i < 8; ++i) {
            if (plugin.getBand(i).enabled.load()) {
                float bx = getXForFreq(plugin.getBand(i).freq.load());
                float by = getYForGain(plugin.getBand(i).gainDb.load());
                float dist = juce::Point<float>(bx, by).getDistanceFrom(e.position);
                if (dist < minDistance) {
                    minDistance = dist;
                    closestBand = i;
                }
            }
        }
        if (closestBand != -1) {
            selectedBand = closestBand;
            if (bandSelectedCallback) bandSelectedCallback(selectedBand);
            repaint();
        }
    }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        if (selectedBand >= 0 && selectedBand < 8) {
            auto& band = plugin.getBand(selectedBand);
            float newFreq = getFreqForX(e.position.x);
            float newGain = getGainForY(e.position.y);
            band.freq.store(juce::jlimit(20.0f, 20000.0f, newFreq));
            band.gainDb.store(juce::jlimit(-24.0f, 24.0f, newGain));
            plugin.updateDSP();
            if (bandSelectedCallback) bandSelectedCallback(selectedBand);
            repaint();
        }
    }

    void setSelectedBand(int idx) { selectedBand = idx; repaint(); }
    void setRtaEnabled(bool enabled) {
        rtaEnabled = enabled;
        repaint();
    }

private:
    float getXForFreq(float freq) {
        return juce::jmap(std::log10(std::max(20.0f, freq)), std::log10(20.0f), std::log10(20000.0f), 0.0f, (float)getWidth());
    }
    float getFreqForX(float x) {
        return std::pow(10.0f, juce::jmap(x, 0.0f, (float)getWidth(), std::log10(20.0f), std::log10(20000.0f)));
    }
    float getYForGain(float gain) {
        return juce::jmap(gain, 24.0f, -24.0f, 0.0f, (float)getHeight());
    }
    float getGainForY(float y) {
        return juce::jmap(y, 0.0f, (float)getHeight(), 24.0f, -24.0f);
    }
    
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds) {
        if (!rtaEnabled) return;
        juce::Path p;
        bool started = false;
        auto mindB = -80.0f;
        auto maxdB = 0.0f;
        for (int i = 0; i < CloudEQPlugin::fftSize / 2; ++i) {
            float freq = ((float)i / CloudEQPlugin::fftSize) * 44100.0f; 
            if (freq < 20.0f) continue;
            if (freq > 20000.0f) break;
            
            float mag = std::abs(fftData[i]);
            float level = juce::Decibels::gainToDecibels(mag) - juce::Decibels::gainToDecibels((float)CloudEQPlugin::fftSize);
            level = juce::jmap(juce::jlimit(mindB, maxdB, level), mindB, maxdB, 0.0f, 1.0f);
            
            if (std::isnan(level) || std::isinf(level)) level = 0.0f;

            float x = getXForFreq(freq);
            float y = bounds.getHeight() - (level * bounds.getHeight());
            if (!started) {
                p.startNewSubPath(x, bounds.getHeight());
                p.lineTo(x, y);
                started = true;
            } else {
                p.lineTo(x, y);
            }
        }
        if (started) {
            p.lineTo(bounds.getWidth(), bounds.getHeight());
            p.closeSubPath();
            g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.15f));
            g.fillPath(p);
        }
    }

    void drawEQCurve(juce::Graphics& g, juce::Rectangle<float> bounds) {
        juce::Path p;
        bool started = false;
        
        // Accurate frequency response calculation
        double sampleRate = 44100.0;
        const int numPoints = 200;
        
        std::vector<juce::dsp::IIR::Coefficients<float>::Ptr> activeCoeffs;
        for (int i = 0; i < 8; ++i) {
            if (plugin.getBand(i).enabled.load()) {
                auto& band = plugin.getBand(i);
                float f = band.freq.load();
                float q = band.q.load();
                float gain = juce::Decibels::decibelsToGain(band.gainDb.load());
                
                juce::dsp::IIR::Coefficients<float>::Ptr c;
                switch (band.type.load()) {
                    case CloudEQPlugin::FilterType::LowCut: c = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, f, q); break;
                    case CloudEQPlugin::FilterType::LowShelf: c = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::Bell: c = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::HighShelf: c = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::HighCut: c = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, f, q); break;
                }
                if (c != nullptr) activeCoeffs.push_back(c);
            }
        }

        for (int i = 0; i <= numPoints; ++i) {
            float x = (float)i / numPoints * bounds.getWidth();
            float freq = getFreqForX(x);
            
            float mag = 1.0f;
            for (auto& c : activeCoeffs) {
                mag *= c->getMagnitudeForFrequency(freq, sampleRate);
            }
            
            float db = juce::Decibels::gainToDecibels(mag);
            float y = getYForGain(db);
            
            if (!started) {
                p.startNewSubPath(x, y);
                started = true;
            } else {
                p.lineTo(x, y);
            }
        }
        
        // Fill gradient under curve
        // Fill gradient under TOTAL response curve
        juce::Path fillPath = p;
        fillPath.lineTo(bounds.getWidth(), bounds.getHeight());
        fillPath.lineTo(0.0f, bounds.getHeight());
        fillPath.closeSubPath();
        
        juce::ColourGradient grad(DesignSystem::Colors::PrimaryAction.withAlpha(0.15f), 0.0f, bounds.getHeight() / 2,
                                  DesignSystem::Colors::PrimaryAction.withAlpha(0.0f), 0.0f, bounds.getHeight(), false);
        g.setGradientFill(grad);
        g.fillPath(fillPath);

        // Draw total combined curve
        g.setColour(DesignSystem::Colors::TextPrimary.withAlpha(0.6f));
        g.strokePath(p, juce::PathStrokeType(1.5f));

        // Draw individual band curves
        for (int b = 0; b < 8; ++b) {
            if (plugin.getBand(b).enabled.load()) {
                auto& band = plugin.getBand(b);
                float f = band.freq.load();
                float q = band.q.load();
                float gain = juce::Decibels::decibelsToGain(band.gainDb.load());
                
                juce::dsp::IIR::Coefficients<float>::Ptr c;
                switch (band.type.load()) {
                    case CloudEQPlugin::FilterType::LowCut: c = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, f, q); break;
                    case CloudEQPlugin::FilterType::LowShelf: c = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::Bell: c = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::HighShelf: c = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, f, q, gain); break;
                    case CloudEQPlugin::FilterType::HighCut: c = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, f, q); break;
                }
                
                if (c != nullptr) {
                    juce::Path bandPath;
                    bool bandStarted = false;
                    for (int i = 0; i <= numPoints; ++i) {
                        float x = (float)i / numPoints * bounds.getWidth();
                        float freq = getFreqForX(x);
                        float mag = c->getMagnitudeForFrequency(freq, sampleRate);
                        float db = juce::Decibels::gainToDecibels(mag);
                        float y = getYForGain(db);
                        if (!bandStarted) {
                            bandPath.startNewSubPath(x, y);
                            bandStarted = true;
                        } else {
                            bandPath.lineTo(x, y);
                        }
                    }
                    g.setColour(getBandColor(b).withAlpha(b == selectedBand ? 0.9f : 0.4f));
                    g.strokePath(bandPath, juce::PathStrokeType(b == selectedBand ? 2.0f : 1.0f));
                }
            }
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Grid
        g.setColour(DesignSystem::Colors::ComponentBorder.withAlpha(0.2f));
        for (float f : {50.0f, 100.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f}) {
            float x = getXForFreq(f);
            g.drawVerticalLine((int)x, 0.0f, bounds.getHeight());
        }
        g.drawHorizontalLine((int)(bounds.getHeight() / 2), 0.0f, bounds.getWidth());
        
        // RTA
        drawSpectrum(g, bounds);
        
        // Curve
        drawEQCurve(g, bounds);
        
        // Nodes
        for (int i = 0; i < 8; ++i) {
            if (plugin.getBand(i).enabled.load()) {
                auto& band = plugin.getBand(i);
                float x = getXForFreq(band.freq.load());
                float y = getYForGain(band.gainDb.load());
                
                bool isSel = (i == selectedBand);
                float radius = isSel ? 6.0f : 4.5f;
                juce::Rectangle<float> nodeRect(x - radius, y - radius, radius * 2.0f, radius * 2.0f);
                
                // Fabfilter-style node: solid filled circle
                g.setColour(getBandColor(i).withAlpha(isSel ? 1.0f : 0.6f));
                g.fillEllipse(nodeRect);
                
                // White outline if selected, otherwise darker outline
                g.setColour(isSel ? juce::Colours::white : DesignSystem::Colors::ComponentBackground);
                g.drawEllipse(nodeRect, 1.5f);
            }
        }
    }

    CloudEQPlugin& plugin;
    std::function<void(int)> bandSelectedCallback;
    int selectedBand = 0;
    std::vector<float> fftData;
    bool rtaEnabled = false;
};


class EQShapeLookAndFeel : public juce::LookAndFeel_V4 {
public:
    EQShapeLookAndFeel() {}

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                            bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColourToUse) override 
    {
        g.fillAll(isHighlighted ? DesignSystem::Colors::PrimaryAction.withAlpha(0.2f) : DesignSystem::Colors::ComponentBackground.brighter(0.1f));
        g.setColour(isHighlighted ? juce::Colours::white : DesignSystem::Colors::TextPrimary);
        g.setFont(12.0f);
        
        juce::Rectangle<int> textRect = area.withTrimmedLeft(40).withTrimmedRight(10);
        g.drawText(text, textRect, juce::Justification::centredLeft, true);
        
        juce::Rectangle<float> iconRect(area.getX() + 10.0f, area.getY() + (area.getHeight() - 12.0f) / 2.0f, 20.0f, 12.0f);
        drawShapeIcon(g, iconRect, text, isHighlighted ? juce::Colours::white : DesignSystem::Colors::PrimaryAction);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        g.setColour(DesignSystem::Colors::ComponentBackground.darker(0.1f));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(DesignSystem::Colors::ComponentBorder);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        juce::Rectangle<float> iconRect = bounds.withSizeKeepingCentre(20.0f, 12.0f);
        drawShapeIcon(g, iconRect, box.getText(), DesignSystem::Colors::TextPrimary);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(0, 0, 0, 0); // Hide the text label completely
    }

private:
    void drawShapeIcon(juce::Graphics& g, juce::Rectangle<float> iconRect, const juce::String& text, juce::Colour color) {
        g.setColour(color);
        juce::Path p;
        if (text == "Low Cut") {
            p.startNewSubPath(iconRect.getX(), iconRect.getBottom());
            p.quadraticTo(iconRect.getX() + iconRect.getWidth() / 2, iconRect.getY() + 2, iconRect.getRight(), iconRect.getY() + 2);
        } else if (text == "Low Shelf") {
            p.startNewSubPath(iconRect.getX(), iconRect.getBottom());
            p.lineTo(iconRect.getX() + iconRect.getWidth() / 3, iconRect.getBottom());
            p.quadraticTo(iconRect.getX() + iconRect.getWidth() / 2, iconRect.getY() + 2, iconRect.getRight() - iconRect.getWidth() / 3, iconRect.getY() + 2);
            p.lineTo(iconRect.getRight(), iconRect.getY() + 2);
        } else if (text == "Bell") {
            p.startNewSubPath(iconRect.getX(), iconRect.getBottom() - 2);
            p.quadraticTo(iconRect.getX() + iconRect.getWidth() / 2, iconRect.getY() - iconRect.getHeight() + 4, iconRect.getRight(), iconRect.getBottom() - 2);
        } else if (text == "High Shelf") {
            p.startNewSubPath(iconRect.getX(), iconRect.getY() + 2);
            p.lineTo(iconRect.getX() + iconRect.getWidth() / 3, iconRect.getY() + 2);
            p.quadraticTo(iconRect.getX() + iconRect.getWidth() / 2, iconRect.getBottom(), iconRect.getRight() - iconRect.getWidth() / 3, iconRect.getBottom());
            p.lineTo(iconRect.getRight(), iconRect.getBottom());
        } else if (text == "High Cut") {
            p.startNewSubPath(iconRect.getX(), iconRect.getY() + 2);
            p.quadraticTo(iconRect.getX() + iconRect.getWidth() / 2, iconRect.getY() + 2, iconRect.getRight(), iconRect.getBottom());
        }
        g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved));
    }
};

class CloudEQPluginEditor : public juce::Component {
public:
    CloudEQPluginEditor(CloudEQPlugin& p, bool isExpandedView = false) : plugin(p), isExpanded(isExpandedView) {
        
        graph = std::make_unique<CloudEQGraph>(plugin, [this](int b) {
            currentBand = b;
            updateSelectedBand();
        });
        addAndMakeVisible(*graph);
        
        for (int i = 0; i < 8; ++i) {
            btnBands[i].setButtonText(juce::String(i + 1));
            btnBands[i].setClickingTogglesState(true);
            btnBands[i].setToggleState(plugin.getBand(i).enabled.load(), juce::dontSendNotification);
            
            // Custom button paint to show active state properly
            btnBands[i].setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::ComponentBackground.brighter(0.1f));
            btnBands[i].setColour(juce::TextButton::buttonOnColourId, getBandColor(i).withAlpha(0.7f));
            btnBands[i].setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            btnBands[i].setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
            
            btnBands[i].onClick = [this, i] {
                plugin.getBand(i).enabled.store(btnBands[i].getToggleState());
                plugin.updateDSP();
                currentBand = i;
                updateSelectedBand();
                graph->repaint();
            };
            addAndMakeVisible(btnBands[i]);
        }
        
        // Single shape selector for the currently active band
        comboShape.setLookAndFeel(&shapeLookAndFeel);
        comboShape.addItemList({"Low Cut", "Low Shelf", "Bell", "High Shelf", "High Cut"}, 1);
        comboShape.onChange = [this] {
            plugin.getBand(currentBand).type.store(static_cast<CloudEQPlugin::FilterType>(comboShape.getSelectedId() - 1));
            plugin.updateDSP();
            graph->repaint();
        };
        addAndMakeVisible(comboShape);
        
        slFreq = std::make_unique<PluginDial>("Freq", 20.0f, 20000.0f, 1000.0f, [this](float v) {
            plugin.getBand(currentBand).freq.store(v);
            plugin.updateDSP();
            graph->repaint();
        }, " Hz");
        slFreq->setDefaultValue(1000.0f);
        slFreq->getSlider().setSkewFactorFromMidPoint(1000.0f);
        
        slGain = std::make_unique<PluginDial>("Gain", -24.0f, 24.0f, 0.0f, [this](float v) {
            plugin.getBand(currentBand).gainDb.store(v);
            plugin.updateDSP();
            graph->repaint();
        }, " dB");
        slGain->setDefaultValue(0.0f);
        
        slQ = std::make_unique<PluginDial>("Q", 0.1f, 18.0f, 0.707f, [this](float v) {
            plugin.getBand(currentBand).q.store(v);
            plugin.updateDSP();
            graph->repaint();
        });
        slQ->setDefaultValue(0.707f);
        
        addAndMakeVisible(slFreq.get());
        addAndMakeVisible(slGain.get());
        addAndMakeVisible(slQ.get());
        
        // Add Toggle Buttons for Expand and RTA
        auto rtaDrawable = juce::Drawable::createFromImageData(BinaryData::waveform_svg, BinaryData::waveform_svgSize);
        if (rtaDrawable) rtaDrawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        btnRTA.setImages(rtaDrawable.get());
        btnRTA.setTooltip("Toggle RTA");
        btnRTA.onClick = [this] {
            isRTAEnabled = !isRTAEnabled;
            graph->setRtaEnabled(isRTAEnabled);
            btnRTA.setAlpha(isRTAEnabled ? 1.0f : 0.5f);
        };
        btnRTA.setAlpha(0.5f);
        addAndMakeVisible(btnRTA);
                         
        if (!isExpanded) {
            auto expandDrawable = juce::Drawable::createFromImageData(BinaryData::arrowupdropcircleoutline_svg, BinaryData::arrowupdropcircleoutline_svgSize);
            if (expandDrawable) expandDrawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
            btnExpand.setImages(expandDrawable.get());
            btnExpand.setTooltip("Expand EQ");
            btnExpand.onClick = [this] {
                auto* popover = new CloudEQPluginEditor(plugin, true);
                juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(popover), getScreenBounds(), nullptr);
            };
            addAndMakeVisible(btnExpand);
        }
        
        updateSelectedBand();
        if (isExpanded) {
            setSize(800, 500);
        } else {
            setSize(550, 240);
        }
    }

    ~CloudEQPluginEditor() override {
        comboShape.setLookAndFeel(nullptr);
    }
    
    void updateSelectedBand() {
        auto& band = plugin.getBand(currentBand);
        comboShape.setSelectedId((int)band.type.load() + 1, juce::dontSendNotification);
        slFreq->setValue(band.freq.load());
        slGain->setValue(band.gainDb.load());
        slQ->setValue(band.q.load());
        graph->setSelectedBand(currentBand);
        
        // Highlight the selected band button
        for (int i = 0; i < 8; ++i) {
            btnBands[i].setColour(juce::TextButton::buttonColourId, 
                i == currentBand ? DesignSystem::Colors::ComponentBackground.brighter(0.3f) : DesignSystem::Colors::ComponentBackground.brighter(0.1f));
        }
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::AppBackground);
        
        auto bounds = getLocalBounds().reduced(8);
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(12);
        
        // Dials and shape selector on the left
        auto leftColumn = bounds.removeFromLeft(64);
        comboShape.setBounds(leftColumn.removeFromTop(24).reduced(2, 0));
        leftColumn.removeFromTop(8); // gap
        slFreq->setBounds(leftColumn.removeFromTop(60));
        slGain->setBounds(leftColumn.removeFromTop(60));
        slQ->setBounds(leftColumn.removeFromTop(60));
        
        bounds.removeFromLeft(8); // spacing
        
        // Graph and buttons taking the rest
        auto rightArea = bounds;
        
        auto btnArea = rightArea.removeFromRight(24);
        if (!isExpanded) {
            btnExpand.setBounds(btnArea.removeFromTop(24).reduced(2));
            btnArea.removeFromTop(8);
        }
        btnRTA.setBounds(btnArea.removeFromTop(24).reduced(2));
        
        rightArea.removeFromRight(8); // spacing before graph
        
        auto bottomBar = rightArea.removeFromBottom(30); // Compact bottom bar for bands
        rightArea.removeFromBottom(8); // spacing between graph and band tools
        
        graph->setBounds(rightArea);
        
        // 8 band selectors at the bottom
        int bandWidth = bottomBar.getWidth() / 8;
        for (int i = 0; i < 8; ++i) {
            auto bBounds = bottomBar.removeFromLeft(bandWidth).reduced(2);
            btnBands[i].setBounds(bBounds);
        }
    }

private:
    CloudEQPlugin& plugin;
    EQShapeLookAndFeel shapeLookAndFeel;
    std::unique_ptr<CloudEQGraph> graph;
    int currentBand = 0;
    juce::ComboBox comboShape;
    std::unique_ptr<PluginDial> slFreq, slGain, slQ;
    std::array<juce::TextButton, 8> btnBands;
    juce::DrawableButton btnRTA{"RTA", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnExpand{"Expand", juce::DrawableButton::ImageFitted};
    bool isRTAEnabled = false;
    bool isExpanded = false;
};

// ==========================================
// Plugin Core
// ==========================================

CloudEQPlugin::CloudEQPlugin() {
    fifo.resize(fftSize, 0.0f);
    scopeData.resize(fftSize * 2, 0.0f);
    
    bands[0].type = FilterType::LowCut;   bands[0].freq = 30.0f;   bands[0].q = 0.707f; bands[0].gainDb = 0.0f; bands[0].enabled = true;
    bands[1].type = FilterType::LowShelf; bands[1].freq = 100.0f;  bands[1].q = 0.707f; bands[1].gainDb = 0.0f; bands[1].enabled = true;
    bands[2].type = FilterType::Bell;     bands[2].freq = 250.0f;  bands[2].q = 1.0f;   bands[2].gainDb = 0.0f; bands[2].enabled = true;
    bands[3].type = FilterType::Bell;     bands[3].freq = 500.0f;  bands[3].q = 1.0f;   bands[3].gainDb = 0.0f; bands[3].enabled = false;
    bands[4].type = FilterType::Bell;     bands[4].freq = 1000.0f; bands[4].q = 1.0f;   bands[4].gainDb = 0.0f; bands[4].enabled = false;
    bands[5].type = FilterType::Bell;     bands[5].freq = 2000.0f; bands[5].q = 1.0f;   bands[5].gainDb = 0.0f; bands[5].enabled = true;
    bands[6].type = FilterType::HighShelf;bands[6].freq = 8000.0f; bands[6].q = 0.707f; bands[6].gainDb = 0.0f; bands[6].enabled = true;
    bands[7].type = FilterType::HighCut;  bands[7].freq = 18000.0f;bands[7].q = 0.707f; bands[7].gainDb = 0.0f; bands[7].enabled = true;
    
    updateDSP();
}

juce::Component* CloudEQPlugin::createEditor() { return new CloudEQPluginEditor(*this); }

void CloudEQPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = maximumExpectedSamplesPerBlock;
    spec.numChannels = 1;

    currentSampleRate = sampleRate;
    
    {
        const juce::SpinLock::ScopedLockType sl(processLock);
        for (int i = 0; i < 8; ++i) {
            leftFilters[i].prepare(spec);
            rightFilters[i].prepare(spec);
        }
    }
    
    updateDSP();
}

void CloudEQPlugin::releaseResources() {
}

void CloudEQPlugin::pushNextSampleIntoFifo(float sample) noexcept {
    if (fifoIndex == fftSize) {
        if (!nextFFTBlockReady) {
            juce::FloatVectorOperations::copy(scopeData.data(), fifo.data(), fftSize);
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

void CloudEQPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (bypassed.load()) return;
    
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    if (numChannels > 0) {
        float* leftChannel = buffer.getWritePointer(0);
        for (int s = 0; s < numSamples; ++s) {
            float sample = leftChannel[s];
            for (size_t i = 0; i < 8; ++i) {
                if (bands[i].enabled.load()) {
                    sample = leftFilters[i].processSample(sample);
                }
            }
            leftChannel[s] = sample;
            pushNextSampleIntoFifo(sample);
        }
    }
    
    if (numChannels > 1) {
        float* rightChannel = buffer.getWritePointer(1);
        for (int s = 0; s < numSamples; ++s) {
            float sample = rightChannel[s];
            for (size_t i = 0; i < 8; ++i) {
                if (bands[i].enabled.load()) {
                    sample = rightFilters[i].processSample(sample);
                }
            }
            rightChannel[s] = sample;
        }
    }
}

void CloudEQPlugin::updateDSP() {
    if (currentSampleRate <= 0.0) return;
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    for (size_t i = 0; i < 8; ++i) {
        auto& band = bands[i];
        float freq = band.freq.load();
        float q = band.q.load();
        float gain = juce::Decibels::decibelsToGain(band.gainDb.load());
        FilterType type = band.type.load();
        
        juce::dsp::IIR::Coefficients<float>::Ptr newCoeffs;
        
        switch (type) {
            case FilterType::LowCut:
                newCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, freq, q);
                break;
            case FilterType::LowShelf:
                newCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, freq, q, gain);
                break;
            case FilterType::Bell:
                newCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, freq, q, gain);
                break;
            case FilterType::HighShelf:
                newCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, freq, q, gain);
                break;
            case FilterType::HighCut:
                newCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, freq, q);
                break;
        }

        if (newCoeffs != nullptr) {
            leftFilters[i].coefficients = newCoeffs;
            rightFilters[i].coefficients = newCoeffs;
        }
    }
}

} // namespace Nimbus
