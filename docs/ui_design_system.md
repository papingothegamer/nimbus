# Nimbus UI Design System

Nimbus requires a highly functional, low-fatigue user interface tailored for long studio sessions. The aesthetic must be strictly professional, emphasizing data density, immediate visual feedback, and zero distractions. 

As is standard for professional Digital Audio Workstations (DAWs), **we strictly avoid glassmorphism, heavy drop shadows, and unnecessary transparency.** The interface must feel tactile, matte, and highly responsive.

## 1. Color Palette

The interface relies on a deep, dark canvas with subtle variations in luminance to establish spatial hierarchy, keeping the focus entirely on the colorful audio/MIDI data and meters.

### Base Canvas (Matte Dark Theme)
- **Application Background:** `#FF111318` (The absolute bottom layer)
- **Panel Background (Mixer, Arranger, Piano Roll):** `#FF181A20`
- **Module Background (Plugin slots, Track headers):** `#FF22252D`
- **Component Background (Knobs, text fields, unselected buttons):** `#FF2A2D35`

### Borders and Separators
- **Subtle Dividers:** `#FF333742` (Used to separate tracks and mixer channels)
- **Component Borders:** `#FF4A4E5A` (Used for outlining active inputs or dropdowns)

### Accent Colors (Data & Interaction)
Colors must be vibrant but matte, avoiding harsh neon glows. 
- **Primary Action (Play, Active, Selected):** `#FF4A90E2` (A reliable, professional blue)
- **Record / Danger / Clip:** `#FFE04A4A` (Soft matte red)
- **Solo:** `#FFE0C04A` (Matte yellow)
- **Mute:** `#FFD35400` (Matte orange)
- **Success / Audio Signal (Meters):** `#FF2ECC71` (Matte green)

### Track Colors
User-assignable track colors should be selected from a predefined palette of desaturated, matte pastel colors to ensure white text remains legible against them.

## 2. Typography

A DAW requires extreme legibility at very small sizes (e.g., plugin parameter values, track names, DB readouts). 

- **Primary Font Family:** `Chakra Petch` (System UI sans-serif fallback).
- **Numbers/Meters Font:** A monospaced or tabular-lining variant (`Inter` with tabular numerals) to ensure that rapidly changing numbers (like DB meters or timecodes) do not cause horizontal jitter.

### Typographic Hierarchy
- **Timecode/Transport (Giant):** 24px - 32px, Bold, Monospaced.
- **Panel Titles (e.g., "MIXER", "ARRANGER"):** 14px, Bold, Tracking/Letter-spacing: +1px, Color: `#FF888C96`.
- **Primary Text (Track names, Button labels):** 13px, Medium, Color: `#FFE2E4E9`.
- **Secondary Text (Values, readouts):** 11px, Regular, Color: `#FFA0A4B0`.
- **Micro Text (Meter labels, scale markers):** 9px, Regular, Color: `#FF6B6F7D`.

## 3. Component Aesthetics

- **Flat & Solid:** Components use solid fills. No glassmorphism, no blurs, and minimal to no gradients. If a gradient is used, it should be an extremely subtle vertical linear gradient (e.g., 2% lighter at the top) purely to imply a tactile button shape.
- **Corners:** Hard edges or very tight border radii (2px to 4px max) to maximize screen real estate and maintain a technical, engineering-focused look.
- **States (Hover/Active):** 
  - *Hover:* Background lightens by 5-10%.
  - *Active/Pressed:* Background darkens or takes on the Primary Action color, text turns bright white.
- **Knobs & Sliders:** Vector-drawn using JUCE `juce::Path`. Knobs should have a clear, high-contrast indicator line.
- **Level Meters:** Segmented or continuous solid bars. Green -> Yellow (-6dB) -> Red (0dB). Must include peak hold lines.

## 4. Layout & Grid

- **Modular Panels:** The UI is divided into resizable, dockable panels with 1px solid borders.
- **Density:** High density. Margins and padding inside track headers and mixer channels should be minimal (4px - 8px) to fit as many channels on screen as possible.

## 5. Animation

- **Micro-interactions:** Instantaneous. No fade-ins for hover states (0ms delay) to make the DAW feel razor-sharp and snappy.
- **Meters & Playhead:** 60fps minimum, hardware-accelerated rendering. No smoothing on the playhead; it must reflect the exact audio engine sample position.
