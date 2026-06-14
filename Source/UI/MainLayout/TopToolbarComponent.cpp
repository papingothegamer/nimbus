#include "TopToolbarComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "BinaryData.h"

namespace Nimbus::MainLayout {

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    auto setupButton = [](juce::DrawableButton& btn, const char* svgData, int svgSize) {
        if (svgData != nullptr) {
            juce::String svgStr(svgData, svgSize);
            svgStr = svgStr.replace("currentColor", "#000000");
            std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(svgStr);
            if (xml != nullptr) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
                    btn.setImages(svg.get());
                }
            }
        }
        btn.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
    };

    setupButton(playButton, BinaryData::play_svg, BinaryData::play_svgSize);
    setupButton(pauseButton, BinaryData::pause_svg, BinaryData::pause_svgSize);
    setupButton(recordButton, BinaryData::circle_svg, BinaryData::circle_svgSize);
    setupButton(loopButton, BinaryData::repeat_svg, BinaryData::repeat_svgSize);
    setupButton(browserToggleButton, BinaryData::panelleft_svg, BinaryData::panelleft_svgSize);
    setupButton(detailToggleButton, BinaryData::panelbottom_svg, BinaryData::panelbottom_svgSize);

    addAndMakeVisible(playButton);
    addAndMakeVisible(pauseButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(loopButton);
    
    addAndMakeVisible(browserToggleButton);
    browserToggleButton.onClick = [this] { if (onBrowserToggle) onBrowserToggle(); };

    addAndMakeVisible(detailToggleButton);
    detailToggleButton.onClick = [this] { if (onDetailToggle) onDetailToggle(); };

    addAndMakeVisible(tempoLabel);
    tempoLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    tempoLabel.setJustificationType(juce::Justification::centred);
    tempoLabel.setText("120.0 BPM", juce::dontSendNotification);
    tempoLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);

    addAndMakeVisible(tapTempoButton);
    tapTempoButton.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::PanelBackground.brighter(0.05f));

    addAndMakeVisible(timeSignatureLabel);
    timeSignatureLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    timeSignatureLabel.setJustificationType(juce::Justification::centred);
    timeSignatureLabel.setText("4 / 4", juce::dontSendNotification);
    timeSignatureLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);

    addAndMakeVisible(metronomeToggle);
    addAndMakeVisible(snapToggle);

    playButton.onClick = [this] { engine.getTransport().play(); };
    pauseButton.onClick = [this] { engine.getTransport().stop(); };
    recordButton.onClick = [this] { /* To be implemented */ };
    loopButton.onClick = [this] { /* To be implemented */ };
    tapTempoButton.onClick = [this] { /* To be implemented */ };
}

TopToolbarComponent::~TopToolbarComponent() = default;

void TopToolbarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Bottom border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TopToolbarComponent::resized() {
    auto bounds = getLocalBounds().reduced(5);
    
    // Toggles left
    browserToggleButton.setBounds(bounds.removeFromLeft(30));
    bounds.removeFromLeft(10);
    detailToggleButton.setBounds(bounds.removeFromLeft(30));

    // Global right
    auto rightBounds = bounds.removeFromRight(200);
    snapToggle.setBounds(rightBounds.removeFromRight(80));
    // Center
    juce::FlexBox fb;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;
    fb.alignItems = juce::FlexBox::AlignItems::center;

    auto addFlexItem = [&](juce::Component& c, float width) {
        fb.items.add(juce::FlexItem(c).withWidth(width).withHeight(30).withMargin(juce::FlexItem::Margin(0, 5, 0, 5)));
    };

    addFlexItem(playButton, 60);
    addFlexItem(pauseButton, 60);
    addFlexItem(recordButton, 60);
    addFlexItem(loopButton, 60);
    addFlexItem(tempoLabel, 100);
    addFlexItem(tapTempoButton, 50);
    addFlexItem(timeSignatureLabel, 60);

    fb.performLayout(bounds);
}

} // namespace Nimbus::MainLayout
