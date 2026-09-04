#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia {

class InputService {
public:
    static constexpr std::size_t kKeyCount = 8;
    static constexpr std::size_t kQueueCapacity = 16;

    enum class EventType : std::uint8_t {
        Press,
        Release,
        LongPress,
    };

    struct Event {
        std::uint8_t key = 0;
        EventType type = EventType::Press;
        std::uint32_t timestampMs = 0;
    };

    struct Config {
        std::uint16_t debounceMs = 25;
        std::uint16_t longPressMs = 600;
    };

    InputService();
    explicit InputService(Config config);

    void begin(std::uint8_t rawPressedMask, std::uint32_t nowMs);
    void poll(std::uint8_t rawPressedMask, std::uint32_t nowMs);
    bool nextEvent(Event &event);

    std::uint8_t stablePressedMask() const { return stablePressedMask_; }
    std::uint32_t droppedEventCount() const { return droppedEventCount_; }
    bool initialized() const { return initialized_; }

private:
    struct KeyState {
        bool rawPressed = false;
        bool stablePressed = false;
        bool longPressEmitted = false;
        std::uint32_t rawChangedAtMs = 0;
        std::uint32_t pressedAtMs = 0;
    };

    static bool elapsed(std::uint32_t nowMs, std::uint32_t sinceMs,
                        std::uint32_t intervalMs);
    void enqueue(std::uint8_t key, EventType type, std::uint32_t nowMs);

    Config config_;
    std::array<KeyState, kKeyCount> keys_{};
    std::array<Event, kQueueCapacity> queue_{};
    std::size_t queueHead_ = 0;
    std::size_t queueSize_ = 0;
    std::uint8_t stablePressedMask_ = 0;
    std::uint32_t droppedEventCount_ = 0;
    bool initialized_ = false;
};

} // namespace obsidia
