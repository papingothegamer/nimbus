#pragma once
#include <JuceHeader.h>

namespace Nimbus::DesignSystem {
    struct Iconography {
        // Transport
        static const inline juce::String Play = "play_svg";
        static const inline juce::String Pause = "pause_svg";
        static const inline juce::String Stop = "stop_svg";
        static const inline juce::String RecordGlobal = "record_global_svg";
        static const inline juce::String Loop = "loop_svg";
        static const inline juce::String JumpStart = "jump_to_start_svg";
        static const inline juce::String FastForward = "fast_forward_svg";
        static const inline juce::String Rewind = "rewind_svg";

        // Tools & Editing
        static const inline juce::String Undo = "undo_svg";
        static const inline juce::String Redo = "redo_svg";
        static const inline juce::String ZoomIn = "zoom_plus_svg";
        static const inline juce::String ZoomOut = "zoom_minus_svg";
        static const inline juce::String Cursor = "cursor_move_svg";
        static const inline juce::String Pointer = "pointer_svg";
        static const inline juce::String Pencil = "pencil_outline_svg";
        static const inline juce::String Eraser = "eraser_svg";
        static const inline juce::String Envelope = "arrow_up_down_svg"; 
        static const inline juce::String Cut = "content_cut_svg";
        static const inline juce::String Copy = "copy_svg";
        static const inline juce::String Paste = "paste_svg";

        // Track & Mixer
        static const inline juce::String Mute = "mute_svg";
        static const inline juce::String Unmute = "volume_source_svg";
        static const inline juce::String Solo = "solo_svg";
        static const inline juce::String RecordArm = "record_arm_track_svg";
        static const inline juce::String Stereo = "link_svg";
        
        // Devices & Plugins
        static const inline juce::String Device = "plugin_config_svg";
        static const inline juce::String AddPlugin = "plugin_add_svg";
        static const inline juce::String Delete = "delete_outline_svg";

        // UI Toggles & General
        static const inline juce::String PianoOn = "piano_svg";
        static const inline juce::String PianoOff = "piano_off_svg";
        static const inline juce::String Mixer = "view_bottom_panel_svg";
        static const inline juce::String Tune = "tune_svg";
        static const inline juce::String Settings = "settings_svg";
        static const inline juce::String Save = "plugin_save_svg";
        
        // Folders: Folded = points right, Unfolded = points down
        static const inline juce::String Fold = "chevron_down_svg";
        static const inline juce::String Unfold = "chevron_right_svg";
        
        static const inline juce::String Metronome = "metronome_tick_svg";
        static const inline juce::String Follow = "arrow_right_thick_svg";
    };
}