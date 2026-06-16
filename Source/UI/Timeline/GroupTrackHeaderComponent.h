#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::Timeline {

class GroupTrackHeaderComponent : public juce::Component, public TimelineProject::Listener {
public:
    GroupTrackHeaderComponent(NimbusEngine& engine, int trackIndex);
    ~GroupTrackHeaderComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void setTrackIndex(int newIndex);
    
    // TimelineProject::Listener
    void trackSelectionChanged() override;
    void trackFoldStateChanged(int changedTrackIndex, bool isFolded) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

private:
    NimbusEngine& engine;
    int trackIndex;

    juce::Label nameLabel;
    juce::TextButton numberButton;
    juce::TextButton foldButton{">"};
    
    // Group controls
    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};

    // Routing placeholders
    juce::Label sourceLabel{"", "Audio From"};
    juce::ComboBox sourceBox;
    juce::Label destLabel{"", "Audio To"};
    juce::ComboBox destBox;

    void updateSelectionState();
    void updateFoldState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GroupTrackHeaderComponent)
};

} // namespace Nimbus::Timeline
