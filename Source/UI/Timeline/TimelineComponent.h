#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"

namespace Nimbus {

class SeekingBarComponent : public juce::Component {
public:
    SeekingBarComponent(NimbusEngine& engine, double& pps, double& scrollX);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    
    std::function<void(float)> onSeek;
    
    bool isDragging = false;
    float lastDragX = 0.0f;
    
private:
    NimbusEngine& engine;
    double& pixelsPerSecond;
    double& scrollOffsetX;
};

class TimelineComponent : public juce::Component, 
                        public juce::FileDragAndDropTarget,
                        private juce::Timer,
                        public TimelineProject::Listener {
public:
    TimelineComponent(NimbusEngine& engine);
    ~TimelineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // TimelineProject::Listener
    void trackAdded(int trackIndex, const TrackModel& track) override;
    void trackRemoved(int trackIndex) override;
    void trackFoldStateChanged(int trackIndex, bool isFolded) override;
    void tracksGrouped() override;

    void paintOverChildren(juce::Graphics& g) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    
    // juce::FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    void zoom(double factor);
    
    double getPixelsPerSecond() const { return pixelsPerSecond; }
    double getScrollOffsetX() const { return scrollOffsetX; }

private:
    NimbusEngine& engine;

    juce::OwnedArray<juce::Component> trackHeaders;
    juce::OwnedArray<Timeline::TrackLaneComponent> trackLanes;
    
    juce::Viewport viewport;
    juce::Component trackContainer;
    
    double pixelsPerSecond = 50.0;
    double scrollOffsetX = 0.0;
    
    SeekingBarComponent seekingBar{engine, pixelsPerSecond, scrollOffsetX};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};

} // namespace Nimbus
