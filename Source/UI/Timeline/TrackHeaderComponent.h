#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/Mixer/GroupIndicatorComponent.h"

namespace Nimbus::Timeline {

class TrackHeaderComponent : public juce::Component, public juce::Label::Listener, public TimelineProject::Listener {
public:
    TrackHeaderComponent(NimbusEngine& engine, int trackIndex);
    ~TrackHeaderComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setTrackIndex(int newIndex);
    
    void labelTextChanged(juce::Label* labelThatHasChanged) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // TimelineProject::Listener
    void trackMuteChanged(int track, bool isMuted) override;
    void trackSelectionChanged() override;

    void trackFoldStateChanged(int track, bool isFolded) override;

    UI::GroupIndicatorComponent groupIndicator;

    void updateSelectionState();

private:
    NimbusEngine& engine;
    int trackIndex;

    juce::TextButton foldButton;
    juce::TextButton numberButton;
    juce::Label nameLabel;
    juce::TextButton soloButton{"S"};
    juce::TextButton armButton{"O"};
    
    juce::Label sourceLabel{"", "Audio From"};
    juce::ComboBox sourceBox;
    juce::Label destLabel{"", "Audio To"};
    juce::ComboBox destBox;
    
    juce::TextButton monitorInButton{"In"};
    juce::TextButton monitorAutoButton{"Auto"};
    juce::TextButton monitorOffButton{"Off"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};

} // namespace Nimbus::Timeline
