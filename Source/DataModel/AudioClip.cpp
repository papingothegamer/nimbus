#include "AudioClip.h"

namespace Nimbus {

AudioClip::AudioClip(const juce::File& file, double startSampleValue, double lengthSamplesValue, double sourceOffsetSamplesValue)
    : Clip(Type::Audio, startSampleValue, lengthSamplesValue),
      sourceFile(file),
      gain(state, "gain", nullptr, 1.0f),
      pan(state, "pan", nullptr, 0.0f),
      reverse(state, "reverse", nullptr, false),
      fadeInSamples(state, "fadeInSamples", nullptr, 0),
      fadeOutSamples(state, "fadeOutSamples", nullptr, 0),
      fadeInCurve(state, "fadeInCurve", nullptr, 0),
      fadeOutCurve(state, "fadeOutCurve", nullptr, 0),
      pitchShift(state, "pitchShift", nullptr, 0.0f),
      speedMultiplier(state, "speedMultiplier", nullptr, 1.0),
      pitchShiftSemitones(state, "pitchShiftSemitones", nullptr, 0),
      matchDawTempo(state, "matchDawTempo", nullptr, false),
      originalBpm(state, "originalBpm", nullptr, 120.0),
      preservePitch(state, "preservePitch", nullptr, false),
      algorithmInt(state, "algorithm", nullptr, static_cast<int>(StretchAlgorithm::Melodic))
{
    this->sourceOffsetSamples = sourceOffsetSamplesValue;
}

std::shared_ptr<Clip> AudioClip::clone() const {
    auto c = std::make_shared<AudioClip>(sourceFile, startSample.get(), lengthSamples.get(), sourceOffsetSamples.get());
    c->state.copyPropertiesFrom(this->state, nullptr);
    return c;
}

} // namespace Nimbus
