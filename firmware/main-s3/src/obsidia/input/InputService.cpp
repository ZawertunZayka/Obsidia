#include "InputService.hpp"

namespace obsidia {

InputService::InputService() : InputService(Config{}) {}

InputService::InputService(Config config) : config_(config) {
    if (config_.debounceMs == 0U) config_.debounceMs = 1U;
    if (config_.longPressMs < config_.debounceMs) {
        config_.longPressMs = config_.debounceMs;
    }
}

bool InputService::elapsed(std::uint32_t nowMs, std::uint32_t sinceMs,
                           std::uint32_t intervalMs) {
    return static_cast<std::uint32_t>(nowMs - sinceMs) >= intervalMs;
}

void InputService::begin(std::uint8_t rawPressedMask, std::uint32_t nowMs) {
    stablePressedMask_ = rawPressedMask;
    queueHead_ = 0;
    queueSize_ = 0;
    droppedEventCount_ = 0;
    for (std::size_t key = 0; key < kKeyCount; ++key) {
        const bool pressed = (rawPressedMask & (1U << key)) != 0U;
        keys_[key] = {pressed, pressed, false, nowMs, nowMs};
    }
    initialized_ = true;
}

void InputService::poll(std::uint8_t rawPressedMask, std::uint32_t nowMs) {
    if (!initialized_) {
        begin(rawPressedMask, nowMs);
        return;
    }

    for (std::size_t key = 0; key < kKeyCount; ++key) {
        KeyState &state = keys_[key];
        const bool rawPressed = (rawPressedMask & (1U << key)) != 0U;
        if (rawPressed != state.rawPressed) {
            state.rawPressed = rawPressed;
            state.rawChangedAtMs = nowMs;
        }

        if (state.rawPressed != state.stablePressed &&
            elapsed(nowMs, state.rawChangedAtMs, config_.debounceMs)) {
            state.stablePressed = state.rawPressed;
            if (state.stablePressed) {
                stablePressedMask_ |= static_cast<std::uint8_t>(1U << key);
                state.pressedAtMs = nowMs;
                state.longPressEmitted = false;
                enqueue(static_cast<std::uint8_t>(key), EventType::Press, nowMs);
            } else {
                stablePressedMask_ &= static_cast<std::uint8_t>(~(1U << key));
                state.longPressEmitted = false;
                enqueue(static_cast<std::uint8_t>(key), EventType::Release, nowMs);
            }
        }

        if (state.stablePressed && !state.longPressEmitted &&
            elapsed(nowMs, state.pressedAtMs, config_.longPressMs)) {
            state.longPressEmitted = true;
            enqueue(static_cast<std::uint8_t>(key), EventType::LongPress, nowMs);
        }
    }
}

void InputService::enqueue(std::uint8_t key, EventType type, std::uint32_t nowMs) {
    if (queueSize_ == queue_.size()) {
        ++droppedEventCount_;
        return;
    }
    const std::size_t tail = (queueHead_ + queueSize_) % queue_.size();
    queue_[tail] = {key, type, nowMs};
    ++queueSize_;
}

bool InputService::nextEvent(Event &event) {
    if (queueSize_ == 0U) return false;
    event = queue_[queueHead_];
    queueHead_ = (queueHead_ + 1U) % queue_.size();
    --queueSize_;
    return true;
}

} // namespace obsidia
