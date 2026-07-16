#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/Mixer/GroupIndicatorComponent.h"
#include "UI/Mixer/MeteredFader.h"
#include "UI/Mixer/MidiActivityMeter.h"

namespace Nimbus::MainLayout {

class ChannelStripComponent : public juce::Component, public TimelineProject::Listener, public juce::Label::Listener {
public:
    ChannelStripComponent(NimbusEngine& engine, const juce::String& trackName, bool isStereo, bool isMaster = false);
    ~ChannelStripComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void setLevelProvider(std::function<float()> provider);
    void updateMeters();

    void trackVolumeChanged(int track, float volume) override;
    void trackPanChanged(int track, float panValue) override;
    void trackMuteChanged(int track, bool isMuted) override;
    void trackSoloChanged(int track, bool isSoloed) override;
    void trackArmChanged(int track, bool isArmed) override;
    void trackNameChanged(int track, const juce::String& newName) override;
    void trackSelectionChanged() override;
    void trackInputChannelChanged(int track, int inputChannel) override;

    void setTrackIndex(int index);
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    std::function<void()> onSelected;

private:
    NimbusEngine& engine;
    int trackIndex = -1;
    juce::String channelName;
    bool stereo;
    bool master;
    bool selected = false;

    juce::Label nameLabel;
    juce::ComboBox inputComboBox;
    juce::ComboBox routingComboBox;
    
    juce::Slider pan;
    UI::MeteredFader meteredFader;
    UI::MidiActivityMeter midiMeter;
    
    juce::DrawableButton muteButton{"M", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton soloButton{"S", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton armButton{"R", juce::DrawableButton::ImageOnButtonBackground};

    // FIX: Removed the incorrect 'Timeline::' namespace prefix
    UI::GroupIndicatorComponent groupIndicator;

    std::function<float()> levelProvider;
    float currentLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripComponent)
};

} // namespace Nimbus::MainLayout