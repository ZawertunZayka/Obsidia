#include <Arduino.h>
#include <Wire.h>

#include <cstdint>

namespace {

constexpr std::uint8_t kCardKbAddress = 0x5F;
constexpr int kSdaPin = 40;
constexpr int kSclPin = 41;
constexpr std::uint32_t kI2cFrequencyHz = 100000;
constexpr std::uint32_t kPollIntervalMs = 20;
constexpr std::uint32_t kPresenceIntervalMs = 1000;
constexpr std::uint32_t kStatusIntervalMs = 5000;

bool present = false;
bool keyPassReported = false;
std::uint32_t lastPollMs = 0;
std::uint32_t lastPresenceMs = 0;
std::uint32_t lastStatusMs = 0;
std::uint32_t keyCount = 0;

void logLine(const char *message) {
    Serial0.println(message);
    Serial.println(message);
}

bool probe() {
    Wire.beginTransmission(kCardKbAddress);
    return Wire.endTransmission(true) == 0;
}

void reportKey(std::uint8_t value) {
    ++keyCount;
    const char printable = value >= 0x20U && value <= 0x7EU
                               ? static_cast<char>(value)
                               : '.';
    Serial0.printf("[KEY] count=%lu hex=0x%02X ascii=%c\n",
                   static_cast<unsigned long>(keyCount), value, printable);
    Serial.printf("[KEY] count=%lu hex=0x%02X ascii=%c\n",
                  static_cast<unsigned long>(keyCount), value, printable);
    if (!keyPassReported) {
        keyPassReported = true;
        logLine("[PASS] CARDKB_I2C_KEY_RECEIVED");
    }
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    delay(300);
    logLine("OBSIDIA M5STACK CARDKB STANDALONE DIAGNOSTIC");
    logLine("[INFO] address=0x5F SDA=GPIO40 SCL=GPIO41 bus=100kHz supply-test=3.3V");

    if (!Wire.begin(kSdaPin, kSclPin, kI2cFrequencyHz)) {
        logLine("[ERROR] CARDKB_I2C_INIT_FAILED");
        return;
    }
    present = probe();
    logLine(present ? "[OK] CARDKB_ADDRESS_ACK" : "[ERROR] CARDKB_NO_ACK");
    lastPresenceMs = millis();
    lastStatusMs = millis();
}

void loop() {
    const std::uint32_t now = millis();
    if (static_cast<std::uint32_t>(now - lastPresenceMs) >= kPresenceIntervalMs) {
        lastPresenceMs = now;
        const bool detected = probe();
        if (detected != present) {
            present = detected;
            logLine(present ? "[RECOVERY] CARDKB_ADDRESS_ACK"
                            : "[ERROR] CARDKB_DISCONNECTED");
        }
    }

    if (present && static_cast<std::uint32_t>(now - lastPollMs) >= kPollIntervalMs) {
        lastPollMs = now;
        const std::uint8_t received = Wire.requestFrom(
            static_cast<std::uint8_t>(kCardKbAddress), static_cast<std::uint8_t>(1), true);
        if (received == 1U && Wire.available() > 0) {
            const int value = Wire.read();
            if (value > 0) reportKey(static_cast<std::uint8_t>(value));
        }
    }
    if (static_cast<std::uint32_t>(now - lastStatusMs) >= kStatusIntervalMs) {
        lastStatusMs = now;
        logLine(present ? "[STATUS] CARDKB_ADDRESS_ACK"
                        : "[STATUS] CARDKB_NO_ACK");
    }
    delay(1);
}
