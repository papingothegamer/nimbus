# Tests CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(catch2)

# Create the test executable
add_executable(NimbusTests
    Main.cpp
    AudioEngine/AudioGraphTests.cpp
    AudioEngine/MixerTests.cpp
    AudioEngine/AudioRingBufferTests.cpp
    Concurrency/LockFreeQueueTests.cpp
    ../Source/AudioEngine/AudioGraph.cpp
    ../Source/AudioEngine/Track.cpp
    ../Source/AudioEngine/Mixer.cpp
    ../Source/AudioEngine/AudioClipNode.cpp
    ../Source/AudioEngine/DiskStreaming/DiskStreamer.cpp
    ../Source/AudioEngine/Transport.cpp
    ../Source/DSP/GainNode.cpp
    ../Source/DataModel/AudioClip.cpp
    ../Source/DataModel/TimelineProject.cpp
)

# Link Catch2 and JUCE modules
target_link_libraries(NimbusTests PRIVATE 
    Catch2::Catch2WithMain
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_core
    juce::juce_events
)
target_include_directories(NimbusTests PRIVATE ../Source)
