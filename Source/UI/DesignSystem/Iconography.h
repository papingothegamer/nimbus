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
    static const inline juce::String LoopOff = "loop_off_svg";
    static const inline juce::String Metronome = "metronome_tick_svg";
    static const inline juce::String Follow = "arrow_right_thick_svg";

    // Track Controls
    static const inline juce::String Solo = "solo_svg";
    static const inline juce::String Mute = "mute_svg";
    static const inline juce::String RecordArm = "record_arm_track_svg";
    static const inline juce::String VolumeSource = "volume_source_svg";
    static const inline juce::String VolumeOff = "volume_off_svg";
    static const inline juce::String VolumeHigh = "volume_high_svg";
    
    // Routing/Other
    static const inline juce::String Stereo = "link_svg";
    static const inline juce::String Routing = "tune_vertical_svg";
    static const inline juce::String Settings = "settings_svg";
    
    // Transport Controls
    static const inline juce::String Undo = "undo_svg";
    static const inline juce::String Redo = "redo_svg";
    static const inline juce::String ZoomIn = "zoom_plus_svg";
    static const inline juce::String ZoomOut = "zoom_minus_svg";
    static const inline juce::String Rewind = "rewind_svg";
    static const inline juce::String FastForward = "fast_forward_svg";
    static const inline juce::String JumpStart = "jump_to_start_svg";
    
    // Track Controls
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
    static const inline juce::String PianoOn = "piano_svg";
    static const inline juce::String Save = "plugin_save_svg";
    static const inline juce::String Search = "search_svg";
    static const inline juce::String Tune = "tune_svg";
    
    // Toggles/Tabs
    static const inline juce::String Sidebar = "sidebar_left_svg";
    static const inline juce::String DetailView = "view_bottom_panel_svg";
    static const inline juce::String Pencil = "pencil_outline_svg";
    static const inline juce::String Piano = "piano_svg";
    static const inline juce::String Midi = "midi_svg";

    // Piano Roll Tools
    static const inline juce::String Pointer = "pointer_svg";
    static const inline juce::String Eraser = "eraser_svg";
    static const inline juce::String CursorMove = "cursor_move_svg";
    static const inline juce::String Copy = "copy_svg";
    static const inline juce::String Paste = "paste_svg";
    
    // Volume variants
    static const inline juce::String VolumeLow = "volume_low_svg";
    static const inline juce::String VolumeMedium = "volume_medium_svg";
    static const inline juce::String VolumePlus = "volume_plus_svg";
    static const inline juce::String VolumeMinus = "volume_minus_svg";
    static const inline juce::String VolumeEqual = "volume_equal_svg";
};

} // namespace Nimbus::DesignSystem
