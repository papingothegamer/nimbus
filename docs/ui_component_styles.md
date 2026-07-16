# Nimbus Component Design Styles & Look-and-Feel

This document outlines the visual language and styling guidelines established for the Nimbus DAW.

## Global Aesthetic
Nimbus employs a sleek, modern dark mode interface inspired by Ableton Live and Bitwig Studio. The interface minimizes gradients and drop shadows in favor of a flat, precise, and high-contrast aesthetic that reduces eye strain during long sessions.

## 1. Track Headers (`TrackHeaderComponent` & `GroupTrackHeaderComponent`)
- **Background**: Dark charcoal (`ModuleBackground`), brightening slightly when selected (`brighter(0.1f)`).
- **Typography**: Primary font (Inter/Roboto equivalent), size 13.0f, single-line to prevent wrapping, using ellipses for overflow.
- **Controls**: 
  - Flat SVG icons loaded natively, stripped of default JUCE button backgrounds.
  - Action colors: `Mute` illuminates Orange, `Solo` illuminates Yellow, `Arm` illuminates Red.
  - The chevron (fold toggle) explicitly avoids standard toggle backgrounds to sit cleanly next to the track numbering.

## 2. Channel Strips (`ChannelStripComponent`)
- **Layout**: Tall, vertical columns placed in the Bottom Mixer panel.
- **Master Strip**: Persists on the far right, independent of the scrolling track container. Hides track-specific controls like Input routing, Arming, and Pan.
- **Faders (`MeteredFader`)**:
  - Employs a custom `AbletonRotaryLAF` for panning.
  - Integrated VU meters run directly alongside the fader track.
  - Stereo tracks feature dual-channel VU meters; Mono features a single-channel meter.
- **MIDI Tracks (`MidiActivityMeter`)**:
  - Replaces the traditional fader with a slim, vertical activity indicator that responds to MIDI velocity events.

## 3. UI Control Primitives
- **Drawables & Icons**: SVG vectors are loaded via `juce::Drawable::createFromSVG`, heavily using `replaceColour` to map raw icon data to the `DesignSystem` palette.
- **Toggle Buttons**: We strictly use `juce::DrawableButton::ImageRaw` or customized TextButtons with `transparentBlack` backgrounds to override JUCE's default LookAndFeel rendering.

## 4. Layout Constraints
- Explicit paddings (e.g., 2px to 4px) are used universally via `juce::Rectangle::reduced()` and `withTrimmedTop()` to prevent elements from bleeding into the edges of their bounding boxes.
