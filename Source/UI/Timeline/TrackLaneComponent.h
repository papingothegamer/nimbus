#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "ClipComponent.h"

namespace Nimbus::Timeline {

class TrackLaneComponent : public juce::Component {
public:
    TrackLaneComponent(NimbusEngine& engine, int trackIndex);
    ~TrackLaneComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateClips();
    void setTrackIndex(int newIndex);

private:
    NimbusEngine& engine;
    int trackIndex;

    juce::OwnedArray<ClipComponent> clipComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackLaneComponent)
};

} // namespace Nimbus::Timeline
