#include <JuceHeader.h>
#include "UI/MainWindow.h"
#include "Core/NimbusEngine.h"

class NimbusApplication : public juce::JUCEApplication {
public:
    NimbusApplication() {
        auto logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("Nimbus_CrashLog.txt");
        fileLogger = std::make_unique<juce::FileLogger>(logFile, "Nimbus Crash Log");
        juce::Logger::setCurrentLogger(fileLogger.get());
        juce::Logger::writeToLog("NimbusApplication constructed");
    }

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& /*commandLine*/) override {
        juce::Logger::writeToLog("1. Requesting Permissions");
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [this](bool granted) {
            juce::Logger::writeToLog(granted ? "Microphone permission granted" : "Microphone permission denied");
            
            juce::Logger::writeToLog("2. Creating Engine");
            engine = std::make_unique<Nimbus::NimbusEngine>();
            
            juce::Logger::writeToLog("3. Initialising Engine");
            engine->initialise();
            
            juce::Logger::writeToLog("4. Creating MainWindow");
            mainWindow.reset(new Nimbus::MainWindow(getApplicationName(), *engine));
            
            juce::Logger::writeToLog("5. Initialise Complete");
        });
    }

    void shutdown() override {
        // Shutdown logic, release audio devices, etc.
        mainWindow = nullptr;
        engine = nullptr;
    }

    void systemRequestedQuit() override {
        // Called when the user clicks the close button
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {
        // Called if a second instance of the app is launched
    }

private:
    std::unique_ptr<juce::FileLogger> fileLogger;
    std::unique_ptr<Nimbus::NimbusEngine> engine;
    std::unique_ptr<Nimbus::MainWindow> mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(NimbusApplication)
