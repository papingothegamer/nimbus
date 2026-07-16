#pragma once
#include <JuceHeader.h>

namespace Nimbus::DesignSystem {
    struct Iconography {
        // Transport
        static const inline juce::String Play = "play_svg";
        static const inline juce::String Pause = "pause_svg";
        static const inline juce::String Stop = "stop_svg";
        static const inline juce::String RecordGlobal = "recordglobal_svg";
        static const inline juce::String Loop = "loop_svg";
        static const inline juce::String LoopOff = "loopoff_svg";
        static const inline juce::String JumpStart = "jumptostart_svg";
        static const inline juce::String FastForward = "fastforward_svg";
        static const inline juce::String FastForwardOutline = "fastforwardoutline_svg";
        static const inline juce::String Rewind = "rewind_svg";
        static const inline juce::String RewindOutline = "rewindoutline_svg";
        static const inline juce::String Metronome = "metronometick_svg";
        static const inline juce::String Follow = "arrowrightthick_svg";

        // Tools & Editing
        static const inline juce::String Undo = "undo_svg";
        static const inline juce::String Redo = "redo_svg";
        static const inline juce::String ZoomIn = "zoomplus_svg";
        static const inline juce::String ZoomOut = "zoomminus_svg";
        static const inline juce::String Cursor = "cursormove_svg";
        static const inline juce::String Pointer = "pointer_svg";
        static const inline juce::String Pencil = "penciloutline_svg";
        static const inline juce::String Eraser = "eraser_svg";
        static const inline juce::String Envelope = "arrowupdown_svg";
        static const inline juce::String Cut = "contentcut_svg";
        static const inline juce::String Trim = "trim_svg";
        static const inline juce::String Copy = "copy_svg";
        static const inline juce::String Paste = "paste_svg";

        // Track & Mixer
        static const inline juce::String Mute = "mute_svg";
        static const inline juce::String Unmute = "volumesource_svg";
        static const inline juce::String VolumeHigh = "volumehigh_svg";
        static const inline juce::String VolumeOff = "volumeoff_svg";
        static const inline juce::String Solo = "solo_svg";
        static const inline juce::String RecordArm = "recordarmtrack_svg";
        static const inline juce::String Stereo = "link_svg";
        
        // Devices & Plugins
        static const inline juce::String Device = "pluginconfig_svg";
        static const inline juce::String AddPlugin = "pluginadd_svg";
        static const inline juce::String Delete = "deleteoutline_svg";

        // UI Toggles & General
        static const inline juce::String PianoOn = "piano_svg";
        static const inline juce::String PianoOff = "pianooff_svg";
        static const inline juce::String Mixer = "viewbottompanel_svg";
        static const inline juce::String Tune = "tune_svg";
        static const inline juce::String Settings = "settings_svg";
        static const inline juce::String Save = "pluginsave_svg";
        
        // Folders: Folded = points right, Unfolded = points down
        static const inline juce::String Fold = "chevrondown_svg";
        static const inline juce::String Unfold = "chevronright_svg";
    };
}