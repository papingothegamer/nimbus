#pragma once

#include <JuceHeader.h>

namespace Nimbus::UI {

// A simple component to visually indicate that a track belongs to a group.
// It can be placed inside Track Headers and Channel Strips.
class GroupIndicatorComponent : public juce::Component {
public:
    GroupIndicatorComponent();
    ~GroupIndicatorComponent() override = default;

    void paint(juce::Graphics& g) override;
    
    // Set whether this indicator is for the last track in the group (so it can draw an ending bracket)
    void setIsLastInGroup(bool isLast);

private:
    bool isLastInGroup = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GroupIndicatorComponent)
};

} // namespace Nimbus::UI
