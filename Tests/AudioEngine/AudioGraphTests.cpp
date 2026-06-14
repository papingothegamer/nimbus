#include <catch2/catch_test_macros.hpp>
#include "AudioEngine/AudioGraph.h"

// A dummy node that adds 1.0 to every sample
class DummyNode : public Nimbus::IAudioNode {
public:
    void prepareToPlay(double /*sampleRate*/, int /*maximumExpectedSamplesPerBlock*/) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) override {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                buffer.addSample(ch, s, 1.0f);
            }
        }
    }
};

TEST_CASE("AudioGraph routing basics", "[audio]") {
    Nimbus::AudioGraph graph;
    
    // Add a node via the lock-free queue
    graph.addNode(std::make_unique<DummyNode>());

    juce::AudioBuffer<float> buffer(2, 64);
    buffer.clear();
    juce::MidiBuffer midi;

    // Process block will pop the queue and then process
    graph.processBlock(buffer, midi);

    REQUIRE(buffer.getSample(0, 0) == 1.0f);
    REQUIRE(buffer.getSample(1, 63) == 1.0f);
}
