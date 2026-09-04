#include <Arduino.h>

#include <cstdint>

namespace {

constexpr std::uint8_t kIrTxPin = 17;
constexpr std::uint32_t kCarrierHz = 38000;
constexpr std::uint8_t kResolutionBits = 8;
constexpr std::uint8_t kDuty25Percent = 64;
constexpr std::uint32_t kBurstMs = 150;
constexpr std::uint32_t kPeriodMs = 1000;

bool carrierReady = false;
std::uint32_t nextBurstAtMs = 0;
std::uint32_t stopBurstAtMs = 0;
bool burstActive = false;
std::uint32_t burstCount = 0;

void logLine(const char *message) {
    Serial0.println(message);
    Serial.println(message);
}

bool reached(std::uint32_t now, std::uint32_t deadline) {
    return static_cast<std::int32_t>(now - deadline) >= 0;
}

void stopCarrier() {
    ledcWrite(kIrTxPin, 0);
    burstActive = false;
}

void startCarrier(std::uint32_t now) {
    ledcWrite(kIrTxPin, kDuty25Percent);
    burstActive = true;
    stopBurstAtMs = now + kBurstMs;
    ++burstCount;
    Serial0.printf("[IR_TX] burst=%lu carrier=%luHz duty=25%%\n",
                   static_cast<unsigned long>(burstCount),
                   static_cast<unsigned long>(kCarrierHz));
    Serial.printf("[IR_TX] burst=%lu carrier=%luHz duty=25%%\n",
                  static_cast<unsigned long>(burstCount),
                  static_cast<unsigned long>(kCarrierHz));
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    delay(300);
    logLine("OBSIDIA KY-005 IR TX STANDALONE DIAGNOSTIC");
    logLine("[INFO] GPIO17, 38kHz, 25% duty, 150ms burst every 1s");
    logLine("[INFO] Observe the IR LED through a phone camera; IR may look violet/white");

    pinMode(kIrTxPin, OUTPUT);
    digitalWrite(kIrTxPin, LOW);
    carrierReady = ledcAttach(kIrTxPin, kCarrierHz, kResolutionBits);
    if (!carrierReady) {
        logLine("[ERROR] IR_TX_LEDC_ATTACH_FAILED");
        return;
    }

    stopCarrier();
    nextBurstAtMs = millis() + 500;
    logLine("[READY] IR_TX_CAMERA_TEST");
}

void loop() {
    if (!carrierReady) {
        delay(1000);
        return;
    }

    const std::uint32_t now = millis();
    if (burstActive && reached(now, stopBurstAtMs)) {
        stopCarrier();
        nextBurstAtMs = now + (kPeriodMs - kBurstMs);
    } else if (!burstActive && reached(now, nextBurstAtMs)) {
        startCarrier(now);
    }
    delay(1);
}
