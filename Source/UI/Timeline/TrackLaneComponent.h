#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "ClipComponent.h"

namespace Nimbus {
class TimelineComponent;
}

namespace Nimbus::Timeline {

class TrackLaneComponent : public juce::Component, public TimelineProject::Listener {
public:
    TrackLaneComponent(NimbusEngine& engine, Nimbus::TimelineComponent& timeline, int trackIndex);
    ~TrackLaneComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setTrackIndex(int newIndex);
    
    void updateClips();
    void showContextMenu(const juce::MouseEvent& event);
    
    // TimelineProject::Listener
    void trackClipsChanged(int changedTrackIndex) override;

private:
    NimbusEngine& engine;
    Nimbus::TimelineComponent& timeline;
    int trackIndex;

    juce::OwnedArray<ClipComponent> clipComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackLaneComponent)
};

} // namespace Nimbus::Timeline
