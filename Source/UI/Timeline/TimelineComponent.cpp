#include "TimelineComponent.h"

namespace Nimbus {

TimelineComponent::TimelineComponent(NimbusEngine& e)
    : engine(e), 
      testThumbnail(512, e.getFormatManager(), e.getThumbnailCache()) 
{
    // Load the test file
    juce::File testFile(R"(C:\Users\Laptop\Desktop\X26\EAGLP\export_1726306721135.wav)");
    if (testFile.existsAsFile()) {
        testThumbnail.setSource(new juce::FileInputSource(testFile));
    }

    startTimerHz(60); // Repaint at 60Hz to keep playhead smooth
}

TimelineComponent::~TimelineComponent() {
    stopTimer();
}

void TimelineComponent::paint(juce::Graphics& g) {
    // 1. Fill background with Panel Background color
    g.fillAll(juce::Colour::fromString("#FF181A20"));

    auto bounds = getLocalBounds();

    // Draw the audio clip thumbnail
    auto trackArea = bounds.withHeight(100).withY(40); // Hardcode some space for the single test track

    // Background for the track
    g.setColour(juce::Colour::fromString("#FF22252D"));
    g.fillRect(trackArea);

    g.setColour(juce::Colour::fromString("#FF4A4E5A"));
    g.drawRect(trackArea);

    if (testThumbnail.getTotalLength() > 0.0) {
        // Draw the waveform in a blue tint
        g.setColour(juce::Colour::fromString("#FF4A90E2").withAlpha(0.6f));
        // We map the full length of the audio file to the width of the component for this simple test
        testThumbnail.drawChannels(g, trackArea, 0.0, testThumbnail.getTotalLength(), 1.0f);
    }

    // 3. Draw playhead
    double sr = engine.getTransport().getSampleRate();
    if (sr <= 0.0) sr = 44100.0;
    double currentPositionSeconds = engine.getTransport().getCurrentPosition() / sr;
    
    // Calculate X position. If the timeline represents 60 seconds across the screen:
    double timelineLengthSeconds = 60.0;
    if (testThumbnail.getTotalLength() > 0.0) {
        timelineLengthSeconds = testThumbnail.getTotalLength();
    }
    
    float playheadX = static_cast<float>((currentPositionSeconds / timelineLengthSeconds) * bounds.getWidth());

    g.setColour(juce::Colours::white);
    g.drawLine(playheadX, 0, playheadX, (float)bounds.getHeight(), 1.0f);
}

void TimelineComponent::resized() {
    // Layout logic will go here
}

void TimelineComponent::timerCallback() {
    // Repaint to update playhead position
    if (engine.getTransport().isPlaying()) {
        repaint();
    }
}

} // namespace Nimbus
