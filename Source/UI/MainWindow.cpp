#include "UI/MainWindow.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus {

MainWindow::MainContentComponent::MainContentComponent(NimbusEngine& e)
    : engine(e), topToolbar(e), sideBrowser(e), bottomMixer(e), detailView(e), timelineComponent(e) {
    
    topToolbar.onBrowserToggle = [this]() { toggleBrowser(); };
    topToolbar.onDetailToggle = [this]() { toggleDetailView(); };

    addAndMakeVisible(topToolbar);
    addAndMakeVisible(sideBrowser);
    addAndMakeVisible(bottomMixer);
    addChildComponent(detailView); // Hidden by default
    addAndMakeVisible(timelineComponent);
}

void MainWindow::MainContentComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
}

void MainWindow::MainContentComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Top Toolbar
    topToolbar.setBounds(bounds.removeFromTop(40));
    
    // Bottom Section
    if (isDetailViewVisible) {
        detailView.setBounds(bounds.removeFromBottom(250));
    } else {
        bottomMixer.setBounds(bounds.removeFromBottom(250));
    }
    
    // Side Browser
    if (isBrowserVisible) {
        sideBrowser.setBounds(bounds.removeFromLeft(200));
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
    bottomMixer.setVisible(!isDetailViewVisible);
    resized();
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
        pluginWindow = std::make_unique<PluginWindow>("Test Plugin", testPluginNode.get());
    }
}

MainWindow::~MainWindow() {
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

bool MainWindow::keyPressed(const juce::KeyPress& key) {
    if (key.getModifiers().isCommandDown()) {
        if (key.getKeyCode() == 'T') {
            if (key.getModifiers().isShiftDown()) {
                mainContent.getEngine().addTrack(true);
                return true;
            } else {
                mainContent.getEngine().addTrack(false);
                return true;
            }
        } else if (key.getKeyCode() == 'E') {
            juce::Logger::writeToLog("Shortcut: Split Clip");
            return true;
        } else if (key.getKeyCode() == 'D') {
            juce::Logger::writeToLog("Shortcut: Duplicate");
            return true;
        } else if (key.getKeyCode() == 'Z') {
            juce::Logger::writeToLog("Shortcut: Undo");
            return true;
        }
    } else if (key.getKeyCode() == juce::KeyPress::deleteKey || key.getKeyCode() == juce::KeyPress::backspaceKey) {
        juce::Logger::writeToLog("Shortcut: Delete Selection");
        return true;
    }
    return false;
}

} // namespace Nimbus
