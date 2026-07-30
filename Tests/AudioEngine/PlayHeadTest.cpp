#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../Source/AudioEngine/PlayHead.h"

using namespace Nimbus;

TEST_CASE("PlayHead tracks uncompensated and audible time correctly", "[AudioEngine][PlayHead]") {
    PlayHead playHead;
    
    // Set typical parameters
    double sampleRate = 48000.0;
    int blockSize = 512;
    int latencySamples = 1024; // e.g. 512 output latency + 512 block size
    
    playHead.setLatencySamples(latencySamples);
    playHead.setPositionSeconds(0.0);

    SECTION("Initial jump freezes audible time for latency duration") {
        REQUIRE(playHead.getLivePositionSeconds() == 0.0);
        
        // Advance exactly 512 samples. 
        // This is less than latencySamples (1024), so audible time should remain frozen at 0.0.
        playHead.advanceAudioTime(blockSize, sampleRate, false, 0.0, 0.0);
        
        REQUIRE(playHead.getLivePositionSeconds() == 0.0);
        
        // Advance another 512 samples. Total advanced = 1024 (equals latencySamples).
        // Audible time should still be frozen at 0.0, or about to unfreeze.
        playHead.advanceAudioTime(blockSize, sampleRate, false, 0.0, 0.0);
        
        REQUIRE(playHead.getLivePositionSeconds() == 0.0);
        
        // Advance one more block. Now we exceed the countdown.
        playHead.advanceAudioTime(blockSize, sampleRate, false, 0.0, 0.0);
        
        // Total samples processed = 1536. Uncompensated time = 1536 / 48000 = 0.032.
        // Audible time = Uncompensated Time (0.032) - Latency Offset (1024 / 48000 = 0.021333) = 0.010666.
        // Which is exactly 512 samples / 48000.
        double expectedAudible = static_cast<double>(blockSize) / sampleRate;
        REQUIRE_THAT(playHead.getLivePositionSeconds(), Catch::Matchers::WithinRel(expectedAudible, 1e-5));
    }
    
    SECTION("Subsequent jumps freeze audible time at the jump position") {
        // Run normally for a while
        playHead.advanceAudioTime(48000, sampleRate, false, 0.0, 0.0); // 1 sec
        
        // Jump to 10.0 seconds
        playHead.setPositionSeconds(10.0);
        REQUIRE(playHead.getLivePositionSeconds() == 10.0);
        
        // Advance less than latency. Audible time remains 10.0.
        playHead.advanceAudioTime(blockSize, sampleRate, false, 0.0, 0.0);
        REQUIRE(playHead.getLivePositionSeconds() == 10.0);
        
        // Clear latency countdown
        playHead.advanceAudioTime(latencySamples, sampleRate, false, 0.0, 0.0);
        
        // Uncompensated time is now 10.0 + (512 + 1024) / 48000 = 10.032.
        // Audible time = 10.032 - 1024/48000 = 10.010666. (which is 10.0 + 512/48000)
        double expectedAudible = 10.0 + static_cast<double>(blockSize) / sampleRate;
        REQUIRE_THAT(playHead.getLivePositionSeconds(), Catch::Matchers::WithinRel(expectedAudible, 1e-5));
    }
}
