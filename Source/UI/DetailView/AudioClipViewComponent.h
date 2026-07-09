#pragma once

#include <JuceHeader.h>
#include <memory>
#include "DataModel/AudioClip.h"

namespace Nimbus {
class NimbusEngine;
namespace DetailView {

class AudioClipContent : public juce::Component, private juce::ChangeListener {
public:
    AudioClipContent(NimbusEngine& e);
    ~AudioClipContent() override;

    void paint(juce::Graphics& g) override;
    void setAudioClip(std::shared_ptr<AudioClip> clip);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    double getTotalLength() const { return thumbnail.getTotalLength(); }

private:
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    NimbusEngine& engine;
    std::shared_ptr<AudioClip> currentClip;
    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClipContent)
};

class AudioClipViewComponent : public juce::Component {
public:
    AudioClipViewComponent(NimbusEngine& engine);
    ~AudioClipViewComponent() override;

    void resized() override;
    void setAudioClip(std::shared_ptr<AudioClip> clip);

private:
    juce::Viewport viewport;
    AudioClipContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClipViewComponent)
};

} // namespace DetailView
} // namespace Nimbus
