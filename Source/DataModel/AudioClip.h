#pragma once

#include <juce_core/juce_core.h>

namespace Nimbus {

/**
 * Represents a region of audio placed on a track's timeline.
 */
class AudioClip {
public:
    AudioClip(const juce::File& file, int startSample, int lengthSamples, int sourceOffsetSamples = 0);
    ~AudioClip() = default;

    const juce::File& getSourceFile() const { return sourceFile; }
    
    // Timeline position (start time in samples relative to the project start)
    int getStartSample() const { return startSample; }
    void setStartSample(int sample) { startSample = sample; }

    // How many samples long this clip is on the timeline
    int getLengthSamples() const { return lengthSamples; }
    void setLengthSamples(int length) { lengthSamples = length; }

    // Offset into the source file (for when the user trims the start of the clip)
    int getSourceOffsetSamples() const { return sourceOffsetSamples; }
    void setSourceOffsetSamples(int offset) { sourceOffsetSamples = offset; }

    // Timeline End Sample
    int getEndSample() const { return startSample + lengthSamples; }

private:
    juce::File sourceFile;
    int startSample = 0;
    int lengthSamples = 0;
    int sourceOffsetSamples = 0;
};

} // namespace Nimbus
