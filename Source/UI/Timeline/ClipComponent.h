#pragma once

#include <JuceHeader.h>
#include "DataModel/TimelineProject.h"
#include "Core/NimbusEngine.h"

namespace Nimbus::Timeline {

class ClipComponent : public juce::Component {
public:
    ClipComponent(AnyClipPtr clip, NimbusEngine& engine);
    ~ClipComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    AnyClipPtr getClip() const { return clipData; }

private:
    NimbusEngine& engine;
    AnyClipPtr clipData;
    juce::AudioThumbnailCache thumbnailCache{5};
    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};

} // namespace Nimbus::Timeline
