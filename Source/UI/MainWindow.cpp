#include "UI/MainWindow.h"
#include "UI/DesignSystem/Colors.h"

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
    mixerHeight = juce::jlimit(150, 600, dragStartH - e.getDistanceFromDragStartY());
    if (onHeightChanged) onHeightChanged();
}

MainWindow::MainContentComponent::MainContentComponent(NimbusEngine& e)
    : engine(e), topToolbar(e), sideBrowser(e), bottomMixer(e), detailView(e), timelineComponent(e) {
    juce::Logger::writeToLog("MainContentComponent constructed");
    
    topToolbar.onBrowserToggle = [this]() { toggleBrowser(); };
    topToolbar.onDetailToggle = [this]() { toggleDetailView(); };

    juce::Logger::writeToLog("Adding children to MainContentComponent");
    addAndMakeVisible(topToolbar);
    addAndMakeVisible(sideBrowser);
    addAndMakeVisible(bottomMixer);
    addAndMakeVisible(mixerResizerBar);
    addChildComponent(detailView); // Hidden by default
    addAndMakeVisible(timelineComponent);
    
    juce::Logger::writeToLog("MainContentComponent finished adding children");
    mixerResizerBar.onHeightChanged = [this]() { resized(); };
}

void MainWindow::MainContentComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
}

void MainWindow::MainContentComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Top Toolbar
    topToolbar.setBounds(bounds.removeFromTop(40));
    
    // Bottom Section
    int h = mixerResizerBar.mixerHeight;
    mixerResizerBar.setBounds(bounds.removeFromBottom(4));
    
    if (isDetailViewVisible) {
        auto bottomArea = bounds.removeFromBottom(h);
        detailView.setBounds(bottomArea.removeFromLeft(bottomArea.getWidth() / 2));
        bottomMixer.setBounds(bottomArea);
    } else {
        bottomMixer.setBounds(bounds.removeFromBottom(h));
    }
    
    // Side Browser
    if (isBrowserVisible) {
        sideBrowser.setBounds(bounds.removeFromRight(200));
    }
    
    // Timeline takes the rest
    timelineComponent.setBounds(bounds);
}

void MainWindow::MainContentComponent::toggleBrowser() {
    isBrowserVisible = !isBrowserVisible;
    sideBrowser.setVisible(isBrowserVisible);
    resized();
}

void MainWindow::MainContentComponent::toggleDetailView() {
    isDetailViewVisible = !isDetailViewVisible;
    detailView.setVisible(isDetailViewVisible);
    bottomMixer.setVisible(true); // Mixer is always visible
    resized();
}

void MainWindow::MainContentComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Insert Audio Track (CMD+T)");
        menu.addItem(2, "Insert MIDI Track (CMD+SHIFT+T)");
        menu.addSeparator();
        menu.addItem(3, "Delete Selected Track (Backspace)");
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                engine.addTrack(false);
            } else if (result == 2) {
                engine.addTrack(true);
            } else if (result == 3) {
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
                juce::Logger::writeToLog("Shortcut: Add MIDI Track");
                mainContent.getEngine().addTrack(true);
            } else {
                juce::Logger::writeToLog("Shortcut: Add Audio Track");
                mainContent.getEngine().addTrack(false);
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
