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
    ../Source/AudioEngine/AudioRecorder.cpp
    ../Source/AudioEngine/MidiRecorder.cpp
    ../Source/AudioEngine/Mixer.cpp
    ../Source/AudioEngine/AudioClipNode.cpp
    ../Source/AudioEngine/DiskStreaming/DiskStreamer.cpp
    ../Source/AudioEngine/Transport.cpp
    ../Source/DSP/GainNode.cpp
    ../Source/DSP/DelayLine.cpp
    ../Source/DataModel/AudioClip.cpp
    ../Source/DataModel/MidiClip.cpp
    ../Source/DataModel/TimelineProject.cpp
)

# Link Catch2 and JUCE modules
target_link_libraries(NimbusTests PRIVATE 
    Catch2::Catch2WithMain
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)
target_include_directories(NimbusTests PRIVATE 
    ../Source 
    ${CMAKE_CURRENT_BINARY_DIR}/../Nimbus_artefacts/JuceLibraryCode
)

if (MSVC)
    target_compile_options(NimbusTests PRIVATE /FS)
endif()
