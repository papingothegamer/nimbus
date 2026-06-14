#pragma once

#include <JuceHeader.h>
#include "DataModel/AudioClip.h"

namespace Nimbus::Timeline {

class ClipComponent : public juce::Component {
public:
    ClipComponent(std::shared_ptr<AudioClip> clip, juce::AudioFormatManager& formatManager);
    ~ClipComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::shared_ptr<AudioClip> clipData;
    juce::AudioThumbnailCache thumbnailCache{5};
    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};

} // namespace Nimbus::Timeline
