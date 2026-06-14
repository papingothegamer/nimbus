#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::Timeline {

class TrackHeaderComponent : public juce::Component {
public:
    TrackHeaderComponent(NimbusEngine& engine, int trackIndex);
    ~TrackHeaderComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NimbusEngine& engine;
    int trackIndex;

    juce::Label nameLabel;
    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};
    juce::Slider volSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};

} // namespace Nimbus::Timeline
