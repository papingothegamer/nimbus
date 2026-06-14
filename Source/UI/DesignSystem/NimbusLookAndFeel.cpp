#include "NimbusLookAndFeel.h"
#include "Colors.h"
#include "Typography.h"

namespace Nimbus::DesignSystem {

NimbusLookAndFeel::NimbusLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, Colors::AppBackground);
    setColour(juce::Slider::thumbColourId, Colors::PrimaryAction);
    setColour(juce::Slider::rotarySliderFillColourId, Colors::PrimaryAction);
    setColour(juce::Slider::rotarySliderOutlineColourId, Colors::ComponentBorder);
    setColour(juce::TextButton::buttonColourId, Colors::ComponentBackground);
    setColour(juce::TextButton::buttonOnColourId, Colors::PrimaryAction);
    setColour(juce::TextButton::textColourOffId, Colors::TextPrimary);
    setColour(juce::TextButton::textColourOnId, Colors::TextPrimary);

    setColour(juce::TabbedButtonBar::tabOutlineColourId, Colors::ComponentBorder);
    setColour(juce::TabbedButtonBar::tabTextColourId, Colors::TextSecondary);
    setColour(juce::TabbedButtonBar::frontTextColourId, Colors::TextPrimary);
}

NimbusLookAndFeel::~NimbusLookAndFeel() = default;

void NimbusLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider) {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    
    // Background track
    g.setColour(Colors::ComponentBackground);
    g.fillEllipse(rx, ry, rw, rw);
    g.setColour(Colors::ComponentBorder);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    // Fill arc
    juce::Path filledArc;
    filledArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.0f);
    g.setColour(Colors::PrimaryAction);
    g.fillPath(filledArc);

    // Indicator line
    juce::Path pointer;
    auto pointerLength = radius * 0.8f;
    auto pointerThickness = 2.0f;
    pointer.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(Colors::TextPrimary);
    g.fillPath(pointer);
}

void NimbusLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 2.0f; // sharp, technical corners

    auto baseColour = backgroundColour;
    if (shouldDrawButtonAsDown || button.getToggleState()) {
        baseColour = Colors::PrimaryAction;
    } else if (shouldDrawButtonAsHighlighted) {
        baseColour = baseColour.brighter(0.1f); // subtle instant hover
    }

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    g.setColour(Colors::ComponentBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
}

void NimbusLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    juce::Font font = Typography::getPrimaryFont();
    g.setFont(font);
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId));
    
    auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
    auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
    auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
    auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    auto textBounds = button.getLocalBounds().withTrimmedLeft(leftIndent)
                                             .withTrimmedRight(rightIndent)
                                             .withTrimmedTop(yIndent)
                                             .withTrimmedBottom(yIndent);

    g.drawFittedText(button.getButtonText(), textBounds, juce::Justification::centred, 2);
}

} // namespace Nimbus::DesignSystem
