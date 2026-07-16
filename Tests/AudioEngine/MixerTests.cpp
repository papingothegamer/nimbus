#include <catch2/catch_test_macros.hpp>
#include "AudioEngine/Mixer.h"
#include "AudioEngine/Track.h"

class DCNode : public Nimbus::IAudioNode {
public:
    explicit DCNode(float value) : dcValue(value) {}
    void prepareToPlay(double /*sampleRate*/, int /*maximumExpectedSamplesPerBlock*/) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) override {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                buffer.addSample(ch, s, dcValue);
            }
        }
    }
private:
    float dcValue;
};

TEST_CASE("Mixer sums tracks correctly", "[mixer]") {
    Nimbus::Mixer mixer;
    mixer.prepareToPlay(44100.0, 64);
    
    // Create track 1 (+1.0)
    auto track1 = std::make_unique<Nimbus::Track>(Nimbus::TrackID(), true);
    track1->setSourceNode(std::make_unique<DCNode>(1.0f));
    
    // Create track 2 (+0.5)
    auto track2 = std::make_unique<Nimbus::Track>(Nimbus::TrackID(), true);
    track2->setSourceNode(std::make_unique<DCNode>(0.5f));
    
    mixer.addTrack(std::move(track1));
    mixer.addTrack(std::move(track2));

    juce::AudioBuffer<float> buffer(2, 64);
    buffer.clear();
    juce::MidiBuffer midi;

    mixer.processBlock(buffer, midi);

    // The tracks apply center pan (-3dB = 0.707), and then the mixer master fader
    // applies center pan AGAIN (-3dB = 0.707).
    // So the total signal is (1.0 + 0.5) * 0.707 * 0.707 = 1.5 * 0.5 = 0.75.
    float expected = 1.5f * 0.5f;
    
    REQUIRE(std::abs(buffer.getSample(0, 0) - expected) < 0.001f);
    REQUIRE(std::abs(buffer.getSample(1, 0) - expected) < 0.001f);
}
