#include <JuceHeader.h>
#include "UI/MainWindow.h"
#include "Core/NimbusEngine.h"

class NimbusApplication : public juce::JUCEApplication {
public:
    NimbusApplication() = default;

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& /*commandLine*/) override {
        // This is where you initialize the app and spawn the main window
        engine.initialise();
        mainWindow.reset(new Nimbus::MainWindow(getApplicationName(), engine));
    }

    void shutdown() override {
        // Shutdown logic, release audio devices, etc.
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override {
        // Called when the user clicks the close button
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {
        // Called if a second instance of the app is launched
    }

private:
    Nimbus::NimbusEngine engine;
    std::unique_ptr<Nimbus::MainWindow> mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(NimbusApplication)
