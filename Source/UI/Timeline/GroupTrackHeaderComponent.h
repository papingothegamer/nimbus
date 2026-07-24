#pragma once
#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::Timeline {

class GroupTrackHeaderComponent : public juce::Component, public TimelineProject::Listener, public juce::DragAndDropTarget {
public:
    GroupTrackHeaderComponent(NimbusEngine& engine, int trackIndex);
    ~GroupTrackHeaderComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    
    // Drag and Drop Target
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void setTrackIndex(int newIndex);
    int getTrackIndex() const { return trackIndex; }

    void trackFoldStateChanged(int track, bool isFolded) override;
    void trackMuteChanged(int track, bool isMuted) override;
    void trackSoloChanged(int track, bool isSoloed) override;

private:
    NimbusEngine& engine;
    int trackIndex;

    bool isBeingDraggedOver = false;
    float dragInsertionY = -1.0f;

    juce::TextButton powerToggle;
    juce::Label trackNameLabel;
    
    juce::DrawableButton foldButton{"Fold", juce::DrawableButton::ImageRaw};
    juce::DrawableButton muteButton{"Mute", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton soloButton{"Solo", juce::DrawableButton::ImageOnButtonBackground};

    void loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName, float rotationDegrees = 0.0f);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GroupTrackHeaderComponent)
};

} // namespace Nimbus::Timeline