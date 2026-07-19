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

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    double getTotalLength() const { return thumbnail.getTotalLength(); }

private:
    NimbusEngine& engine;
    std::shared_ptr<AudioClip> currentClip;
    juce::AudioThumbnail thumbnail;
    int draggedMarkerIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClipContent)
};

class AudioClipViewComponent : public juce::Component, private juce::Timer {
public:
    AudioClipViewComponent(NimbusEngine& engine);
    ~AudioClipViewComponent() override;

    void resized() override;
    void setAudioClip(std::shared_ptr<AudioClip> clip);
    
    void lookAndFeelChanged() override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
    void timerCallback() override;

    NimbusEngine& engine;
    std::shared_ptr<AudioClip> currentClip;
    juce::Viewport viewport;
    AudioClipContent content;
    double zoomFactor = 1.0;
    
    // Zoom and follow controls
    juce::TextButton zoomInButton{"zoomplus_svg"};
    juce::TextButton zoomOutButton{"zoomminus_svg"};
    juce::TextButton followButton{"arrowrightthick_svg"};
    bool autoScrollEnabled = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClipViewComponent)
};

} // namespace DetailView
} // namespace Nimbus
