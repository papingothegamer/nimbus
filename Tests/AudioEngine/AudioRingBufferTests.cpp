#include <catch2/catch_test_macros.hpp>
#include "AudioEngine/DiskStreaming/AudioRingBuffer.h"

TEST_CASE("AudioRingBuffer writes and reads correctly lock-free", "[audio_ring_buffer]") {
    Nimbus::AudioRingBuffer ringBuffer(2, 1024);

    juce::AudioBuffer<float> sourceBuffer(2, 512);
    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 512; ++i) {
            sourceBuffer.setSample(ch, i, (float)i);
        }
    }

    REQUIRE(ringBuffer.getFreeSpace() == 1023);
    REQUIRE(ringBuffer.getNumReady() == 0);

    // Write 512 samples
    int written = ringBuffer.write(sourceBuffer, 512);
    REQUIRE(written == 512);
    REQUIRE(ringBuffer.getNumReady() == 512);

    juce::AudioBuffer<float> destBuffer(2, 512);
    destBuffer.clear();

    // Read 512 samples
    int read = ringBuffer.read(destBuffer, 512);
    REQUIRE(read == 512);
    REQUIRE(ringBuffer.getNumReady() == 0);
    REQUIRE(ringBuffer.getFreeSpace() == 1023);

    // Verify data
    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 512; ++i) {
            REQUIRE(destBuffer.getSample(ch, i) == (float)i);
        }
    }

    // Write 800 samples (will wrap around)
    juce::AudioBuffer<float> largeSource(2, 800);
    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 800; ++i) {
            largeSource.setSample(ch, i, (float)i + 1000.0f);
        }
    }

    written = ringBuffer.write(largeSource, 800);
    REQUIRE(written == 800);
    REQUIRE(ringBuffer.getNumReady() == 800);

    juce::AudioBuffer<float> largeDest(2, 800);
    largeDest.clear();

    read = ringBuffer.read(largeDest, 800);
    REQUIRE(read == 800);

    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 800; ++i) {
            REQUIRE(largeDest.getSample(ch, i) == (float)i + 1000.0f);
        }
    }
}
