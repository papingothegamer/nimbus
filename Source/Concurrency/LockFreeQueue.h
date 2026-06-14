#pragma once

#include <atomic>
#include <vector>
#include <cassert>
#include <optional>

namespace Nimbus {

/**
 * A wait-free, lock-free Single-Producer-Single-Consumer (SPSC) queue.
 * Safe for use between the UI thread and the Audio thread.
 * 
 * Capacity must be a power of 2 for fast modulo arithmetic.
 * Objects pushed into this queue must be trivially copyable or at least 
 * cheap to move/copy, as they are stored by value.
 */
template <typename T>
class LockFreeQueue {
public:
    explicit LockFreeQueue(size_t capacity)
        : mask(capacity - 1)
    {
        // Capacity must be a power of 2
        assert((capacity & (capacity - 1)) == 0 && "Capacity must be a power of 2");
        buffer.resize(capacity);
    }

    ~LockFreeQueue() = default;

    /**
     * Pushes an item to the queue. Returns true if successful, false if full.
     * Can be called from the Producer thread (e.g. UI).
     */
    bool push(const T& item) {
        const auto currentWrite = writeIndex.load(std::memory_order_relaxed);
        const auto nextWrite = currentWrite + 1;

        // Check if queue is full
        if ((nextWrite - readIndex.load(std::memory_order_acquire)) > buffer.size()) {
            return false;
        }

        buffer[currentWrite & mask] = item;
        writeIndex.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * Pushes an item via move semantics.
     */
    bool push(T&& item) {
        const auto currentWrite = writeIndex.load(std::memory_order_relaxed);
        const auto nextWrite = currentWrite + 1;

        if ((nextWrite - readIndex.load(std::memory_order_acquire)) > buffer.size()) {
            return false;
        }

        buffer[currentWrite & mask] = std::move(item);
        writeIndex.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * Pops an item from the queue. Returns true if successful, false if empty.
     * Can be called from the Consumer thread (e.g. Audio thread).
     */
    bool pop(T& outItem) {
        const auto currentRead = readIndex.load(std::memory_order_relaxed);

        // Check if queue is empty
        if (currentRead == writeIndex.load(std::memory_order_acquire)) {
            return false;
        }

        outItem = std::move(buffer[currentRead & mask]);
        readIndex.store(currentRead + 1, std::memory_order_release);
        return true;
    }

    /**
     * Checks if the queue is empty.
     */
    bool isEmpty() const {
        return readIndex.load(std::memory_order_acquire) == writeIndex.load(std::memory_order_acquire);
    }

private:
    std::vector<T> buffer;
    const size_t mask;
    
    // alignas helps prevent false sharing
    alignas(64) std::atomic<size_t> writeIndex{0};
    alignas(64) std::atomic<size_t> readIndex{0};
};

} // namespace Nimbus
