# Nimbus Plugin Hosting

This document details the strategy for integrating third-party audio plugins (primarily VST3) into the Nimbus ecosystem. Plugin hosting is notoriously difficult due to the wide variance in third-party code quality. Nimbus employs a robust, out-of-process sandboxing model to ensure DAW stability.

## 1. Plugin Sandboxing (Out-of-Process Hosting)

A single crashing plugin should never take down the entire DAW or cause the user to lose unsaved work. To prevent this, Nimbus hosts plugins in separate, lightweight helper processes.

### 1.1 Architecture
*   **Nimbus Host Process:** The main DAW application. Contains the UI, audio engine, and timeline.
*   **Nimbus Plugin Sandbox Process (`NimbusSandbox.exe`):** A headless worker process spawned by the host. Its sole job is to load a specific third-party `.vst3` DLL, instantiate it, and communicate with the host.

We will support two sandboxing modes:
1.  **Isolated (Highest Security):** Every plugin instance runs in its own dedicated sandbox process. A crash affects only that specific instance. (Higher memory overhead).
2.  **Bridged/Pooled (Balanced):** Plugins of a specific architecture (e.g., all 64-bit VST3s) run in a single shared sandbox process. A crash takes down the pool, but the DAW survives.

### 1.2 IPC Mechanism (Inter-Process Communication)
The Host and Sandbox communicate using a high-performance IPC bridge.

*   **Audio/MIDI Data (Real-time):** Passed via shared memory (`mmap` / `CreateFileMapping`). The host writes the input audio buffer and MIDI events to shared memory, signals an inter-process semaphore, and waits. The Sandbox processes the data and writes the output back to shared memory.
*   **Parameters & State (Real-time & Asynchronous):** Handled via named pipes or lock-free queues in shared memory.
*   **UI Integration:** The Sandbox process creates a hidden native window. When the user opens the plugin UI in Nimbus, the DAW uses OS-level window embedding (e.g., `SetParent` on Windows, `NSView` embedding on macOS) to draw the Sandbox's window inside the Nimbus UI frame.

### 1.3 Crash Recovery
If a Sandbox process terminates unexpectedly (crashes):
1.  The Host detects the broken IPC pipe/shared memory connection immediately.
2.  The Audio Engine instantly bypasses that specific node in the graph, replacing its output with silence to prevent loud audio bursts.
3.  The UI displays a "Plugin Crashed" placeholder where the plugin window used to be.
4.  The user is given the option to "Reload Plugin," which spawns a new Sandbox process and restores the last known good state (which the host maintains a copy of).

## 2. Plugin Scanning & Database

Users often have hundreds or thousands of plugins. Scanning must be fast, non-blocking, and fail-safe.

### 2.1 The Scanner Application
Scanning is performed by a separate utility application (`NimbusScanner.exe`). 
*   If a plugin crashes during scanning, only `NimbusScanner.exe` crashes. The main DAW detects the crash, marks that specific plugin DLL as "Failed/Blacklisted" in the database, and restarts the scanner to continue with the next file.

### 2.2 The Plugin Database
Scanned plugin metadata is stored in a local SQLite database (`plugins.db`).
*   **Metadata Stored:** Name, Vendor, Category (Instrument/Effect), UID, file path, architecture, last modified timestamp, and I/O bus configuration.
*   **Fast Startup:** On startup, Nimbus only checks file modification timestamps against the database. It does not load the actual `.vst3` DLLs until the user instantiates them.

## 3. Preset Management

VST3 provides a standardized way to handle state, but saving raw binary blobs in the project file makes them large and opaque.

*   **State Serialization:** When a project is saved, Nimbus requests the plugin's state as a `MemoryBlock`. This block is Base64 encoded and stored in the project JSON.
*   **Preset Library:** Users can save plugin states as Nimbus `.nmbpreset` files. These are small JSON wrappers around the Base64 state blob, allowing users to build a searchable library of favorite sounds independent of the project file.
