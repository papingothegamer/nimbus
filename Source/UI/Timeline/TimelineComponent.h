#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"

namespace Nimbus {

class SeekingBarComponent : public juce::Component {
public:
    SeekingBarComponent();
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    std::function<void(float)> onSeek;
};

class TimelineComponent : public juce::Component, 
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

    void paintOverChildren(juce::Graphics& g) override;
    
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
    NimbusEngine& engine;

    juce::OwnedArray<Timeline::TrackHeaderComponent> trackHeaders;
    juce::OwnedArray<Timeline::TrackLaneComponent> trackLanes;
    
    SeekingBarComponent seekingBar;

    double pixelsPerSecond = 50.0;
    double scrollOffsetX = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};

} // namespace Nimbus
