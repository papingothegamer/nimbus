#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "ChannelStripComponent.h"

namespace Nimbus::MainLayout {

class BottomMixerComponent : public juce::Component, public TimelineProject::Listener {
public:
    BottomMixerComponent(NimbusEngine& engine);
    ~BottomMixerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // TimelineProject::Listener
    void trackAdded(int trackIndex, const TrackModel& track) override;
    void trackRemoved(int trackIndex) override;

private:
    NimbusEngine& engine;
    juce::Label titleLabel{"", "MIXER"};

    juce::OwnedArray<ChannelStripComponent> trackStrips;
    std::unique_ptr<ChannelStripComponent> masterStrip;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomMixerComponent)
};

} // namespace Nimbus::MainLayout
