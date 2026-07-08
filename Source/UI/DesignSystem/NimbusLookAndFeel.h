#pragma once

#include <JuceHeader.h>
#include <map>
#include <memory>

namespace Nimbus::DesignSystem {

class NimbusLookAndFeel : public juce::LookAndFeel_V4 {
public:
    NimbusLookAndFeel();
    ~NimbusLookAndFeel() override;

    // Overrides to make UI flat and matte
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
                          
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                              
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    
    // Text Editors
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;

    // Document Window Title Bar
    void drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics&, int w, int h, int titleSpaceX, int titleSpaceW, const juce::Image* icon, bool drawTitleTextOnLeft) override;

    // Tabbed Component
    void drawTabButton(juce::TabBarButton&, juce::Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar&, juce::Graphics&, int w, int h) override;
    int getTabButtonBestWidth(juce::TabBarButton& button, int depth) override;

    // ComboBox
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    
    // Label
    juce::Font getLabelFont(juce::Label&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    juce::BorderSize<int> getLabelBorderSize(juce::Label&) override;
    
    // CallOutBox
    void drawCallOutBoxBackground(juce::CallOutBox&, juce::Graphics&, const juce::Path&, juce::Image&) override;
    
    // PopupMenu
    void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;
    int getPopupMenuBorderSize() override;

private:
    juce::Drawable* getOrCacheSvg(const juce::String& resourceName);
    std::map<juce::String, std::unique_ptr<juce::Drawable>> svgCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NimbusLookAndFeel)
};

} // namespace Nimbus::DesignSystem
