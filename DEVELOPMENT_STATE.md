# Nimbus v2 - Development State & Handoff Guide

## 1. Overview & Current Stage
Development is paused as of August 2026. All latest code changes, graph routing fixes, and DSP build configurations are committed and synchronized to the remote Git branch:
`origin/feature/timestretch-and-fixes`

---

## 2. Recent Fixes & Current Status

### A. DetailView & Audio Panel UI
- **Issue**: The Audio Panel inside `ClipPropertiesComponent` remained locked in a placeholder/disabled state upon clip selection.
- **Resolution**: `ClipPropertiesComponent::setAudioClip()` was updated to call `audioPanel.setEnabled(clip != nullptr)`, enabling all real-time clip manipulation widgets (Warp, Pitch Shift slider, Gain dB slider, Pan, Algorithm selector, Reverse toggle, Half/Double speed buttons) when a valid audio clip is selected.

### B. MIDI Clip Playback in Audio Graph
- **Issue**: Recorded MIDI tracks failed to output sound through VST instruments during playback after recording stopped.
- **Resolution**: `PlaybackContext::createGraphFromProject()` previously only checked for `AudioClip` instances. It has been updated to check for `MidiClip` and instantiate cached `MidiClipNode` instances, adding them as inputs to the `TrackNode`. This routes all MIDI note-on/note-off sequences into the plugin chain during playback.

### C. DSP Compilation & Debug Mode Performance
- **Issue**: The phase vocoder and time-stretching DSP caused severe CPU overload warnings and audio dropouts when running in MSVC Debug mode.
- **Resolution**: `CMakeLists.txt` now specifies targeted `/O2` optimization flags for `Source/AudioEngine/DiskStreaming/TimeStretchReader.cpp` on MSVC builds, ensuring full debuggability for the rest of the application while preventing DSP audio thread buffer underruns.

---

## 3. Tracktion Engine Rework Architecture

### Context & Goal
The custom `signalsmith-stretch` and basic interpolation logic had several edge-case drift issues during dynamic tempo changes. Moving forward, Nimbus is migrating clip modifications to **Tracktion Engine's native backend modules** (`tracktion_engine/modules/tracktion_engine`).

### Architectural Plan
1. **Headless Backend Integration**:
   - Tracktion Engine is headless and provides no front-end GUI widgets.
   - We will utilize Tracktion Engine's `tracktion_engine::ClipEffects` pipeline (e.g. `VolumeEffect`, `PitchShiftEffect`, `FadeInOutEffect`, `ReverseEffect`, `MakeMonoEffect`) and `tracktion_engine::WarpTimeManager` / `TimeStretcher`.
2. **UI Binding**:
   - `ClipPropertiesComponent` will bind its controls (Warp mode, transients, pitch semitones, gain, pan, reverse) directly to the parameters of the underlying Tracktion `ClipEffects` / `WarpTimeManager`.
3. **Graph Processing**:
   - `AudioClipNode` will delegate block rendering to Tracktion's proven render job and stream reader pipeline rather than manual buffer manipulation.

---

## 4. VST Plugin Window State & Reversion

### Previous vs New Implementation
- In commit `a121bbadefa4deb984540382f533d28ceb3bddce`, third-party VST plugins were hosted in dedicated `PluginWindow` instances holding direct `PluginNode*` references.
- Moving forward, plugin editor hosting will follow this separate window paradigm to avoid editor destruction during graph rebuilds.
- `releaseResources()` will be maintained during graph rebuilds so plugins (e.g., Roland ZENOLOGY) properly re-initialize when sample rate or buffer sizes change.

---

## 5. Next Steps for Resuming Development

When development resumes:
1. **Clone/Checkout Branch**:
   ```bash
   git clone https://github.com/papingothegamer/nimbus.git
   git checkout feature/timestretch-and-fixes
   ```
2. **Complete Tracktion Engine Module Linking**:
   - Add `tracktion_engine` module path to `CMakeLists.txt`.
   - Link `tracktion_engine` and `tracktion_graph` in `target_link_libraries`.
3. **Refactor AudioClip to use Tracktion ClipEffects**:
   - Connect `AudioClip::pitchShiftSemitones`, `gain`, `pan`, and `reverse` to `tracktion_engine::ClipEffects`.
   - Integrate `tracktion_engine::TimeStretcher` into the streaming reader.
4. **Test MIDI & VST Graph End-to-End**:
   - Verify recording from MIDI keyboard / Virtual Keyboard.
   - Verify recorded `MidiClip` triggers VST sound on playback.
   - Verify track soloing with active VST effects.
