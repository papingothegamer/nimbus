# Timeline and Clip Editing Enhancements

## Overview
This document summarizes the features, fixes, and workflow enhancements added to the Nimbus DAW Timeline and Clip architecture.

## 1. Clip Styling and Property Alignment
- **Dynamic Waveforms and MIDI Overlays:** Clips now consistently render audio waveforms and MIDI notes relative to their signal amplitude and vertical pitch positions respectively.
- **Persistent Visibility:** Changes in clip length or velocity edits no longer cause visual artifacting or disappearing waveforms.
- **Aesthetics & Readability:**
  - Integrated `ellipsis-horizontal.svg` / `ellipsis-vertical.svg` for clean clip properties menus.
  - Implemented dynamic text color contrasting (black/white) based on the background color's perceived brightness to ensure clip names and properties are always legible.
  - Clip renaming now cascades gracefully from Track Header renames.

## 2. Contextual Menus
Split the massive unified context menu into three targeted, workflow-specific menus, mimicking industry-standard DAWs like Ableton Live:
- **Clip Context Menu:** Click a specific clip to access Cut, Copy, Duplicate, Delete, and Rename for that specific clip.
- **Time Selection Menu:** Right-click a highlighted time selection to perform edit actions bounded strictly within the selected time range.
- **Empty Lane Menu:** Right-click an empty area in a track lane to access global Paste and Insert Empty MIDI Clip commands.

## 3. Workflow Improvements
- **Global Drag-to-Highlight:** You can now click and drag starting from the empty global grid space (below all tracks) to highlight time regions and dynamically select multiple tracks based on mouse intersection, replicating Ableton's fluid arrangement workflow.
- **Timeline Zoom Fixes:** Unified timeline zooming depth controls to guarantee zoom states remain consistent across the application, with a strict 10% - 200% clamped range for optimal navigation.
- **Track & Clip Management:** Added native support for full Track Duplication (including clips, automation, effects, and settings) and precise Clip Copy/Paste functionalities across the Timeline.
