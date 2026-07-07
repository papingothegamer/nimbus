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
    GIT_TAG        8.0.14
)
FetchContent_MakeAvailable(juce)

# Nimbus Executable
juce_add_gui_app(Nimbus
    PRODUCT_NAME "Nimbus"
    VERSION 0.1.0
)

target_compile_definitions(Nimbus PRIVATE
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
    JUCE_ASIO=1
)

if (MSVC)
    target_compile_options(Nimbus PRIVATE /FS)
endif()

juce_generate_juce_header(Nimbus)

# Assets
juce_add_binary_data(Nimbus_Assets
    HEADER_NAME "BinaryData.h"
    NAMESPACE "BinaryData"
    SOURCES
        Assets/Fonts/ChakraPetch-Bold.ttf
        Assets/Fonts/ChakraPetch-Regular.ttf
        Assets/Fonts/Inter-Bold.ttf
        Assets/Fonts/Inter-Regular.ttf
        Assets/Fonts/MaterialSymbolsRounded.ttf
        Assets/Icons/Nimbus/alert-outline.svg
        Assets/Icons/Nimbus/arrow-right-thick.svg
        Assets/Icons/Nimbus/chevron-down.svg
        Assets/Icons/Nimbus/chevron-right.svg
        Assets/Icons/Nimbus/chevron-up.svg
        Assets/Icons/Nimbus/delete-outline.svg
        Assets/Icons/Nimbus/link.svg
        Assets/Icons/Nimbus/loop.svg
        Assets/Icons/Nimbus/metronome-tick.svg
        Assets/Icons/Nimbus/midi.svg
        Assets/Icons/Nimbus/music-accidental-flat.svg
        Assets/Icons/Nimbus/music-accidental-sharp.svg
        Assets/Icons/Nimbus/mute.svg
        Assets/Icons/Nimbus/pause.svg
        Assets/Icons/Nimbus/pencil-outline.svg
        Assets/Icons/Nimbus/piano-off.svg
        Assets/Icons/Nimbus/piano.svg
        Assets/Icons/Nimbus/play.svg
        Assets/Icons/Nimbus/plugin-add.svg
        Assets/Icons/Nimbus/plugin-config.svg
        Assets/Icons/Nimbus/plugin-save.svg
        Assets/Icons/Nimbus/record-arm-track.svg
        Assets/Icons/Nimbus/record-global.svg
        Assets/Icons/Nimbus/search.svg
        Assets/Icons/Nimbus/settings.svg
        Assets/Icons/Nimbus/sidebar-left.svg
        Assets/Icons/Nimbus/solo.svg
        Assets/Icons/Nimbus/stop.svg
        Assets/Icons/Nimbus/tune-vertical.svg
        Assets/Icons/Nimbus/tune.svg
        Assets/Icons/Nimbus/view-bottom-panel.svg
        Assets/Icons/Nimbus/volume-source.svg
        Assets/Icons/Nimbus/volume-off.svg
        Assets/Icons/Nimbus/waveform.svg
)
target_link_libraries(Nimbus PRIVATE Nimbus_Assets)

# Source files
target_sources(Nimbus PRIVATE
    Source/Main.cpp
    Source/UI/MainWindow.cpp
    Source/UI/MainWindow.h
    Source/UI/Timeline/TimelineComponent.cpp
    Source/UI/Timeline/TimelineComponent.h
    Source/UI/Timeline/TrackHeaderComponent.cpp
    Source/UI/Timeline/TrackHeaderComponent.h
    Source/UI/Timeline/GroupTrackHeaderComponent.cpp
    Source/UI/Timeline/GroupTrackHeaderComponent.h
    Source/UI/Timeline/TrackLaneComponent.cpp
    Source/UI/Timeline/TrackLaneComponent.h
    Source/UI/Timeline/ClipComponent.cpp
    Source/UI/Timeline/ClipComponent.h
    Source/UI/DesignSystem/Colors.h
    Source/UI/DesignSystem/Typography.h
    Source/UI/DesignSystem/NimbusLookAndFeel.cpp
    Source/UI/DesignSystem/NimbusLookAndFeel.h
    Source/UI/MainLayout/TopToolbarComponent.cpp
    Source/UI/MainLayout/TopToolbarComponent.h
    Source/UI/MainLayout/SideBrowserComponent.cpp
    Source/UI/MainLayout/SideBrowserComponent.h
    Source/UI/MainLayout/BottomMixerComponent.cpp
    Source/UI/MainLayout/BottomMixerComponent.h
    Source/UI/MainLayout/ChannelStripComponent.cpp
    Source/UI/MainLayout/ChannelStripComponent.h
    Source/UI/MainLayout/DetailViewComponent.cpp
    Source/UI/MainLayout/DetailViewComponent.h
    Source/UI/MainLayout/DeviceChainComponent.cpp
    Source/UI/MainLayout/DeviceChainComponent.h
    Source/UI/Mixer/GroupIndicatorComponent.cpp
    Source/UI/Mixer/GroupIndicatorComponent.h
    Source/UI/Settings/SettingsMenuComponent.cpp
    Source/UI/Settings/SettingsMenuComponent.h
    Source/UI/DetailView/PianoRollComponent.cpp
    Source/UI/DetailView/PianoRollComponent.h
    Source/UI/DetailView/AudioClipViewComponent.cpp
    Source/UI/DetailView/AudioClipViewComponent.h
    Source/UI/DetailView/ClipPropertiesComponent.cpp
    Source/UI/DetailView/ClipPropertiesComponent.h
    Source/UI/DetailView/NotesPanelComponent.cpp
    Source/UI/DetailView/NotesPanelComponent.h
    Source/UI/DetailView/PianoRollTimelineComponent.cpp
    Source/UI/DetailView/PianoRollTimelineComponent.h
    Source/UI/PluginWindow.cpp
    Source/UI/PluginWindow.h
    Source/Core/NimbusEngine.cpp
    Source/AudioEngine/AudioGraph.cpp
    Source/AudioEngine/AudioDeviceManagerWrapper.cpp
    Source/AudioEngine/Transport.cpp
    Source/AudioEngine/TestToneNode.cpp
    Source/AudioEngine/Track.cpp
    Source/AudioEngine/AudioRecorder.cpp
    Source/AudioEngine/MidiRecorder.cpp
    Source/AudioEngine/Mixer.cpp
    Source/AudioEngine/AudioClipNode.cpp
    Source/AudioEngine/DiskStreaming/DiskStreamer.cpp
    Source/DSP/GainNode.cpp
    Source/DataModel/AudioClip.cpp
    Source/DataModel/MidiClip.cpp
    Source/AudioEngine/MidiClipNode.cpp
    Source/DataModel/TimelineProject.cpp
    Source/PluginHost/PluginManager.cpp
    Source/AudioEngine/PluginNode.cpp
    Source/AudioEngine/PlaybackEngine.cpp
    Source/AudioEngine/TrackSourceNode.cpp
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
