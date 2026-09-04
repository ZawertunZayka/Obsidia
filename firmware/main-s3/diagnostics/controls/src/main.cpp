#include <Arduino.h>
#include <InputService.hpp>

#include <array>
#include <cstdint>

namespace {

constexpr std::array<std::uint8_t, obsidia::InputService::kKeyCount> kKeyPins = {
    1, 2, 4, 5, 6, 7, 8, 15};

obsidia::InputService input({25, 600});
std::uint8_t observedPressMask = 0;
bool passReported = false;

void logLine(const char *message) {
    Serial0.println(message);
    Serial.println(message);
}

void logEvent(const obsidia::InputService::Event &event) {
    const char *type = "UNKNOWN";
    switch (event.type) {
        case obsidia::InputService::EventType::Press: type = "PRESS"; break;
        case obsidia::InputService::EventType::Release: type = "RELEASE"; break;
        case obsidia::InputService::EventType::LongPress: type = "LONG_PRESS"; break;
    }
    Serial0.printf("[KEY] K%u %s t=%lu ms\n", event.key + 1U, type,
                   static_cast<unsigned long>(event.timestampMs));
    Serial.printf("[KEY] K%u %s t=%lu ms\n", event.key + 1U, type,
                  static_cast<unsigned long>(event.timestampMs));
}

std::uint8_t readPressedMask() {
    std::uint8_t mask = 0;
    for (std::size_t key = 0; key < kKeyPins.size(); ++key) {
        if (digitalRead(kKeyPins[key]) == LOW) {
            mask |= static_cast<std::uint8_t>(1U << key);
        }
    }
    return mask;
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    delay(300);
    logLine("OBSIDIA CONTROLS STANDALONE DIAGNOSTIC");

    for (const std::uint8_t pin : kKeyPins) pinMode(pin, INPUT_PULLUP);
    delay(50);
    input.begin(readPressedMask(), millis());
    logLine("[READY] Press and release K1 through K8; hold one key for 0.6 s");
}

void loop() {
    const std::uint32_t now = millis();
    input.poll(readPressedMask(), now);

    obsidia::InputService::Event event;
    while (input.nextEvent(event)) {
        logEvent(event);
        if (event.type == obsidia::InputService::EventType::Press) {
            observedPressMask |= static_cast<std::uint8_t>(1U << event.key);
        }
    }

    if (!passReported && observedPressMask == 0xFFU) {
        passReported = true;
        logLine("[PASS] ALL 8 KEYS OBSERVED");
    }
    if (input.droppedEventCount() != 0U) {
        Serial0.printf("[ERROR] INPUT_QUEUE_OVERFLOW count=%lu\n",
                       static_cast<unsigned long>(input.droppedEventCount()));
        Serial.printf("[ERROR] INPUT_QUEUE_OVERFLOW count=%lu\n",
                      static_cast<unsigned long>(input.droppedEventCount()));
        delay(1000);
    }
    delay(5);
}
