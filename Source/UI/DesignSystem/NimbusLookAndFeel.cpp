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

    setColour(juce::ComboBox::backgroundColourId, Colors::ComponentBackground.brighter(0.05f));
    setColour(juce::ComboBox::textColourId, Colors::TextPrimary);
    setColour(juce::ComboBox::outlineColourId, Colors::ComponentBorder);
    setColour(juce::ComboBox::arrowColourId, Colors::TextSecondary);

    setColour(juce::PopupMenu::backgroundColourId, Colors::ModuleBackground);
    setColour(juce::PopupMenu::textColourId, Colors::TextPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::PrimaryAction);
    setColour(juce::PopupMenu::highlightedTextColourId, Colors::TextPrimary);

    setColour(juce::Label::textColourId, Colors::TextPrimary);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    // TextEditor highlight colours
    setColour(juce::TextEditor::highlightColourId, Colors::PrimaryAction.withAlpha(0.3f));
    setColour(juce::TextEditor::highlightedTextColourId, Colors::TextPrimary);
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
        if (button.isColourSpecified(juce::TextButton::buttonOnColourId)) {
            baseColour = button.findColour(juce::TextButton::buttonOnColourId);
        } else {
            baseColour = Colors::PrimaryAction.withAlpha(0.8f);
        }
    } else if (shouldDrawButtonAsHighlighted) {
        baseColour = Colors::TextSecondary.withAlpha(0.2f); // subtle instant hover
    }

    bool isIconOnly = button.getButtonText().endsWith("_svg");
    if (isIconOnly && !isActive && shouldDrawButtonAsHighlighted) {
        // Do not draw hover background for icon-only buttons, just highlight the icon (handled in drawButtonText)
        return;
    }

    bool transparentBg = button.getProperties().contains("transparentBackground");

    if (!transparentBg) {
        if (baseColour.isOpaque() || baseColour.getAlpha() > 0.0f) {
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, cornerSize);
        }

        if (!isIconOnly) {
            g.setColour(Colors::ComponentBorder);
            g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
        }
    }
}

void NimbusLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    juce::String text = button.getButtonText();
    juce::Colour textColour = button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    
    if (text.endsWith("_svg")) {
        if (auto* svg = getOrCacheSvg(text)) {
            std::unique_ptr<juce::Drawable> clone(svg->createCopy());
            // When toggled on, we make the icon white to contrast against the bright background.
            // Otherwise, we use TextPrimary or TextSecondary based on toggle state.
            juce::Colour iconColor = button.getToggleState() ? Colors::TextPrimary : Colors::TextSecondary;
            // If the button explicitly overrides the icon color when toggled on, use it (though usually it's white).
            if (button.getToggleState() && button.isColourSpecified(juce::TextButton::textColourOnId)) {
                iconColor = button.findColour(juce::TextButton::textColourOnId);
            }
            
            if (!button.getToggleState() && shouldDrawButtonAsHighlighted) {
                iconColor = Colors::TextPrimary; // Highlight icon when hovered
            }

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

// ==============================================================================
// Text Editors

void NimbusLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) {
    bool isLabelEditor = dynamic_cast<juce::Label*>(editor.getParentComponent()) != nullptr;
    if (isLabelEditor) {
        g.fillAll(juce::Colours::transparentBlack);
    } else {
        if (editor.isOpaque()) {
            g.fillAll(editor.findColour(juce::TextEditor::backgroundColourId));
        }
    }
}

void NimbusLookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) {
    bool isLabelEditor = dynamic_cast<juce::Label*>(editor.getParentComponent()) != nullptr;
    if (isLabelEditor) {
        g.setColour(Colors::PrimaryAction);
        g.fillRect(0, height - 2, width, 2);
    } else {
        auto bounds = editor.getLocalBounds();
        if (editor.hasKeyboardFocus(true) && !editor.isReadOnly()) {
            g.setColour(editor.findColour(juce::TextEditor::focusedOutlineColourId));
            g.drawRect(bounds, 2);
        } else {
            g.setColour(editor.findColour(juce::TextEditor::outlineColourId));
            g.drawRect(bounds, 1);
        }
    }
}

// ==============================================================================
// Document Window Title Bar

void NimbusLookAndFeel::drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& g, int w, int h, int titleSpaceX, int titleSpaceW, const juce::Image* icon, bool drawTitleTextOnLeft) {
    g.setColour(Colors::ModuleBackground);
    g.fillRect(0, 0, w, h);
    
    g.setColour(Colors::TextPrimary);
    g.setFont(Typography::getPrimaryFont().withHeight(18.0f).boldened());
    
    int textX = drawTitleTextOnLeft ? titleSpaceX : (w - titleSpaceW) / 2;
    g.drawText(window.getName(), textX, 0, titleSpaceW, h, juce::Justification::centredLeft, true);
    
    g.setColour(Colors::Divider);
    g.fillRect(0, h - 1, w, 1);
}

// ==============================================================================
// Tabs

void NimbusLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool /*isMouseDown*/) {
    auto bounds = button.getLocalBounds().toFloat();
    bool isFront = button.getToggleState();
    
    // Background
    if (isFront) {
        g.setColour(Colors::ComponentBackground.brighter(0.05f));
    } else {
        g.setColour(isMouseOver ? Colors::ComponentBackground.brighter(0.02f) : Colors::ComponentBackground);
    }
    
    // Only small roundness on top corners
    g.fillRoundedRectangle(bounds, 3.0f);
    g.fillRect(bounds.withTrimmedTop(3.0f));

    if (!isFront) {
        g.setColour(Colors::ComponentBorder);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
    }

    // Draw text
    auto textColour = isFront ? Colors::TextPrimary : Colors::TextTitle; // use lighter grey again
    g.setColour(textColour);
    g.setFont(Typography::getPrimaryFont().withHeight(14.0f).boldened());
    
    juce::String text = button.getButtonText();
    if (text.isEmpty()) {
        int index = button.getIndex();
        auto names = button.getTabbedButtonBar().getTabNames();
        if (index >= 0 && index < names.size()) {
            text = names[index];
        } else {
            text = button.getName();
        }
    }
    
    g.drawText(text, bounds, juce::Justification::centred);
    
    // Active line
    if (isFront) {
        g.setColour(Colors::PrimaryAction);
        g.fillRect(bounds.removeFromBottom(2.0f));
    }
}

int NimbusLookAndFeel::getTabButtonBestWidth(juce::TabBarButton& button, int depth) {
    return 100; // Fixed width for settings tabs
}

void NimbusLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) {
    // Don't fill the entire area opaquely — the inactive tab buttons are behind
    // this component in z-order and would be completely hidden.
    // Only draw the bottom divider line.
    g.setColour(Colors::Divider);
    g.fillRect(0, h - 1, w, 1);
}

// ==============================================================================
// ComboBox

void NimbusLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    
    // Minimal modern look (slightly rounded, not too much)
    auto cornerSize = 3.0f;
    g.setColour(Colors::ComponentBackground.brighter(0.05f));
    g.fillRoundedRectangle(bounds, cornerSize);
    
    g.setColour(Colors::ComponentBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    
    // Arrow
    juce::Path arrow;
    auto arrowSize = 4.0f;
    arrow.addTriangle(-arrowSize, -arrowSize * 0.5f, arrowSize, -arrowSize * 0.5f, 0.0f, arrowSize);
    arrow.applyTransform(juce::AffineTransform::translation((float)buttonX + (float)buttonW * 0.5f, (float)buttonY + (float)buttonH * 0.5f));
    g.setColour(Colors::TextSecondary);
    g.fillPath(arrow);
}

juce::Font NimbusLookAndFeel::getComboBoxFont(juce::ComboBox& box) {
    return Typography::getPrimaryFont().withHeight(13.0f);
}

// ==============================================================================
// Label

juce::Font NimbusLookAndFeel::getLabelFont(juce::Label& label) {
    return Typography::getPrimaryFont().withHeight(14.0f);
}

juce::BorderSize<int> NimbusLookAndFeel::getLabelBorderSize(juce::Label&) {
    return juce::BorderSize<int>(0);
}

void NimbusLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));
    
    if (!label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        auto font = getLabelFont(label);
        
        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);
        
        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());
        
        g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
    } else if (label.isEnabled()) {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }
    
    g.drawRect(label.getLocalBounds());
}

// ==============================================================================
// CallOutBox

void NimbusLookAndFeel::drawCallOutBoxBackground(juce::CallOutBox& box, juce::Graphics& g, const juce::Path& path, juce::Image& cachedImage) {
    // Dark sleek background, minimal roundness
    g.setColour(Colors::AppBackground);
    g.fillPath(path);
    g.setColour(Colors::ComponentBorder);
    g.strokePath(path, juce::PathStrokeType(1.0f));
}

// ==============================================================================
// PopupMenu

int NimbusLookAndFeel::getPopupMenuBorderSize() {
    return 4;
}

void NimbusLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height);
    g.setColour(Colors::ModuleBackground);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Add subtle shadow / border
    g.setColour(Colors::ComponentBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}

void NimbusLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                          bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                                          const juce::String& text, const juce::String& shortcutKeyText,
                                          const juce::Drawable* icon, const juce::Colour* textColour) {
    if (isSeparator) {
        auto r = area.reduced(5, 0);
        r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));
        g.setColour(Colors::Divider);
        g.fillRect(r.removeFromTop(1));
        return;
    }

    auto textRect = area.reduced(24, 0);

    if (isHighlighted && isActive) {
        g.setColour(Colors::PrimaryAction);
        g.fillRoundedRectangle(area.reduced(4, 2).toFloat(), 4.0f);
        g.setColour(Colors::TextPrimary);
    } else {
        g.setColour(textColour != nullptr ? *textColour : Colors::TextPrimary);
        if (!isActive)
            g.setOpacity(0.5f);
    }

    g.setFont(Typography::getPrimaryFont().withHeight(14.0f));

    g.drawText(text, textRect, juce::Justification::centredLeft, true);

    if (shortcutKeyText.isNotEmpty()) {
        g.drawText(shortcutKeyText, textRect, juce::Justification::centredRight, true);
    }

    if (isTicked) {
        g.setColour(isHighlighted ? Colors::TextPrimary : Colors::PrimaryAction);
        float cx = area.getX() + 12.0f;
        float cy = area.getCentreY();
        juce::Path tick;
        tick.startNewSubPath(cx - 4.0f, cy);
        tick.lineTo(cx - 1.0f, cy + 3.0f);
        tick.lineTo(cx + 5.0f, cy - 4.0f);
        g.strokePath(tick, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    if (hasSubMenu) {
        auto arrowRect = area.withLeft(area.getRight() - 16).reduced(4, 4).toFloat();
        juce::Path arrow;
        arrow.addTriangle(arrowRect.getX(), arrowRect.getY() + 2.0f,
                          arrowRect.getRight() - 2.0f, arrowRect.getCentreY(),
                          arrowRect.getX(), arrowRect.getBottom() - 2.0f);
        g.fillPath(arrow);
    }
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
