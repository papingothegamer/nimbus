#pragma once

#include <JuceHeader.h>

namespace Nimbus::Timeline {

/**
 * A wrapper around juce::AudioThumbnail that automatically registers with a 
 * background TimeSliceThread and throttles repaint callbacks to the UI.
 */
class SmartThumbnail : public juce::ChangeBroadcaster,
                       private juce::ChangeListener,
                       private juce::Timer {
public:
    SmartThumbnail(juce::AudioFormatManager& formatManagerToUse,
                   juce::AudioThumbnailCache& cacheToUse,
                   juce::TimeSliceThread& backgroundThreadToUse);
    
    ~SmartThumbnail() override;

    void setSource(juce::InputSource* newSource);
    void clear();
    
    // Pass-through to juce::AudioThumbnail
    double getTotalLength() const noexcept { return thumbnail.getTotalLength(); }
    int getNumChannels() const noexcept { return thumbnail.getNumChannels(); }
    bool isFullyLoaded() const noexcept { return thumbnail.isFullyLoaded(); }
    
    void drawChannel(juce::Graphics& g, const juce::Rectangle<int>& area, 
                     double startTimeSeconds, double endTimeSeconds, 
                     int channelNum, float verticalZoomFactor) {
        thumbnail.drawChannel(g, area, startTimeSeconds, endTimeSeconds, channelNum, verticalZoomFactor);
    }
    
    void drawChannels(juce::Graphics& g, const juce::Rectangle<int>& area, 
                      double startTimeSeconds, double endTimeSeconds, 
                      float verticalZoomFactor) {
        thumbnail.drawChannels(g, area, startTimeSeconds, endTimeSeconds, verticalZoomFactor);
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

    juce::AudioThumbnail thumbnail;
    juce::TimeSliceThread& backgroundThread;
    
    std::atomic<bool> needsRepaint { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartThumbnail)
};

} // namespace Nimbus::Timeline
