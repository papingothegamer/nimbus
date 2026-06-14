#include "UI/MainWindow.h"

namespace Nimbus {

MainWindow::MainContentComponent::MainContentComponent(NimbusEngine& engine)
    : timelineComponent(engine) {
    addAndMakeVisible(timelineComponent);
}

void MainWindow::MainContentComponent::resized() {
    timelineComponent.setBounds(getLocalBounds());
}

MainWindow::MainWindow(juce::String name, NimbusEngine& engineToUse)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons),
      mainContent(engineToUse) {
    
    // Set a dark theme background color from our specification: #111318
    setBackgroundColour(juce::Colour::fromString("#FF111318"));
    setUsingNativeTitleBar(true);
    
    setContentNonOwned(&mainContent, true);

    setResizable(true, true);
    centreWithSize(1024, 768);
    setVisible(true);

    if (auto testPluginNode = engineToUse.getTestPluginNode()) {
        pluginWindow = std::make_unique<PluginWindow>("Test Plugin", testPluginNode.get());
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed() {
    // This dictates that the app will quit when the main window is closed
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace Nimbus
