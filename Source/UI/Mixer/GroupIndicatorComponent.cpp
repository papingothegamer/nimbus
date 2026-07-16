#include "GroupIndicatorComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::UI {

GroupIndicatorComponent::GroupIndicatorComponent() {}

void GroupIndicatorComponent::paint(juce::Graphics& g) {
    g.setColour(DesignSystem::Colors::PrimaryAction);
    
    float w = static_cast<float>(getWidth());
    float h = static_cast<float>(getHeight());
    
    if (w > h) { // Horizontal layout (for Channel Strip)
        g.fillRect(0.0f, h - 4.0f, w, 4.0f); // Bottom thick line
        g.fillRect(0.0f, 0.0f, 2.0f, h);     // Left connector
        if (isLastInGroup) g.fillRect(w - 2.0f, 0.0f, 2.0f, h); // Right connector
    } else { // Vertical layout (for Track Header)
        g.fillRect(0.0f, 0.0f, 4.0f, h); // Left thick line
        g.fillRect(0.0f, 0.0f, w, 2.0f); // Top connector
        if (isLastInGroup) g.fillRect(0.0f, h - 2.0f, w, 2.0f); // Bottom connector
    }
}

void GroupIndicatorComponent::setIsLastInGroup(bool isLast) {
    if (isLastInGroup != isLast) {
        isLastInGroup = isLast;
        repaint();
    }
}

} // namespace Nimbus::UI
