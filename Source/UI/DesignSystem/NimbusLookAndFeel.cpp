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
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Draw flat background arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(Colors::TextSecondary.withAlpha(0.2f));
    g.strokePath(backgroundArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw filled arc
    bool isPan = slider.getProperties().contains("isPan");
    if (isPan ? std::abs(sliderPosProportional - 0.5f) > 0.001f : sliderPosProportional > 0.0f) {
        juce::Path filledArc;
        float startArc = isPan ? (rotaryStartAngle + rotaryEndAngle) * 0.5f : rotaryStartAngle;
        if (isPan) {
            if (sliderPosProportional < 0.5f) {
                filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, angle, startArc, true);
            } else {
                filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, startArc, angle, true);
            }
        } else {
            filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        }
        g.setColour(Colors::PrimaryAction);
        g.strokePath(filledArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Draw pointer
    juce::Path p;
    auto pointerLength = radius * 0.8f;
    p.addRectangle(-1.0f, -radius, 2.0f, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(Colors::TextPrimary);
    g.fillPath(p);
}

void NimbusLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 2.0f; // sharp, technical corners

    auto baseColour = backgroundColour;
    bool isActive = shouldDrawButtonAsDown || button.getToggleState();
    
    if (isActive) {
        baseColour = Colors::PrimaryAction.withAlpha(0.8f);
    } else if (shouldDrawButtonAsHighlighted) {
        baseColour = Colors::TextSecondary.withAlpha(0.2f); // subtle instant hover
    }

    bool isIconOnly = button.getButtonText().endsWith("_svg");

    if (baseColour.isOpaque() || baseColour.getAlpha() > 0.0f) {
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);
    }

    if (!isIconOnly) {
        g.setColour(Colors::ComponentBorder);
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }
}

void NimbusLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    juce::String text = button.getButtonText();
    juce::Colour textColour = button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    
    if (text.endsWith("_svg")) {
        if (auto* svg = getOrCacheSvg(text)) {
            std::unique_ptr<juce::Drawable> clone(svg->createCopy());
            // When toggled on, we make the icon white to contrast against the bright PrimaryAction background.
            // Otherwise, we use TextPrimary.
            juce::Colour iconColor = button.getToggleState() ? Colors::TextPrimary : Colors::TextSecondary;
            clone->replaceColour(juce::Colours::black, iconColor);
            clone->replaceColour(juce::Colours::white, iconColor);
            
            // Adjust bounds to allow icons to be large, but not cramped.
            auto iconBounds = button.getLocalBounds().toFloat();
            
            // If the button is larger than standard 24x24, we don't want the icon to scale too massively
            float dim = juce::jmin(iconBounds.getWidth(), iconBounds.getHeight());
            float iconSize = juce::jmax(14.0f, dim * 0.65f);
            float w = juce::jmin(iconBounds.getWidth() - 4.0f, iconSize);
            float h = juce::jmin(iconBounds.getHeight() - 4.0f, iconSize);
            auto drawBounds = juce::Rectangle<float>(0, 0, w, h).withCentre(iconBounds.getCentre());
            
            clone->drawWithin(g, drawBounds, juce::RectanglePlacement::centred, 1.0f);
            return;
        }
    }

    auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
    auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
    auto fontHeight = juce::roundToInt(Typography::getPrimaryFont().getHeight() * 0.6f);
    auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    auto textBounds = button.getLocalBounds().withTrimmedLeft(leftIndent)
                                             .withTrimmedRight(rightIndent)
                                             .withTrimmedTop(yIndent)
                                             .withTrimmedBottom(yIndent);

    juce::Font font = Typography::getPrimaryFont();
    g.setFont(font);
    g.setColour(textColour);
    g.drawText(text, textBounds, juce::Justification::centred);
}

juce::Font NimbusLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight) {
    return Typography::getPrimaryFont();
}

juce::Drawable* NimbusLookAndFeel::getOrCacheSvg(const juce::String& resourceName) {
    if (svgCache.find(resourceName) != svgCache.end()) {
        return svgCache[resourceName].get();
    }
    
    int size = 0;
    if (auto* data = BinaryData::getNamedResource(resourceName.toRawUTF8(), size)) {
        auto drawable = juce::Drawable::createFromImageData(data, size);
        if (drawable) {
            auto* raw = drawable.get();
            svgCache[resourceName] = std::move(drawable);
            return raw;
        }
    }
    return nullptr;
}

} // namespace Nimbus::DesignSystem
