#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "ClipComponent.h"

namespace Nimbus {
class TimelineComponent;
}

namespace Nimbus::Timeline {

class TrackLaneComponent : public juce::Component, public TimelineProject::Listener, public juce::FileDragAndDropTarget {
public:
    TrackLaneComponent(NimbusEngine& engine, Nimbus::TimelineComponent& timeline, int trackIndex);
    ~TrackLaneComponent() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    
    void showContextMenuForClip(const juce::MouseEvent& event, AnyClipPtr clip);
    void showContextMenuForTimeSelection(const juce::MouseEvent& event);
    void showContextMenuForLane(const juce::MouseEvent& event);

    void setTrackIndex(int newIndex);
    int getNumClipComponents() const { return clipComponents.size(); }
    
    void updateClips();
    
    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    // TimelineProject::Listener
    void trackClipsChanged(int changedTrackIndex) override;
    void trackNameChanged(int changedTrackIndex, const juce::String& newName) override;
    void timeSelectionChanged() override;

private:
    NimbusEngine& engine;
    Nimbus::TimelineComponent& timeline;
    int trackIndex;

    juce::OwnedArray<ClipComponent> clipComponents;
    
    bool isDraggingSelection = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackLaneComponent)
};

} // namespace Nimbus::Timeline
