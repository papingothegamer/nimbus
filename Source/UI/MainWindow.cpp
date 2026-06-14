#include "UI/MainWindow.h"

namespace Nimbus {

MainWindow::MainWindow(juce::String name)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons) {
    
    // Set a dark theme background color from our specification: #111318
    setBackgroundColour(juce::Colour::fromString("#FF111318"));
    
    setUsingNativeTitleBar(true);
    setContentOwned(new juce::Component(), true);

    centreWithSize(1024, 768);
    setVisible(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed() {
    // This dictates that the app will quit when the main window is closed
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace Nimbus
