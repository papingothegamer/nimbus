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
    static const inline juce::String RecordGlobal = "recordglobal_svg";
    static const inline juce::String Loop = "loop_svg";
    static const inline juce::String LoopOff = "loopoff_svg";
    static const inline juce::String Metronome = "metronometick_svg";
    static const inline juce::String Follow = "arrowrightthick_svg";

    // Track Controls
    static const inline juce::String Solo = "solo_svg";
    static const inline juce::String Mute = "mute_svg";
    static const inline juce::String RecordArm = "recordarmtrack_svg";
    static const inline juce::String VolumeSource = "volumesource_svg";
    static const inline juce::String VolumeOff = "volumeoff_svg";
    static const inline juce::String VolumeHigh = "volumehigh_svg";
    
    // Routing/Other
    static const inline juce::String Stereo = "link_svg";
    static const inline juce::String Routing = "tunevertical_svg";
    static const inline juce::String Settings = "settings_svg";
    
    // Transport Controls
    static const inline juce::String Undo = "undo_svg";
    static const inline juce::String Redo = "redo_svg";
    static const inline juce::String ZoomIn = "zoomplus_svg";
    static const inline juce::String ZoomOut = "zoomminus_svg";
    static const inline juce::String Rewind = "rewind_svg";
    static const inline juce::String FastForward = "fastforward_svg";
    static const inline juce::String JumpStart = "jumptostart_svg";
    
    // Track Controls
    static const inline juce::String Device = "pluginconfig_svg";
    static const inline juce::String Fold = "chevronup_svg";
    static const inline juce::String Unfold = "chevrondown_svg";
    static const inline juce::String LeftFold = "chevronleft_svg"; // Maybe not exist, fallback
    static const inline juce::String RightFold = "chevronright_svg";
    static const inline juce::String Delete = "deleteoutline_svg";
    static const inline juce::String AddPlugin = "pluginadd_svg";
    static const inline juce::String Alert = "alertoutline_svg";
    static const inline juce::String Flat = "musicaccidentalflat_svg";
    static const inline juce::String Sharp = "musicaccidentalsharp_svg";
    static const inline juce::String PianoOff = "pianooff_svg";
    static const inline juce::String PianoOn = "piano_svg";
    static const inline juce::String Save = "pluginsave_svg";
    static const inline juce::String Search = "search_svg";
    static const inline juce::String Tune = "tune_svg";
    
    // Toggles/Tabs
    static const inline juce::String Sidebar = "sidebarleft_svg";
    static const inline juce::String DetailView = "viewbottompanel_svg";
    static const inline juce::String Pencil = "penciloutline_svg";
    static const inline juce::String Piano = "piano_svg";
    static const inline juce::String Midi = "midi_svg";

    // Piano Roll Tools
    static const inline juce::String Pointer = "pointer_svg";
    static const inline juce::String Eraser = "eraser_svg";
    static const inline juce::String CursorMove = "cursormove_svg";
    static const inline juce::String Copy = "copy_svg";
    static const inline juce::String Paste = "paste_svg";
    
    // Volume variants
    static const inline juce::String VolumeLow = "volumelow_svg";
    static const inline juce::String VolumeMedium = "volumemedium_svg";
    static const inline juce::String VolumePlus = "volumeplus_svg";
    static const inline juce::String VolumeMinus = "volumeminus_svg";
    static const inline juce::String VolumeEqual = "volumeequal_svg";
};

} // namespace Nimbus::DesignSystem
