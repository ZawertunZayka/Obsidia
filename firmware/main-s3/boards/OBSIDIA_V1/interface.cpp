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
uint8_t activeCardKbKey = 0;

void probeSdBreakoutPullups() {
    constexpr int8_t pins[] = {SDCARD_CS, SDCARD_MOSI, SDCARD_SCK, SDCARD_MISO};
    constexpr const char *names[] = {"CS", "MOSI", "SCK", "MISO"};

    Serial.println("[OBSIDIA][SD][WIRE] probing passive-breakout pull-ups");
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) pinMode(pins[i], INPUT_PULLDOWN);
    delay(20);
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) {
        Serial.printf(
            "[OBSIDIA][SD][WIRE] %s GPIO%d=%s\n",
            names[i],
            pins[i],
            digitalRead(pins[i]) == HIGH ? "HIGH (external pull-up detected)" : "LOW (no pull-up detected)"
        );
    }
}

uint8_t transferSdBitBang(uint8_t value) {
    uint8_t received = 0;
    for (int8_t bit = 7; bit >= 0; --bit) {
        digitalWrite(SDCARD_MOSI, (value & (1U << bit)) != 0 ? HIGH : LOW);
        delayMicroseconds(50);
        digitalWrite(SDCARD_SCK, HIGH);
        received = static_cast<uint8_t>((received << 1U) | (digitalRead(SDCARD_MISO) == HIGH ? 1U : 0U));
        delayMicroseconds(50);
        digitalWrite(SDCARD_SCK, LOW);
    }
    return received;
}

void probeSdCmd0BitBang() {
    constexpr uint8_t kCmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};

    pinMode(SDCARD_CS, OUTPUT);
    pinMode(SDCARD_SCK, OUTPUT);
    pinMode(SDCARD_MOSI, OUTPUT);
    pinMode(SDCARD_MISO, INPUT_PULLUP);
    digitalWrite(SDCARD_CS, HIGH);
    digitalWrite(SDCARD_SCK, LOW);
    digitalWrite(SDCARD_MOSI, HIGH);
    delay(20);

    for (uint8_t i = 0; i < 10; ++i) transferSdBitBang(0xFF);
    digitalWrite(SDCARD_CS, LOW);
    for (uint8_t byte : kCmd0) transferSdBitBang(byte);

    uint8_t response = 0xFF;
    uint8_t responseOffset = 0xFF;
    for (uint8_t i = 0; i < 32; ++i) {
        response = transferSdBitBang(0xFF);
        if ((response & 0x80U) == 0) {
            responseOffset = i;
            break;
        }
    }

    digitalWrite(SDCARD_CS, HIGH);
    transferSdBitBang(0xFF);
    Serial.printf(
        "[OBSIDIA][SD][BITBANG] cmd0=0x%02X byte=%u at approximately 10 kHz (expected 0x01)\n",
        response,
        responseOffset
    );
}

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
    probeSdBreakoutPullups();
    probeSdCmd0BitBang();

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
    // The persisted Bruce setting is loaded after _setup_gpio(). This panel
    // needs the opposite ST7735 inversion state from the controller default.
    // Persist it once so the Bruce Config switch also shows the real state.
    if (bruceConfig.colorInverted != 0) bruceConfig.setColorInverted(0);
    tft.invertDisplay(false);

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
            activeCardKbKey = 0;
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
        activeCardKbKey = 0;
        KeyStroke.pressed = false;
        return;
    }
    // CardKB can report a held key on consecutive I2C polls.  Publish only
    // the press edge so Enter cannot select an item in the newly opened menu.
    if (key == activeCardKbKey) {
        KeyStroke.pressed = false;
        return;
    }
    activeCardKbKey = key;
    publishKey(key);
}

void powerOff() {
    digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON);
    esp_deep_sleep_start();
}

void checkReboot() {}
