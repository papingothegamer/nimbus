#pragma once

#include <JuceHeader.h>
#include "DataModel/TimelineProject.h"
#include "Core/NimbusEngine.h"
#include "SmartThumbnail.h"

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

    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    NimbusEngine& engine;
    AnyClipPtr clipData;
    SmartThumbnail thumbnail;
    
    bool isResizingLeft = false;
    bool isResizingRight = false;
    bool isDragging = false;
    bool isSelectingTime = false;
    
    // Fade UI state
    bool isHoveringEdges = false;
    bool isDraggingFadeIn = false;
    bool isDraggingFadeOut = false;
    bool isDraggingFadeInCurve = false;
    bool isDraggingFadeOutCurve = false;
    int dragStartFadeInSamples = 0;
    int dragStartFadeOutSamples = 0;
    float dragStartFadeInCurve = 1.0f;
    float dragStartFadeOutCurve = 1.0f;

    int dragStartX = 0;
    int dragStartY = 0;
    double originalStartSamples = 0;
    double originalLengthSamples = 0;
    double originalSourceOffsetSamples = 0;

    void showPropertiesMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};

} // namespace Nimbus::Timeline
