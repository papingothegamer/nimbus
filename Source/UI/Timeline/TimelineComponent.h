#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus {

class TimelineComponent : public juce::Component, private juce::Timer {
public:
    TimelineComponent(NimbusEngine& engine);
    ~TimelineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    NimbusEngine& engine;

    // Cache of AudioThumbnails per AudioClip
    // For now, we will just use a single AudioThumbnail for the test clip
    juce::AudioThumbnail testThumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};

} // namespace Nimbus
