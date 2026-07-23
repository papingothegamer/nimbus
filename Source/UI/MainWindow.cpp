#include "UI/MainWindow.h"
#include "UI/DesignSystem/Colors.h"
#include "AudioEngine/ComputerMidiController.h"

namespace Nimbus {

MixerResizerBar::MixerResizerBar() {
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}
void MixerResizerBar::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::Divider);
}
void MixerResizerBar::mouseDown(const juce::MouseEvent& e) {
    dragStartH = mixerHeight;
}
void MixerResizerBar::mouseDrag(const juce::MouseEvent& e) {
    mixerHeight = juce::jlimit(310, 450, dragStartH - e.getDistanceFromDragStartY());
    if (onHeightChanged) onHeightChanged();
}

SidebarResizerBar::SidebarResizerBar(NimbusEngine& e) : engine(e) {
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}
void SidebarResizerBar::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::Divider);
}
void SidebarResizerBar::mouseDown(const juce::MouseEvent& e) {
    dragStartW = sidebarWidth;
}
void SidebarResizerBar::mouseDrag(const juce::MouseEvent& e) {
    if (engine.getSidebarLocation() == 0) { // Left
        sidebarWidth = juce::jlimit(150, 600, dragStartW + e.getDistanceFromDragStartX());
    } else { // Right
        sidebarWidth = juce::jlimit(150, 600, dragStartW - e.getDistanceFromDragStartX());
    }
    if (onWidthChanged) onWidthChanged();
}

MainWindow::MainContentComponent::MainContentComponent(NimbusEngine& e)
    : engine(e), topToolbar(e), sideBrowser(e), bottomPanel(e), timelineComponent(e), sidebarResizerBar(e) {
    juce::Logger::writeToLog("MainContentComponent constructed");
    
    topToolbar.onBrowserToggle = [this]() { toggleBrowser(); };
    topToolbar.onDetailToggle = [this]() { toggleDetailView(); };
    topToolbar.onZoomIn = [this]() { timelineComponent.zoom(1.1); };
    topToolbar.onZoomOut = [this]() { timelineComponent.zoom(0.9); };
    topToolbar.onZoomLevelRequested = [this](int zoom) { timelineComponent.setZoomLevel(zoom); };
    timelineComponent.onZoomLevelChanged = [this](int percentage) { topToolbar.setZoomLevel(percentage); };

    juce::Logger::writeToLog("Adding children to MainContentComponent");
    addAndMakeVisible(topToolbar);
    addAndMakeVisible(sideBrowser);
    addAndMakeVisible(bottomPanel);
    addAndMakeVisible(mixerResizerBar);
    addAndMakeVisible(sidebarResizerBar);
    addAndMakeVisible(timelineComponent);
    
    juce::Logger::writeToLog("MainContentComponent finished adding children");
    mixerResizerBar.onHeightChanged = [this]() { resized(); };
    sidebarResizerBar.onWidthChanged = [this]() { resized(); };
    engine.onSidebarLocationChanged = [this]() { resized(); };

    topToolbar.onBrowserToggle = [this]() { toggleBrowser(); };
    topToolbar.onDetailToggle = [this]() { toggleDetailView(); };
    topToolbar.onBottomPanelToggle = [this]() { toggleBottomPanel(); };
}

void MainWindow::MainContentComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
}

void MainWindow::MainContentComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Top Toolbar
    // Audacity-style workspace header: menu strip plus a distinct tools row.
    topToolbar.setBounds(bounds.removeFromTop(88));
    
    // Bottom Section
    if (isBottomPanelVisible) {
        int h = mixerResizerBar.mixerHeight;
        auto bottomArea = bounds.removeFromBottom(h);
        
        bottomPanel.setBounds(bottomArea);
        
        // Resizer bar sits directly above the bottom mixer section
        mixerResizerBar.setBounds(bounds.removeFromBottom(4));
        mixerResizerBar.setVisible(true);
    } else {
        bottomPanel.setVisible(false);
        mixerResizerBar.setVisible(false);
    }
    
    // Side Browser
    if (isBrowserVisible) {
        int w = sidebarResizerBar.sidebarWidth;
        if (engine.getSidebarLocation() == 0) {
            sideBrowser.setBounds(bounds.removeFromLeft(w));
            sidebarResizerBar.setBounds(bounds.removeFromLeft(8));
        } else {
            sideBrowser.setBounds(bounds.removeFromRight(w));
            sidebarResizerBar.setBounds(bounds.removeFromRight(8));
        }
    } else {
        sidebarResizerBar.setVisible(false);
    }
    
    // Timeline takes the rest
    timelineComponent.setBounds(bounds);
}

void MainWindow::MainContentComponent::toggleBrowser() {
    isBrowserVisible = !isBrowserVisible;
    sideBrowser.setVisible(isBrowserVisible);
    sidebarResizerBar.setVisible(isBrowserVisible);
    resized();
}

void MainWindow::MainContentComponent::toggleDetailView() {
    if (!isBottomPanelVisible) {
        // Auto-show bottom panel if detail view is turned on
        isBottomPanelVisible = true;
        bottomPanel.setVisible(true);
    }
    bottomPanel.showDeviceView();
    resized();
}

void MainWindow::MainContentComponent::toggleBottomPanel() {
    isBottomPanelVisible = !isBottomPanelVisible;
    if (isBottomPanelVisible) {
        bottomPanel.setVisible(true);
    }
    resized();
}

void MainWindow::MainContentComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Insert Mono Audio Track (Ctrl+T)");
        menu.addItem(2, "Insert Stereo Audio Track (Ctrl+Shift+T)");
        menu.addItem(3, "Insert MIDI Track (Ctrl+Alt+T)");
        menu.addSeparator();
        menu.addItem(4, "Delete Selected Track (Backspace)");
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                engine.addTrack(false, false); // isMidi = false, isStereo = false
            } else if (result == 2) {
                engine.addTrack(false, true);  // isMidi = false, isStereo = true
            } else if (result == 3) {
                engine.addTrack(true, true);   // isMidi = true, isStereo = true
            } else if (result == 4) {
                juce::Logger::writeToLog("Shortcut: Delete Selection");
            }
        });
    }
}

MainWindow::MainWindow(juce::String name, NimbusEngine& engineToUse)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons),
      mainContent(engineToUse) {
    
    // Apply Look and Feel
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);
    
    setBackgroundColour(DesignSystem::Colors::AppBackground);
    setUsingNativeTitleBar(true);
    
    setContentNonOwned(&mainContent, true);

    if (auto* ctrl = engineToUse.getComputerMidiController()) {
        addKeyListener(ctrl);
    }

    setResizable(true, true);
    centreWithSize(1280, 800);
    setVisible(true);

    if (auto testPluginNode = engineToUse.getTestPluginNode()) {
        pluginWindow = std::make_unique<PluginWindow>("Test Plugin", testPluginNode);
    }
}

MainWindow::~MainWindow() {
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

bool MainWindow::keyPressed(const juce::KeyPress& key) {
    if (key.getKeyCode() == 't' || key.getKeyCode() == 'T') {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            if (key.getModifiers().isShiftDown()) {
                juce::Logger::writeToLog("Shortcut: Add Stereo Audio Track");
                mainContent.getEngine().addTrack(false, true);
            } else if (key.getModifiers().isAltDown()) {
                juce::Logger::writeToLog("Shortcut: Add MIDI Track");
                mainContent.getEngine().addTrack(true, true);
            } else {
                juce::Logger::writeToLog("Shortcut: Add Mono Audio Track");
                mainContent.getEngine().addTrack(false, false);
            }
            return true;
        }
    } else if (key.getKeyCode() == 'e' || key.getKeyCode() == 'E') {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            juce::Logger::writeToLog("Shortcut: Split Clip");
            return true;
        }
    } else if (key.getKeyCode() == 'd' || key.getKeyCode() == 'D') {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            juce::Logger::writeToLog("Shortcut: Duplicate");
            return true;
        }
    } else if (key.getKeyCode() == 'g' || key.getKeyCode() == 'G') {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            if (key.getModifiers().isShiftDown()) {
                juce::Logger::writeToLog("Shortcut: Ungroup Tracks");
                // Find selected group track
                auto selected = mainContent.getEngine().getTimelineProject().getSelectedTracks();
                if (!selected.isEmpty()) {
                    mainContent.getEngine().getTimelineProject().ungroupTracks(selected[0]);
                }
            } else {
                juce::Logger::writeToLog("Shortcut: Group Tracks");
                mainContent.getEngine().getTimelineProject().groupTracks(mainContent.getEngine().getTimelineProject().getSelectedTracks());
            }
            return true;
        }
    } else if (key.getKeyCode() == juce::KeyPress::backspaceKey || key.getKeyCode() == juce::KeyPress::deleteKey) {
        juce::Logger::writeToLog("Shortcut: Delete");
        auto selected = mainContent.getEngine().getTimelineProject().getSelectedTracks();
        for (int i = selected.size() - 1; i >= 0; --i) {
            mainContent.getEngine().getTimelineProject().removeTrack(selected[i]);
        }
        return true;
    } else if (key.getKeyCode() == 'z' || key.getKeyCode() == 'Z') {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            juce::Logger::writeToLog("Shortcut: Undo");
            return true;
        }
    } else if (key.getKeyCode() == juce::KeyPress::spaceKey) {
        if (mainContent.getEngine().getTransport().isRecording()) {
            mainContent.getEngine().stopRecording();
        } else if (mainContent.getEngine().getTransport().isPlaying()) {
            mainContent.getEngine().getTransport().stop();
        } else {
            mainContent.getEngine().getTransport().play();
        }
        return true;
    }
    return false;
}

} // namespace Nimbus
