#pragma once

#include <JuceHeader.h>
#include "DataModel/TimelineProject.h"
#include "Core/NimbusEngine.h"

namespace Nimbus::Timeline {

class ClipComponent : public juce::Component, public juce::ChangeListener {
public:
    ClipComponent(AnyClipPtr clip, NimbusEngine& engine);
    ~ClipComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    AnyClipPtr getClip() const { return clipData; }
    
    static juce::Colour getClipColor(int index);

private:
    NimbusEngine& engine;
    AnyClipPtr clipData;
    juce::AudioThumbnail thumbnail;
    
    bool isResizingLeft = false;
    bool isResizingRight = false;
    bool isDragging = false;
    bool isSelectingTime = false;
    int dragStartX = 0;
    double originalStartSamples = 0;
    double originalLengthSamples = 0;
    double originalSourceOffsetSamples = 0;

    void showPropertiesMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};

} // namespace Nimbus::Timeline
