#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DetailView/PianoRollComponent.h"
#include "UI/DetailView/AudioClipViewComponent.h"
#include "UI/DetailView/ClipPropertiesComponent.h"
#include "UI/DetailView/PianoRollTimelineComponent.h"
#include "UI/MainLayout/DeviceChainComponent.h"

namespace Nimbus::MainLayout {

class DetailViewComponent : public juce::Component, public TimelineProject::Listener {
public:
    DetailViewComponent(NimbusEngine& engine);
    ~DetailViewComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // TimelineProject::Listener
    void trackAdded(int trackIndex, const TrackModel& track) override;
    void trackRemoved(int trackIndex) override;
    void trackSelectionChanged() override;
    void selectedClipChanged() override;

private:
    NimbusEngine& engine;
    juce::Label placeholderLabel{"Detail", "Select a track to view devices or clips."};
    
    DetailView::PianoRollComponent pianoRoll;
    juce::Viewport pianoRollViewport;
    DetailView::PianoRollTimelineComponent pianoRollTimeline;
    DetailView::ClipPropertiesComponent clipProperties;
    DetailView::AudioClipViewComponent audioClipView;
    DeviceChainComponent deviceChain;
    
    juce::TextButton clipTabButton{"Clip"};
    juce::TextButton deviceTabButton{"Device"};
    
    bool showDeviceView = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetailViewComponent)
};

} // namespace Nimbus::MainLayout
