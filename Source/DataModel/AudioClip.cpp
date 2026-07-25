#include "AudioClip.h"

namespace Nimbus {

AudioClip::AudioClip(const juce::File& file, double startSampleValue, double lengthSamplesValue, double sourceOffsetSamplesValue)
    : Clip(Type::Audio, startSampleValue, lengthSamplesValue),
      sourceFile(file),
      gain(state, "gain", nullptr, 1.0f),
      pan(state, "pan", nullptr, 0.0f),
      reverse(state, "reverse", nullptr, false),
      fadeInSamples(state, "fadeInSamples", nullptr, 192),
      fadeOutSamples(state, "fadeOutSamples", nullptr, 192),
      fadeInCurve(state, "fadeInCurve", nullptr, 1.0f),
      fadeOutCurve(state, "fadeOutCurve", nullptr, 1.0f),
      isCrossfadingIn(state, "isCrossfadingIn", nullptr, false),
      isCrossfadingOut(state, "isCrossfadingOut", nullptr, false),
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
    c->name = name.get();
    c->colorIndex = colorIndex.get();
    c->muted = muted.get();
    c->gain = gain.get();
    c->pan = pan.get();
    c->reverse = reverse.get();
    c->fadeInSamples = fadeInSamples.get();
    c->fadeOutSamples = fadeOutSamples.get();
    c->fadeInCurve = fadeInCurve.get();
    c->fadeOutCurve = fadeOutCurve.get();
    c->pitchShift = pitchShift.get();
    c->speedMultiplier = speedMultiplier.get();
    c->pitchShiftSemitones = pitchShiftSemitones.get();
    c->matchDawTempo = matchDawTempo.get();
    c->originalBpm = originalBpm.get();
    c->preservePitch = preservePitch.get();
    c->algorithmInt = algorithmInt.get();
    return c;
}

} // namespace Nimbus
