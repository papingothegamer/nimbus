#include "AbletonWidgets.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::UI {

// ==============================================================================
AbletonToggleButton::AbletonToggleButton(const juce::String& name) : juce::Button(name) {
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
    g.drawText(getName(), getLocalBounds(), juce::Justification::centred, false);
    
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
    g.setColour(DesignSystem::Colors::ComponentBackground);
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
    
    juce::String text = juce::String(value, decimalPlaces) + suffix;
    
    if (!editLabel.isBeingEdited()) {
        g.drawText(text, getLocalBounds(), juce::Justification::centred, false);
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
    g.drawText(panelTitle.toUpperCase(), titleBar.withTrimmedLeft(5), juce::Justification::centredLeft, false);
    
    // Border
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(bounds, 1.0f);
}

void AbletonPanel::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(18); // Header
    
    contentContainer.setBounds(bounds.reduced(5)); // Padding
    
    // Layout contents vertically or as grid? We will just let the caller position them via setBounds on the components themselves, or we can lay them out here.
    // For now, let caller manage `contentContainer` or we just bounds them.
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

} // namespace Nimbus::UI
