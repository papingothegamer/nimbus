# Nimbus Architecture

This document defines the high-level architecture, module breakdown, dependency rules, and key interfaces for Nimbus — a production-grade digital audio workstation. 

Our architectural philosophy centers around extreme performance, stability at scale, and uncompromising separation of concerns.

## 1. Module Breakdown

Nimbus is organized into distinct modules to enforce boundaries and ensure maintainability as the codebase grows to support hundreds of tracks and plugins.

*   **Core**: The foundation of the application. Contains fundamental types, threading primitives, lock-free data structures (e.g., SPSC queues), memory allocators (e.g., object pools for real-time use), and generic design patterns (Command, Observer).
*   **AudioEngine**: Manages the hardware interface (ASIO, WASAPI, CoreAudio) and owns the high-priority real-time audio thread. Responsible for the audio processing graph, graph scheduling, and the global transport clock.
*   **MidiEngine**: Manages MIDI inputs/outputs, hardware controllers, MIDI routing, and event scheduling. Designed with deep support for MPE (MIDI Polyphonic Expression).
*   **Timeline**: The data model for the arrangement. Manages Tracks, Clips (Audio and MIDI), the Tempo Map, Time Signatures, and playhead positioning.
*   **Mixer**: The routing and summing engine. Manages channel strips, volume/pan, sends, returns, subgroups, and the master bus. Handles delay compensation.
*   **PluginHost**: Handles the lifecycle, scanning, and instantiation of third-party plugins (initially VST3). Includes process sandboxing (IPC) to prevent rogue plugins from crashing the host DAW.
*   **DSP**: A library of high-performance, SIMD-optimized digital signal processing algorithms. Includes stock effects (EQ, Compression), oscillators, filters, and high-quality resampling algorithms.
*   **Automation**: The engine for time-varying parameter changes. Supports sample-accurate evaluation of curves and various modes (Read, Write, Latch, Touch).
*   **Recording**: Manages real-time disk I/O for capturing audio and MIDI. Handles take management, punch-in/punch-out, and pre-allocation of disk buffers to prevent dropouts.
*   **Rendering**: The offline processing engine. Responsible for bouncing projects, freezing tracks, and exporting stems as fast as the CPU allows.
*   **ProjectSystem**: Owns the document lifecycle, the Undo/Redo command stack, and state serialization.
*   **FileManagement**: Handles indexing user sample libraries, caching waveforms, managing project dependencies (assets), and project archiving.
*   **UI**: The graphical frontend built with JUCE. Strictly follows the Model-View-Presenter (MVP) or Model-View-ViewModel (MVVM) patterns. The UI is completely decoupled from the real-time processing engines.
*   **Utilities**: Common helper functions, math libraries, logging, and platform-specific abstractions.

## 2. Dependency Rules & Constraints

To maintain a pristine architecture, all modules must adhere to the following rules:

### 2.1 The "Golden Rule" of the Audio Thread
The audio processing thread is sacred. It must complete its work within the buffer deadline (e.g., < 2ms for a 64-sample buffer at 44.1kHz). Therefore, code executing on the audio thread **MUST NEVER**:
*   Allocate or deallocate memory (`new`, `delete`, `malloc`, `free`, `std::string`, `std::vector::push_back`, etc.).
*   Acquire locks, mutexes, or spinlocks.
*   Perform File I/O or Network I/O.
*   Call into UI frameworks or trigger OS window repaints.
*   Yield, sleep, or wait on arbitrary condition variables.

### 2.2 Module Dependency Direction
Dependencies must flow top-down. 
*   **UI** depends on everything but nothing depends on UI. The AudioEngine is completely ignorant of the UI's existence.
*   **AudioEngine** depends on **Mixer**, **Timeline**, **PluginHost**, and **Core**.
*   **Mixer**, **Timeline**, and **PluginHost** depend on **Core** and **DSP**.
*   **Core** and **Utilities** depend on nothing else in the project.

### 2.3 UI <-> Audio Thread Communication
Direct calls between the message thread (UI) and the audio thread are forbidden. All communication must happen via lock-free mechanisms:
*   **Audio to UI (e.g., level meters, playhead position):** Triple buffering or atomic variables.
*   **UI to Audio (e.g., parameter changes, transport commands):** Lock-free Single-Producer-Single-Consumer (SPSC) queues. The audio thread pops messages at the start of its process block.

### 2.4 No Singletons
Global singletons hide dependencies, make testing impossible, and create unpredictable lifecycle issues. We will use **Dependency Injection (DI)**. Services are instantiated at startup and passed down through constructors or a scoped ServiceLocator.

### 2.5 State Mutations & Undo/Redo
Any action that alters the project state must be encapsulated in an `ICommand`. The `ProjectSystem` manages a stack of these commands to provide infinite Undo/Redo.

## 3. Interface-Driven Design

Key components will be defined by pure virtual interfaces to allow mocking during unit tests and hot-swapping of implementations.

### 3.1 IAudioNode
The foundational interface for any object in the processing graph (Tracks, Plugins, Busses).
```cpp
class IAudioNode {
public:
    virtual ~IAudioNode() = default;
    
    // Called from the message thread when sample rate/block size changes
    virtual void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) = 0;
    virtual void releaseResources() = 0;
    
    // Called exclusively on the real-time audio thread
    // Must be lock-free and allocation-free
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;
};
```

### 3.2 IPlugin
An abstraction over different plugin formats (VST3, AU, internal).
```cpp
class IPlugin : public IAudioNode {
public:
    virtual void loadState(const juce::MemoryBlock& state) = 0;
    virtual void saveState(juce::MemoryBlock& state) = 0;
    virtual void setParameterValue(int parameterIndex, float normalizedValue) = 0;
    virtual float getParameterValue(int parameterIndex) const = 0;
    virtual bool hasEditor() const = 0;
    virtual juce::AudioProcessorEditor* createEditor() = 0;
};
```

### 3.3 ICommand
The basis of the Undo/Redo system.
```cpp
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual juce::String getName() const = 0;
};
```

### 3.4 ITransport
Abstracts the playback state.
```cpp
class ITransport {
public:
    virtual ~ITransport() = default;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void record() = 0;
    virtual void setPosition(double samplePosition) = 0;
    virtual double getCurrentPosition() const = 0;
    virtual bool isPlaying() const = 0;
    virtual bool isRecording() const = 0;
};
```

### 3.5 IEventDispatcher
For thread-safe event publishing (Observer pattern).
```cpp
template <typename EventType>
class IEventDispatcher {
public:
    virtual ~IEventDispatcher() = default;
    virtual void addListener(IEventListener<EventType>* listener) = 0;
    virtual void removeListener(IEventListener<EventType>* listener) = 0;
    virtual void dispatch(const EventType& event) = 0;
};
```
