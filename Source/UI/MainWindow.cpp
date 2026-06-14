#include "UI/MainWindow.h"

namespace Nimbus {

MainWindow::MainWindow(juce::String name, NimbusEngine& engineToUse)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons),
      engine(engineToUse) {
    
    // Set a dark theme background color from our specification: #111318
    setBackgroundColour(juce::Colour::fromString("#FF111318"));
    
    setUsingNativeTitleBar(true);
    // Set up the temporary volume slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(0.1); // Match TestToneNode level roughly
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    volumeSlider.onValueChange = [this]() {
        // Set the master volume smoothly
        if (auto* mixer = engine.getMixer()) {
            mixer->setMasterVolume(static_cast<float>(volumeSlider.getValue()));
        }
    };

    // Instead of directly setting a component, we create a generic one just to hold our slider for now.
    // In Phase 6, we'll build a proper juce::Component tree.
    auto* contentComponent = new juce::Component();
    setContentOwned(contentComponent, true);
    contentComponent->addAndMakeVisible(volumeSlider);

    setResizable(true, true);
    centreWithSize(1024, 768);
    setVisible(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed() {
    // This dictates that the app will quit when the main window is closed
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainWindow::resized() {
    DocumentWindow::resized();
    if (auto* content = getContentComponent()) {
        volumeSlider.setBounds(content->getLocalBounds().withSizeKeepingCentre(300, 50));
    }
}

} // namespace Nimbus
