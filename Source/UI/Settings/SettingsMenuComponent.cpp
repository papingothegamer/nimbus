#include "SettingsMenuComponent.h"
#include "../DesignSystem/NimbusLookAndFeel.h"
#include "../DesignSystem/Iconography.h"
#include "../DesignSystem/Colors.h"
#include "../DesignSystem/Typography.h"

namespace Nimbus::UI::Settings {

// ==============================================================================
// GeneralSettingsComponent
// ==============================================================================

GeneralSettingsComponent::GeneralSettingsComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(followPlayheadToggle);
    followPlayheadToggle.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    followPlayheadToggle.onClick = [this] {
        engine.setFollowPlayheadEnabled(followPlayheadToggle.getToggleState());
    };
    
    addAndMakeVisible(metronomeToggle);
    metronomeToggle.setToggleState(false, juce::dontSendNotification); // Dummy state for now
    metronomeToggle.onClick = [this] {
        // Toggle metronome logic
    };

    addAndMakeVisible(multiArmingToggle);
    multiArmingToggle.setToggleState(engine.getTimelineProject().isMultiArmingEnabled(), juce::dontSendNotification);
    multiArmingToggle.onClick = [this] {
        engine.getTimelineProject().setMultiArmingEnabled(multiArmingToggle.getToggleState());
    };
    
    addAndMakeVisible(defaultTrackColorLabel);
    defaultTrackColorLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    
    addAndMakeVisible(defaultTrackColorCombo);
    defaultTrackColorCombo.addItem("Blue", 1);
    defaultTrackColorCombo.addItem("Green", 2);
    defaultTrackColorCombo.addItem("Red", 3);
    defaultTrackColorCombo.addItem("Yellow", 4);
    defaultTrackColorCombo.setSelectedId(1, juce::dontSendNotification);

    addAndMakeVisible(projectSaveDirLabel);
    projectSaveDirLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    addAndMakeVisible(projectSaveDirEditor);
    projectSaveDirEditor.setText(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getFullPathName());
    addAndMakeVisible(projectSaveDirBrowseBtn);
    projectSaveDirBrowseBtn.onClick = [this] {
        // Mock browse logic
    };

    addAndMakeVisible(pluginsDirLabel);
    pluginsDirLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    addAndMakeVisible(pluginsDirEditor);
    pluginsDirEditor.setText("C:\\Program Files\\Common Files\\VST3");
    addAndMakeVisible(pluginsDirBrowseBtn);
    pluginsDirBrowseBtn.onClick = [this] {
        // Mock browse logic
    };

    addAndMakeVisible(scanPluginsBtn);
    scanPluginsBtn.onClick = [this] {
        if (!engine.getPluginManager().isScanning()) {
            engine.getPluginManager().startScanning();
        }
    };
}

GeneralSettingsComponent::~GeneralSettingsComponent() {}

void GeneralSettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
}

void GeneralSettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);
    
    followPlayheadToggle.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    metronomeToggle.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    multiArmingToggle.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    auto colorRect = bounds.removeFromTop(30);
    defaultTrackColorLabel.setBounds(colorRect.removeFromLeft(150));
    defaultTrackColorCombo.setBounds(colorRect.removeFromLeft(150));
    bounds.removeFromTop(10);
    
    auto projDirRect = bounds.removeFromTop(30);
    projectSaveDirLabel.setBounds(projDirRect.removeFromLeft(150));
    projectSaveDirBrowseBtn.setBounds(projDirRect.removeFromRight(80));
    projDirRect.removeFromRight(10);
    projectSaveDirEditor.setBounds(projDirRect);
    bounds.removeFromTop(10);
    
    auto plugDirRect = bounds.removeFromTop(30);
    pluginsDirLabel.setBounds(plugDirRect.removeFromLeft(150));
    pluginsDirBrowseBtn.setBounds(plugDirRect.removeFromRight(80));
    plugDirRect.removeFromRight(10);
    pluginsDirEditor.setBounds(plugDirRect);
    bounds.removeFromTop(10);
    
    scanPluginsBtn.setBounds(bounds.removeFromTop(30).withWidth(200));
}

// ==============================================================================
// DisplaySettingsComponent
// ==============================================================================

DisplaySettingsComponent::DisplaySettingsComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(themeLabel);
    themeLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    
    addAndMakeVisible(themeCombo);
    themeCombo.addItem("Dark Mode (Default)", 1);
    themeCombo.addItem("Light Mode", 2);
    themeCombo.setSelectedId(1, juce::dontSendNotification);
    
    addAndMakeVisible(zoomLabel);
    zoomLabel.setText("Global UI Scale", juce::dontSendNotification);
    zoomLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    
    addAndMakeVisible(zoomInput);
    zoomInput.setText(juce::String(juce::Desktop::getInstance().getGlobalScaleFactor() * 100.0f, 0) + "%");
    zoomInput.onReturnKey = [this] {
        float val = zoomInput.getText().retainCharacters("0123456789.").getFloatValue();
        if (val >= 50.0f && val <= 250.0f) {
            juce::Desktop::getInstance().setGlobalScaleFactor(val / 100.0f);
            zoomInput.setText(juce::String(val, 0) + "%");
        }
    };
    
    addAndMakeVisible(locationLabel);
    locationLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    
    addAndMakeVisible(locationCombo);
    locationCombo.addItem("Left", 1);
    locationCombo.addItem("Right", 2);
    locationCombo.setSelectedId(engine.getSidebarLocation() == 0 ? 1 : 2, juce::dontSendNotification);
    locationCombo.onChange = [this] {
        engine.setSidebarLocation(locationCombo.getSelectedId() == 1 ? 0 : 1);
    };
    
    addAndMakeVisible(animationsToggle);
    animationsToggle.setToggleState(true, juce::dontSendNotification);
}

DisplaySettingsComponent::~DisplaySettingsComponent() {}

void DisplaySettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
}

void DisplaySettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);
    
    auto themeRect = bounds.removeFromTop(30);
    themeLabel.setBounds(themeRect.removeFromLeft(150));
    themeCombo.setBounds(themeRect.removeFromLeft(150));
    
    bounds.removeFromTop(10);
    
    auto zoomRect = bounds.removeFromTop(30);
    zoomLabel.setBounds(zoomRect.removeFromLeft(200));
    zoomInput.setBounds(zoomRect.removeFromLeft(80));
    
    bounds.removeFromTop(10);
    
    auto locRect = bounds.removeFromTop(30);
    locationLabel.setBounds(locRect.removeFromLeft(150));
    locationCombo.setBounds(locRect.removeFromLeft(150));
    
    bounds.removeFromTop(10);
    animationsToggle.setBounds(bounds.removeFromTop(30));
}

// ==============================================================================
// SettingsMenuComponent
// ==============================================================================

SettingsMenuComponent::SettingsMenuComponent(NimbusEngine& engineToUse)
    : engine(engineToUse),
      categoryList("SettingsCategories", this),
      // Audio Setup: hide midi options
      audioSetupComp(engine.getAudioDeviceManager().getJuceAudioDeviceManager(),
                     0, 256, 0, 256, false, false, true, false),
      // MIDI Setup: hide audio options, show midi
      midiSetupComp(engine.getAudioDeviceManager().getJuceAudioDeviceManager(),
                     0, 0, 0, 0, true, true, false, false),
      generalComp(engine),
      displayComp(engine)
{
    setSize(700, 500);
    
    categories = { "Audio", "MIDI", "Playback / Recording", "Plugins", "Shortcuts", "General", "Display", "Advanced" };
    
    categoryList.setModel(this);
    categoryList.setRowHeight(36);
    categoryList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    addAndMakeVisible(categoryList);
    
    addChildComponent(audioSetupComp);
    addChildComponent(midiSetupComp);
    addChildComponent(playbackComp);
    addChildComponent(pluginsComp);
    addChildComponent(shortcutsComp);
    addChildComponent(generalComp);
    addChildComponent(displayComp);
    addChildComponent(advancedComp);
    
    // Select first row initially
    categoryList.selectRow(0);
}

SettingsMenuComponent::~SettingsMenuComponent() {
    categoryList.setModel(nullptr);
}

void SettingsMenuComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
    // Draw vertical divider
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(180, 0, 1, getHeight());
}

void SettingsMenuComponent::resized() {
    auto bounds = getLocalBounds();
    categoryList.setBounds(bounds.removeFromLeft(180));
    
    // Leave 1 px for divider and some padding
    bounds.removeFromLeft(1);
    
    if (currentContent != nullptr) {
        currentContent->setBounds(bounds.reduced(10));
    }
}

int SettingsMenuComponent::getNumRows() {
    return categories.size();
}

void SettingsMenuComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    if (rowNumber < 0 || rowNumber >= categories.size()) return;
    
    if (rowIsSelected) {
        g.fillAll(DesignSystem::Colors::AppBackground.withAlpha(0.2f));
    }
    
    g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f));
    g.drawText(categories[rowNumber], 16, 0, width - 32, height, juce::Justification::centredLeft, true);
}

void SettingsMenuComponent::selectedRowsChanged(int lastRowSelected) {
    if (currentContent != nullptr) {
        currentContent->setVisible(false);
    }
    
    switch (lastRowSelected) {
        case 0: currentContent = &audioSetupComp; break;
        case 1: currentContent = &midiSetupComp; break;
        case 2: currentContent = &playbackComp; break;
        case 3: currentContent = &pluginsComp; break;
        case 4: currentContent = &shortcutsComp; break;
        case 5: currentContent = &generalComp; break;
        case 6: currentContent = &displayComp; break;
        case 7: currentContent = &advancedComp; break;
        default: currentContent = nullptr; break;
    }
    
    if (currentContent != nullptr) {
        currentContent->setVisible(true);
        currentContent->setBounds(getLocalBounds().withTrimmedLeft(181).reduced(10));
    }
}

} // namespace Nimbus::UI::Settings
