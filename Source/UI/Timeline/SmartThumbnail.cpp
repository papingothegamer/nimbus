#include "SmartThumbnail.h"

namespace Nimbus::Timeline {

SmartThumbnail::SmartThumbnail(juce::AudioFormatManager& formatManagerToUse,
                               juce::AudioThumbnailCache& cacheToUse,
                               juce::TimeSliceThread& backgroundThreadToUse)
    : thumbnail(512, formatManagerToUse, cacheToUse),
      backgroundThread(backgroundThreadToUse)
{
    thumbnail.addChangeListener(this);
}

SmartThumbnail::~SmartThumbnail() {
    stopTimer();
    thumbnail.removeChangeListener(this);
}

void SmartThumbnail::setSource(juce::InputSource* newSource) {
    thumbnail.setSource(newSource);
}

void SmartThumbnail::clear() {
    thumbnail.clear();
}

void SmartThumbnail::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &thumbnail) {
        needsRepaint.store(true, std::memory_order_release);
        if (!isTimerRunning()) {
            startTimerHz(30);
        }
    }
}

void SmartThumbnail::timerCallback() {
    if (needsRepaint.exchange(false, std::memory_order_acquire)) {
        sendChangeMessage(); // Notify UI to repaint
    }
    
    if (thumbnail.isFullyLoaded()) {
        stopTimer();
    }
}

} // namespace Nimbus::Timeline
