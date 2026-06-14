# nimbus

**Status:** 🚧 Early Development (Scaffolding & Core Engine)

nimbus is a modern, open-source Digital Audio Workstation (DAW) built from the ground up with a focus on **safety**, **professional audio routing**, and **minimalist design**. Starting as a Windows-only application, nimbus aims to provide a fast, reliable environment for music production and audio editing.

## 🚀 Core Philosophy

*   **Safety First:** The project prioritizes a robust save/recovery system and uses a "Lock-Free" Audio Graph architecture to prevent crashes and data corruption.
*   **Professional Audio:** Implements essential features like **Delay Compensation**, **Sample-Accurate Automation**, and **Hardware-Aware Performance** (ASIO/WASAPI).
*   **Extensibility:** Designed to scale from a simple 2-bus DAW to a complex multitrack studio with plugin hosting (VST3) and advanced MIDI capabilities.

## 🛠️ Getting Started (Build Instructions)

nimbus uses **CMake** for cross-platform build configuration and **JUCE** for GUI and audio primitives.

### Prerequisites
*   **Windows:** Visual Studio (with C++ Desktop Development workload) and CMake.
*   **JUCE Framework:** Downloaded and configured in your system's CMake path.

### Build Steps
1.  Clone the repository:
    ```bash
    git clone https://github.com/papingothegamer/nimbus.git
    cd nimbus
    ```

2.  Generate build files with CMake:
    ```bash
    cmake -B build
    ```

3.  Build the project (Visual Studio will open automatically):
    ```bash
    cmake --build build --config Release
    ```

4.  Run the application:
    *   The executable will be located in `build/bin/Release/nimbus.exe`
    *   Or run directly from the build folder:
        ```bash
        cmake --build build --config Release --target nimbus_App
        ```

## 🏗️ Project Architecture

The codebase is organized into several key modules:

*   `nimbus_core`: Contains the low-level audio engine, transport clock, and lock-free data structures.
*   `nimbus_gui`: The JUCE-based user interface components.
*   `nimbus_engine`: The "brain" of the DAW, handling audio graph construction, routing, and plugin hosting.
*   `nimbus_plugin_host`: The isolated sandbox process for running VST3 plugins safely.

## 📋 Development Roadmap

The project is currently in **Phase 1 (Scaffolding)** of its roadmap. Future development will focus on:

*   **Phase 2:** The Mixer & Basic DSP (Metering, EQ, Routing).
*   **Phase 3:** The Timeline & Waveform Rendering.
*   **Phase 4:** VST3 Plugin Integration (Sandboxed).
*   **Phase 5:** MIDI & Piano Roll.
*   **Phase 6:** Recording & Automation.

## 🤝 Contributing

Contributions are welcome! This project is an excellent opportunity to learn about professional audio programming. Please feel free to:
*   Report bugs (preferably with steps to reproduce).
*   Submit pull requests for missing features or performance improvements.
*   Help test the application on different hardware configurations.

## 📄 License

[LICENSE_FILE] (Currently under development)
