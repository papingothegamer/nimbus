# Nimbus Development Roadmap

This document outlines the phased development plan for Nimbus. The goal is to build a rock-solid, production-grade foundation before expanding into complex workflow features.

## Phase 1: MVP Scaffolding & Audio Engine Foundation
**Goal:** Establish the build system and prove the core audio engine can process sound with low latency.
*   CMake project setup with Catch2 and Clang tools.
*   JUCE GUI shell integration.
*   Hardware enumeration (ASIO, WASAPI).
*   Core Audio Graph implementation (lock-free).
*   Global Transport and sample-accurate clock.

## Phase 2: The Mixer & Basic DSP
**Goal:** Prove routing, summing, and basic signal processing capabilities.
*   Mixer routing matrix (Tracks -> Master).
*   Volume, Panning, and Level Metering (Audio -> UI triple buffering).
*   Internal DSP implementation: Basic EQ and Gain plugins to validate the IAudioNode interface.
*   Delay Compensation (PDC) engine.

## Phase 3: Timeline & Waveform Rendering
**Goal:** Implement the visual arrangement view and disk streaming.
*   Timeline data model (Tracks, Audio Clips).
*   Worker threads for asynchronous waveform generation.
*   Custom UI components for the Arranger View (Track headers, zoomable timeline, clip drawing).
*   Lock-free disk streaming for audio playback.

## Phase 4: Plugin Hosting (VST3)
**Goal:** Integrate third-party tools safely.
*   Plugin Sandbox process implementation (IPC shared memory bridge).
*   VST3 scanning and SQLite database integration.
*   Plugin UI embedding.
*   Basic parameter mapping.

## Phase 5: MIDI & Piano Roll
**Goal:** Add instrument and MIDI support.
*   MIDI hardware input integration.
*   Piano Roll UI and MIDI clip data model.
*   MIDI routing to VST3 Instruments.
*   Initial MPE (MIDI Polyphonic Expression) support.

## Phase 6: Recording & Automation
**Goal:** Make it a fully functional creation tool.
*   Real-time Audio and MIDI recording to the timeline.
*   Take management and basic comping.
*   Automation envelopes (Volume, Pan, Plugin Parameters) with sample-accurate evaluation.
*   Project System Serialization (Save/Load `.nimbusproj`).
*   Offline Export / Bounce to WAV.

---

# Future Roadmap (Post-MVP)

Once the MVP (Phases 1-6) is stable and performant, development will proceed to advanced, modern DAW features.

## Phase 7: Advanced Workflow
*   **Clip Launcher:** Ableton/Bitwig-style session view for non-linear live performance and composition.
*   **Advanced Comping:** Swipe comping across multiple audio takes.
*   **Groove Extraction & Quantization:** Audio-to-MIDI extraction and groove templates.

## Phase 8: Post-Production & Media
*   **Video Tracks:** Frame-accurate video playback for scoring and post-production.
*   **ARA 2 Integration:** Deep integration with Melodyne and VocALign.
*   **Time-stretching:** High-quality offline and real-time audio warping algorithms (e.g., integrating a licensed stretch engine or Rubberband).

## Phase 9: Modern Formats & Platform Expansion
*   **Spatial Audio:** Native Dolby Atmos and Ambisonics mixing support.
*   **Cloud Collaboration:** Project syncing, version control, and real-time remote session collaboration.
*   **Cross-Platform Port:** Porting the Windows-first codebase to macOS (CoreAudio/AU) and Linux (ALSA/JACK).
