#pragma once

#include <JuceHeader.h>
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

// A collapsible section with a header that can be clicked to expand/collapse
class ExpandableSection : public juce::Component {
public:
    ExpandableSection(const juce::String& title) : sectionTitle(title) {
        setInterceptsMouseClicks(true, true);
    }
    
    void paint(juce::Graphics& g) override {
        auto headerBounds = getLocalBounds().removeFromTop(headerHeight);
        
        // Header background
        g.setColour(DesignSystem::Colors::ComponentBackground.brighter(0.05f));
        g.fillRect(headerBounds);
        
        // Bottom border
        g.setColour(DesignSystem::Colors::Divider);
        g.fillRect(headerBounds.getX(), headerBounds.getBottom() - 1, headerBounds.getWidth(), 1);
        
        // Expand/collapse triangle
        juce::Path triangle;
        auto triArea = headerBounds.removeFromLeft(20).toFloat().reduced(6, 7);
        if (expanded) {
            triangle.addTriangle(triArea.getX(), triArea.getY(), 
                               triArea.getRight(), triArea.getY(),
                               triArea.getCentreX(), triArea.getBottom());
        } else {
            triangle.addTriangle(triArea.getX(), triArea.getY(),
                               triArea.getRight(), triArea.getCentreY(),
                               triArea.getX(), triArea.getBottom());
        }
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.fillPath(triangle);
        
        // Title
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f).withStyle(juce::Font::bold));
        g.drawText(sectionTitle.toUpperCase(), headerBounds.reduced(4, 0), juce::Justification::centredLeft, true);
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.y < headerHeight) {
            expanded = !expanded;
            if (onExpandChanged) onExpandChanged();
            repaint();
        }
    }
    
    void addContentComponent(juce::Component* comp) {
        addAndMakeVisible(comp);
        contentComponents.add(comp);
    }
    
    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(headerHeight);
        
        for (auto* comp : contentComponents) {
            comp->setVisible(expanded);
            if (expanded) {
                // Fixed row height to prevent recursive shrinking
                comp->setBounds(area.removeFromTop(28).reduced(4, 2));
            }
        }
    }
    
    int getDesiredHeight() const {
        if (!expanded) return headerHeight;
        int h = headerHeight;
        for (auto* comp : contentComponents) {
            h += 28;
        }
        return h + 4; // small padding at bottom
    }
    
    bool isExpanded() const { return expanded; }
    void setExpanded(bool e) { expanded = e; if (onExpandChanged) onExpandChanged(); repaint(); }
    
    std::function<void()> onExpandChanged;

private:
    juce::String sectionTitle;
    bool expanded = true;
    int headerHeight = 20;
    juce::Array<juce::Component*> contentComponents;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExpandableSection)
};

} // namespace Nimbus::DetailView
