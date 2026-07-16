#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../../Core/NimbusEngine.h"

namespace Nimbus::UI::Settings {

// A custom component for General Settings
class GeneralSettingsComponent : public juce::Component {
public:
    GeneralSettingsComponent(NimbusEngine& engine);
    ~GeneralSettingsComponent() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    NimbusEngine& engine;
    juce::ToggleButton followPlayheadToggle { "Follow Playhead" };
    juce::ToggleButton metronomeToggle { "Enable Metronome" };
    
    juce::Label defaultTrackColorLabel { {}, "Default Track Color:" };
    juce::ComboBox defaultTrackColorCombo;

    juce::Label projectSaveDirLabel { {}, "Project Save Directory:" };
    juce::TextEditor projectSaveDirEditor;
    juce::TextButton projectSaveDirBrowseBtn { "Browse..." };

    juce::Label pluginsDirLabel { {}, "Plugins Directory:" };
    juce::TextEditor pluginsDirEditor;
    juce::TextButton pluginsDirBrowseBtn { "Browse..." };
    juce::TextButton scanPluginsBtn { "Scan for Plugins" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneralSettingsComponent)
};

// A custom component for Display Settings
class DisplaySettingsComponent : public juce::Component {
public:
    DisplaySettingsComponent(NimbusEngine& engine);
    ~DisplaySettingsComponent() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::Label themeLabel { {}, "Theme:" };
    juce::ComboBox themeCombo;
    
    juce::Label zoomLabel { {}, "Viewport Zoom (Global Scale):" };
    juce::TextEditor zoomInput;
    
    juce::Label locationLabel { {}, "Sidebar Location:" };
    juce::ComboBox locationCombo;
    
    juce::ToggleButton animationsToggle { "Enable UI Animations" };

    NimbusEngine& engine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DisplaySettingsComponent)
};

class SettingsMenuComponent : public juce::Component, public juce::ListBoxModel {
public:
    SettingsMenuComponent(NimbusEngine& engine);
    ~SettingsMenuComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

private:
    NimbusEngine& engine;
    
    juce::ListBox categoryList;
    juce::StringArray categories;
    
    juce::Component* currentContent { nullptr };
    
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioDeviceSelectorComponent midiSetupComp;
    
    // Placeholder components for new Audacity-like tabs
    juce::Component playbackComp;
    juce::Component pluginsComp;
    juce::Component shortcutsComp;
    juce::Component advancedComp;
    
    GeneralSettingsComponent generalComp;
    DisplaySettingsComponent displayComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsMenuComponent)
};

} // namespace Nimbus::UI::Settings
