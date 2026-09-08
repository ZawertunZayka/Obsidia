#include "core/bus_HAL.h"
#include "core/powerSave.h"
#include <Wire.h>
#include <interface.h>

namespace {

constexpr uint8_t kCardKbAddress = 0x5F;
constexpr uint32_t kCardKbFrequencyHz = 100000;
constexpr uint32_t kCardKbPollIntervalMs = 20;

constexpr uint8_t kKeyBackspace = 0x08;
constexpr uint8_t kKeyEnter = 0x0D;
constexpr uint8_t kKeyEscape = 0x1B;
constexpr uint8_t kKeyLeft = 0xB4;
constexpr uint8_t kKeyUp = 0xB5;
constexpr uint8_t kKeyDown = 0xB6;
constexpr uint8_t kKeyRight = 0xB7;

bool cardKbPresent = false;

bool probeCardKb() {
    Wire.beginTransmission(kCardKbAddress);
    return Wire.endTransmission(true) == 0;
}

uint8_t readCardKb() {
    const uint8_t received = Wire.requestFrom(kCardKbAddress, static_cast<uint8_t>(1), true);
    if (received != 1 || Wire.available() == 0) return 0;
    const int value = Wire.read();
    return value > 0 ? static_cast<uint8_t>(value) : 0;
}

void publishKey(uint8_t key) {
    if (wakeUpScreen()) return;

    AnyKeyPress = true;
    KeyStroke.Clear();
    KeyStroke.pressed = true;
    KeyStroke.hid_keys.push_back(key);

    const bool previous = key == kKeyLeft || key == kKeyUp || key == 'w' || key == 'W' || key == 'a' || key == 'A';
    const bool next = key == kKeyRight || key == kKeyDown || key == 's' || key == 'S' || key == 'd' || key == 'D';

    if (previous) {
        PrevPress = true;
        UpPress = true;
        PrevPagePress = key == kKeyLeft;
    } else if (next) {
        NextPress = true;
        DownPress = true;
        NextPagePress = key == kKeyRight;
    } else if (key == kKeyEnter) {
        SelPress = true;
        KeyStroke.enter = true;
    } else if (key == kKeyEscape || key == kKeyBackspace) {
        EscPress = true;
        KeyStroke.exit_key = key == kKeyEscape;
        KeyStroke.del = key == kKeyBackspace;
    }

    if ((key >= 0x20 && key <= 0x7E) || key == kKeyBackspace || key == kKeyEnter) {
        KeyStroke.word.push_back(static_cast<char>(key));
    }
}

} // namespace

void _setup_gpio() {
    pinMode(TFT_CS, OUTPUT);
    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(SDCARD_CS, HIGH);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    setSysI2CBus(&Wire);
    if (!Wire.begin(SYS_I2C_SDA, SYS_I2C_SCL, kCardKbFrequencyHz)) {
        Serial.println("[OBSIDIA] CardKB I2C initialization failed");
    }
    cardKbPresent = probeCardKb();
    Serial.printf("[OBSIDIA] CardKB 0x%02X: %s\n", kCardKbAddress, cardKbPresent ? "ready" : "not found");
}

void _post_setup_gpio() {
    pinMode(TFT_BL, OUTPUT);
    ledcAttach(TFT_BL, 5000, 8);
    ledcWrite(TFT_BL, 255);
}

int getBattery() { return 0; }

bool isCharging() { return false; }

void _setBrightness(uint8_t brightval) {
    const uint32_t duty = (static_cast<uint32_t>(brightval) * 255U) / 100U;
    ledcWrite(TFT_BL, duty);
}

void InputHandler() {
    static uint32_t lastPollMs = 0;
    static uint32_t lastProbeMs = 0;
    const uint32_t now = millis();

    if (static_cast<uint32_t>(now - lastProbeMs) >= 1000U) {
        lastProbeMs = now;
        const bool detected = probeCardKb();
        if (detected != cardKbPresent) {
            cardKbPresent = detected;
            Serial.printf("[OBSIDIA] CardKB: %s\n", cardKbPresent ? "reconnected" : "disconnected");
        }
    }

    if (!cardKbPresent || static_cast<uint32_t>(now - lastPollMs) < kCardKbPollIntervalMs) {
        KeyStroke.pressed = false;
        return;
    }
    lastPollMs = now;
    const uint8_t key = readCardKb();
    if (key == 0) {
        KeyStroke.pressed = false;
        return;
    }
    publishKey(key);
}

void powerOff() {
    digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON);
    esp_deep_sleep_start();
}

void checkReboot() {}
