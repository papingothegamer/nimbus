#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "AudioEngine/NimbusEngine.h"
#include "AudioEngine/PlaybackEngine.h"
#include "AudioEngine/DiskStreaming/DiskStreamer.h"
#include "Models/AudioClip.h"
#include <iostream>

using namespace Nimbus;

int main() {
    juce::ScopedJuceInitialiser_GUI juceInit;
    
    NimbusEngine engine;
    
    // Create a 1-second dummy wav file
    juce::File testFile = juce::File::getCurrentWorkingDirectory().getChildFile("test.wav");
    if (testFile.existsAsFile()) testFile.deleteFile();
    
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(
        new juce::FileOutputStream(testFile), 48000.0, 2, 16, {}, 0));
        
    if (writer) {
        juce::AudioBuffer<float> testBuffer(2, 48000);
        for (int ch = 0; ch < 2; ++ch) {
            auto* out = testBuffer.getWritePointer(ch);
            for (int i = 0; i < 48000; ++i) out[i] = 0.5f; // Constant DC offset for easy detection
        }
        writer->writeFromAudioSampleBuffer(testBuffer, 0, 48000);
        writer.reset();
    }
    
    // Add track and clip
    engine.addTrack(false, true); // stereo audio track
    auto clip = std::make_shared<AudioClip>(testFile, 0, 48000);
    engine.getTimelineProject().addClipToTrack(0, clip);
    
    // Wait a bit for async stuff
    juce::Thread::sleep(500);
    
    // Run processBlock directly
    juce::AudioBuffer<float> outputBuffer(2, 512);
    engine.getTransport().setPlaying(true);
    
    for (int i = 0; i < 10; ++i) {
        outputBuffer.clear();
        engine.getPlaybackEngine().getMixer().processBlock(outputBuffer);
        
        float maxVal = 0.0f;
        for (int j = 0; j < 512; ++j) {
            maxVal = std::max(maxVal, std::abs(outputBuffer.getSample(0, j)));
        }
        std::cout << "Block " << i << " max amp: " << maxVal << std::endl;
        
        engine.getTransport().advancePosition(512);
        juce::Thread::sleep(50);
    }
    
    return 0;
}
