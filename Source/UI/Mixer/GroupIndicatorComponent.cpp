#include "GroupIndicatorComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::UI {

GroupIndicatorComponent::GroupIndicatorComponent() {}

void GroupIndicatorComponent::paint(juce::Graphics& g) {
    g.setColour(DesignSystem::Colors::PrimaryAction);
    
    float w = static_cast<float>(getWidth());
    float h = static_cast<float>(getHeight());
    
    // Draw a vertical thick rectangle on the left edge
    g.fillRect(0.0f, 0.0f, 4.0f, h);
    
    // Draw horizontal bracket lines to connect
    g.fillRect(0.0f, 0.0f, w, 2.0f);
    
    if (isLastInGroup) {
        g.fillRect(0.0f, h - 2.0f, w, 2.0f);
    }
}

void GroupIndicatorComponent::setIsLastInGroup(bool isLast) {
    if (isLastInGroup != isLast) {
        isLastInGroup = isLast;
        repaint();
    }
}

} // namespace Nimbus::UI
