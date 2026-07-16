#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/Mixer/GroupIndicatorComponent.h"

namespace Nimbus::Timeline {

class TrackHeaderComponent : public juce::Component,
                           public juce::Label::Listener,
                           public juce::ComboBox::Listener,
                           public TimelineProject::Listener,
                           public juce::ChangeListener {
public:
    TrackHeaderComponent(NimbusEngine& engine, int trackIndex);
    ~TrackHeaderComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters();
    
    void setTrackIndex(int newIndex);
    
    void labelTextChanged(juce::Label* labelThatHasChanged) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void mouseDown(const juce::MouseEvent& event) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void trackMuteChanged(int track, bool isMuted) override;
    void trackSoloChanged(int track, bool isSoloed) override;
    void trackArmChanged(int track, bool isArmed) override;
    void trackNameChanged(int track, const juce::String& newName) override;
    void trackSelectionChanged() override;
    void trackFoldStateChanged(int track, bool isFolded) override;

    UI::GroupIndicatorComponent groupIndicator;

    void updateSelectionState();
    void updateInputSources();

private:
    NimbusEngine& engine;
    int trackIndex;
    float currentLevel = 0.0f;

    juce::TextButton powerToggle; 
    juce::Label nameLabel;
    
    juce::DrawableButton muteButton{"Mute", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton soloButton{"Solo", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton armButton{"Arm", juce::DrawableButton::ImageOnButtonBackground};
    
    juce::Slider panSlider{juce::Slider::LinearHorizontal, juce::Slider::NoTextBox};
    juce::Slider gainSlider{juce::Slider::LinearHorizontal, juce::Slider::NoTextBox};

    juce::DrawableButton foldButton{"Fold", juce::DrawableButton::ImageOnButtonBackground};
    juce::TextButton linkIcon{"Link"};

    void loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};

} // namespace Nimbus::Timeline