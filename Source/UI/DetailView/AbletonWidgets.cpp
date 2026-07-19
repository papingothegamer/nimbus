#include "AbletonWidgets.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/NimbusLookAndFeel.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::UI {

// ==============================================================================
AbletonToggleButton::AbletonToggleButton(const juce::String& name) : juce::Button(name) {
    setButtonText(name);
    setClickingTogglesState(true);
}

void AbletonToggleButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = getLocalBounds().toFloat();
    
    if (getToggleState()) {
        g.setColour(DesignSystem::Colors::PrimaryAction);
    } else {
        g.setColour(DesignSystem::Colors::ComponentBackground);
    }
    
    g.fillRect(bounds);
    
    if (getToggleState()) {
        g.setColour(DesignSystem::Colors::TextPrimary);
    } else {
        g.setColour(DesignSystem::Colors::TextPrimary.withAlpha(0.6f));
    }
    
    g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f).boldened());
    g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centred, false);
    
    // Slight border
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(bounds, 1.0f);
}

// ==============================================================================
AbletonNumberBox::AbletonNumberBox() {
    addAndMakeVisible(editLabel);
    editLabel.setEditable(true, false, false);
    editLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    editLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    editLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    editLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(12.0f));
    editLabel.setJustificationType(juce::Justification::centred);
    
    editLabel.onTextChange = [this] {
        double parsed = editLabel.getText().getDoubleValue();
        setValue(parsed, juce::sendNotificationAsync);
    };
}

AbletonNumberBox::~AbletonNumberBox() = default;

void AbletonNumberBox::setRange(double min, double max, double step) {
    minValue = min;
    maxValue = max;
    stepSize = step;
}

void AbletonNumberBox::setValue(double newValue, juce::NotificationType notification) {
    double clamped = juce::jlimit(minValue, maxValue, newValue);
    if (clamped != value) {
        value = clamped;
        repaint();
        if (notification != juce::dontSendNotification && onValueChanged)
            onValueChanged(value);
    }
}

void AbletonNumberBox::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(juce::Colours::transparentBlack);
    g.fillRect(bounds);
    
    // Fill representing value (optional, like Ableton's gain slider)
    float proportion = static_cast<float>((value - minValue) / (maxValue - minValue));
    if (proportion > 0.0f) {
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.fillRect(bounds.withWidth(bounds.getWidth() * proportion));
    }
    
    // Text
    g.setColour(DesignSystem::Colors::TextPrimary);
    g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(12.0f));
    
    juce::String textToDraw;
    if (textFormatter) {
        textToDraw = textFormatter(value);
    } else {
        textToDraw = juce::String(value, decimalPlaces) + suffix;
    }
    
    if (!editLabel.isBeingEdited()) {
        g.drawText(textToDraw, getLocalBounds(), juce::Justification::centred, false);
    }
    
    // Border
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(bounds, 1.0f);
}

void AbletonNumberBox::resized() {
    editLabel.setBounds(getLocalBounds());
}

void AbletonNumberBox::mouseDown(const juce::MouseEvent& e) {
    dragStartValue = static_cast<float>(value);
}

void AbletonNumberBox::mouseDrag(const juce::MouseEvent& e) {
    float delta = -e.getDistanceFromDragStartY() * static_cast<float>(stepSize);
    
    if (e.mods.isShiftDown()) {
        delta *= 0.1f; // fine adjustment
    }
    
    setValue(dragStartValue + delta, juce::sendNotificationAsync);
}

void AbletonNumberBox::mouseDoubleClick(const juce::MouseEvent& e) {
    setValue(defaultValue, juce::sendNotificationAsync);
}

// ==============================================================================
AbletonPanel::AbletonPanel(const juce::String& title) : panelTitle(title) {
    addAndMakeVisible(contentContainer);
}

void AbletonPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(DesignSystem::Colors::PanelBackground);
    g.fillRect(bounds);
    
    // Title bar
    juce::Rectangle<float> titleBar(0, 0, bounds.getWidth(), 18);
    g.setColour(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    g.fillRect(titleBar);
    
    g.setColour(DesignSystem::Colors::TextPrimary);
    g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f).boldened());
    
    // Draw Chevron
    auto& lnf = dynamic_cast<DesignSystem::NimbusLookAndFeel&>(getLookAndFeel());
    auto iconName = folded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold;
    if (auto* svg = lnf.getOrCacheSvg(iconName)) {
        svg->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        juce::Rectangle<float> iconBounds(5, (18 - 10) / 2.0f, 10, 10);
        svg->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
    }
    
    g.drawText(panelTitle.toUpperCase(), titleBar.withTrimmedLeft(20), juce::Justification::centredLeft, false);
    
    // Border
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(bounds, 1.0f);
}

void AbletonPanel::setFolded(bool shouldBeFolded) {
    if (folded != shouldBeFolded) {
        folded = shouldBeFolded;
        contentContainer.setVisible(!folded);
        repaint();
        if (onFoldStateChanged) onFoldStateChanged();
    }
}

void AbletonPanel::mouseDown(const juce::MouseEvent& e) {
    if (e.y < 18) {
        setFolded(!folded);
    }
}

void AbletonPanel::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(18); // Header
    
    if (!folded) {
        contentContainer.setBounds(bounds.reduced(5)); // Padding
    } else {
        contentContainer.setBounds(0, 0, 0, 0);
    }
}

void AbletonPanel::addContent(juce::Component* comp) {
    contentContainer.addAndMakeVisible(comp);
    contents.add(comp);
}

void AbletonPanel::clearContent() {
    for (auto* c : contents) {
        contentContainer.removeChildComponent(c);
    }
    contents.clear();
}

// ==============================================================================
void AbletonVerticalGainSlider::GainSliderLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                                                        const juce::Slider::SliderStyle style, juce::Slider& slider) {
    float meterWidth = width - 12.0f; 
    juce::Rectangle<float> meterBounds(0, 0, meterWidth, height);
    
    // Background Track
    g.setColour(juce::Colour(0xff111111));
    g.fillRect(meterBounds);
    
    // Ticks
    g.setColour(juce::Colour(0xff555555));
    for (int db = -60; db <= 10; db += 5) {
        float proportion = 1.0f - ((db + 60.0f) / 70.0f);
        float tickY = proportion * height;
        g.fillRect(meterWidth + 2.0f, tickY, 4.0f, 1.0f);
    }
    
    // Fill up to the sliderPos (orange)
    float fillHeight = height - sliderPos;
    if (fillHeight > 0) {
        g.setColour(DesignSystem::Colors::Mute);
        g.fillRect(0.0f, sliderPos, meterWidth, fillHeight);
    }

    // Horizontal line overlaid perfectly on top of the meter
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.fillRect((float)x, sliderPos - 1.0f, meterWidth, 2.0f);

    // Ableton-style triangle pointer thumb on the right
    juce::Path p;
    float pW = 6.0f;
    float pH = 8.0f;
    float pX = meterWidth + 1.0f; 
    
    p.addTriangle(pX, sliderPos, pX + pW, sliderPos - pH * 0.5f, pX + pW, sliderPos + pH * 0.5f);
    g.setColour(juce::Colour(0xff888888));
    g.fillPath(p);
    g.setColour(juce::Colour(0xff222222));
    g.strokePath(p, juce::PathStrokeType(1.0f)); 
}

AbletonVerticalGainSlider::AbletonVerticalGainSlider() {
    setLookAndFeel(&customLaf);
    setSliderStyle(juce::Slider::LinearVertical);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRange(-60.0, 10.0, 0.1); 
    setValue(0.0);
}

AbletonVerticalGainSlider::~AbletonVerticalGainSlider() {
    setLookAndFeel(nullptr);
}

} // namespace Nimbus::UI
