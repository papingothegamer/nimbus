#pragma once

#include <JuceHeader.h>
#include "Typography.h"

namespace Nimbus::DesignSystem {

struct Iconography {
    // Media Controls
    static const inline juce::String Play = "play_svg";
    static const inline juce::String Pause = "pause_svg";
    static const inline juce::String Stop = "stop_svg";
    static const inline juce::String Record = "record_svg";
    static const inline juce::String RecordGlobal = "record_global_svg";
    static const inline juce::String Loop = "loop_svg";
    static const inline juce::String Metronome = "metronome_tick_svg";
    static const inline juce::String Follow = "arrow_right_thick_svg";

    // Track Controls
    static const inline juce::String Solo = "solo_svg";
    static const inline juce::String Mute = "mute_svg";
    static const inline juce::String RecordArm = "record_arm_track_svg";
    
    // Routing/Other
    static const inline juce::String Stereo = "link_svg";
    static const inline juce::String Routing = "tune_vertical_svg";
    static const inline juce::String Settings = "settings_svg";
    static const inline juce::String Device = "plugin_config_svg";
    static const inline juce::String Fold = "chevron_up_svg";
    static const inline juce::String Unfold = "chevron_down_svg";
    static const inline juce::String LeftFold = "chevron_left_svg"; // Maybe not exist, fallback
    static const inline juce::String RightFold = "chevron_right_svg";
    static const inline juce::String Delete = "delete_outline_svg";
    static const inline juce::String AddPlugin = "plugin_add_svg";
    static const inline juce::String Alert = "alert_outline_svg";
    static const inline juce::String Flat = "music_accidental_flat_svg";
    static const inline juce::String Sharp = "music_accidental_sharp_svg";
    static const inline juce::String PianoOff = "piano_off_svg";
    static const inline juce::String Save = "plugin_save_svg";
    static const inline juce::String Search = "search_svg";
    static const inline juce::String Tune = "tune_svg";
    
    // Toggles/Tabs
    static const inline juce::String Sidebar = "sidebar_left_svg";
    static const inline juce::String DetailView = "view_bottom_panel_svg";
    static const inline juce::String Pencil = "pencil_outline_svg";
    static const inline juce::String Piano = "piano_svg";
    static const inline juce::String Midi = "midi_svg";
};

} // namespace Nimbus::DesignSystem
