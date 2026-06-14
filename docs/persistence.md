# Nimbus Persistence Strategy

This document describes how project data is serialized, saved to disk, and managed over time. The safety of user data is critical; DAW crashes or power failures should never result in a corrupted, unrecoverable project file.

## 1. Project Format (`.nimbusproj`)

Initially, Nimbus projects will be saved as human-readable JSON files. This allows for rapid debugging during development and ensures that we can easily inspect the DOM structure.

### 1.1 JSON Structure
The `.nimbusproj` file contains the complete state of the `ProjectSystem`, serialized hierarchically.

```json
{
  "version": "1.0.0",
  "projectInfo": {
    "sampleRate": 48000,
    "bpm": 120.0,
    "timeSignature": [4, 4]
  },
  "tracks": [
    {
      "id": "trk_1a2b3c",
      "name": "Lead Synth",
      "type": "instrument",
      "color": "#6EC1FF",
      "volume": 0.8,
      "pan": 0.0,
      "plugins": [
        {
          "id": "plg_9x8y7z",
          "vst3_uid": "A1B2C3D4...",
          "state_base64": "eJztwTEBAAAAwqD1T+1..."
        }
      ],
      "clips": [
        {
          "id": "clp_4d5e6f",
          "start_beat": 0.0,
          "length_beats": 4.0,
          "type": "midi",
          "data_ref": "internal"
        }
      ]
    }
  ],
  "routing": { ... }
}
```

### 1.2 Binary Migration Path
While JSON is excellent for MVP development, parsing large JSON files (e.g., 100+ tracks with thousands of automation points) can become a bottleneck. 

*   **Future Strategy:** We will eventually migrate to a binary format (e.g., FlatBuffers or a custom binary tree structure).
*   **Forward Compatibility:** The ProjectSystem's serialization interfaces will be abstracted so that swapping the underlying serializer (JSON -> Binary) does not require changing the core data model. The application will always be able to read legacy JSON projects and upgrade them.

## 2. Save Safety & Crash Prevention

Writing directly to the target `.nimbusproj` file is dangerous. If the application crashes halfway through the write operation, the file is corrupted.

### 2.1 The Atomic Save Process
Nimbus uses an atomic saving strategy:
1.  Serialize the project state into a temporary file in the same directory (e.g., `project_name.nimbusproj.tmp`).
2.  Once the entire file is written and flushed to disk successfully, OS-level atomic rename is used to swap the old file with the new one.
3.  On Windows, this uses `ReplaceFile()`. On POSIX, it uses `rename()`.

## 3. Autosave & Recovery Strategy

To protect users against crashes (either DAW crashes or OS/Power failures), Nimbus implements a robust autosave system.

### 3.1 Background Autosaving
*   **Trigger:** Autosave occurs on a timed interval (e.g., every 5 minutes) AND whenever the user is idle (no UI interaction or transport movement for 10 seconds).
*   **Thread:** Serialization happens on a low-priority Worker Thread, ensuring the UI and Audio engines do not stutter. (Note: the state tree must be safely cloned before being passed to the worker thread to prevent data races).
*   **Storage:** Autosaves are written to a hidden `.backup` folder within the project directory, timestamped (e.g., `autosave_20260614_1430.nimbusproj`).

### 3.2 Crash Recovery
On launch, Nimbus checks for the presence of an unexpected crash flag or lock file.
1.  If a crash is detected, Nimbus scans the `.backup` directory.
2.  It compares the timestamp of the latest autosave against the last user-saved `.nimbusproj`.
3.  If the autosave is newer, Nimbus prompts the user: *"Nimbus shut down unexpectedly. Would you like to recover the most recent autosave?"*

## 4. Asset Management (Self-Contained Projects)

A DAW project is useless if the underlying audio samples are missing.
*   When importing a sample (e.g., a kick drum WAV) into the timeline, Nimbus defaults to copying that file into a local `/Audio` folder next to the `.nimbusproj` file.
*   This ensures that the project directory is a self-contained package that can be zipped and shared without risking "Missing File" errors on another machine.
