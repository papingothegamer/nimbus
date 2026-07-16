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

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // TimelineProject::Listener
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

    juce::TextButton foldButton;
    juce::TextButton selectButton{"Select"};
    juce::Label nameLabel;
    juce::TextButton muteButton{"Mute"};
    juce::TextButton soloButton{"Solo"};
    
    juce::Slider panSlider{juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft};
    juce::Label lPan{"", "L"};
    juce::Label rPan{"", "R"};

    juce::Slider gainSlider{juce::Slider::LinearHorizontal, juce::Slider::TextBoxLeft};
    juce::Label mGain{"", "-"};
    juce::Label pGain{"", "+"};
    
    juce::TextButton linkIcon{"Link"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};

} // namespace Nimbus::Timeline
