cmake_minimum_required(VERSION 3.24)

project(Nimbus VERSION 0.1.0)

# Set C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.1
)
FetchContent_MakeAvailable(juce)

# Nimbus Executable
juce_add_gui_app(Nimbus
    PRODUCT_NAME "Nimbus"
    VERSION 0.1.0
)

juce_generate_juce_header(Nimbus)

# Source files
target_sources(Nimbus PRIVATE
    Source/Main.cpp
    Source/UI/MainWindow.cpp
    Source/Core/NimbusEngine.cpp
    Source/AudioEngine/AudioGraph.cpp
    Source/AudioEngine/AudioDeviceManagerWrapper.cpp
    Source/AudioEngine/Transport.cpp
    Source/AudioEngine/TestToneNode.cpp
    Source/AudioEngine/Track.cpp
    Source/AudioEngine/Mixer.cpp
    Source/DSP/GainNode.cpp
)

# Link JUCE modules
target_link_libraries(Nimbus PRIVATE
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

# Include directories
target_include_directories(Nimbus PRIVATE Source)

# Tests
add_subdirectory(Tests)
