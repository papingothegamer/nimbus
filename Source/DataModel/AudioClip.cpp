#include "AudioClip.h"

namespace Nimbus {

AudioClip::AudioClip(const juce::File& file, int startPos, int length, int offset)
    : sourceFile(file), startSample(startPos), lengthSamples(length), sourceOffsetSamples(offset) {
    warpMarkers.push_back(0.0); // Add default warp marker at the start
}

} // namespace Nimbus
