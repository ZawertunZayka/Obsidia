#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <cstring>

// Confirmed assignments: hardware/pinout/pinmap.md. No other modules used.
constexpr int kSdCs = 17;
constexpr int kSdSck = 18;
constexpr int kSdMosi = 3;
constexpr int kSdMiso = 46;
constexpr int kBacklight = 21;
TFT_eSPI display;
SPIClass sdSpi(HSPI);
uint8_t written[512];
uint8_t received[512];
const char *result = "NOT_RUN";
volatile uint32_t sdCsEdges = 0;
volatile uint32_t sdCsLowEdges = 0;

void ARDUINO_ISR_ATTR onSdCsChange() {
    ++sdCsEdges;
    if (digitalRead(kSdCs) == LOW) ++sdCsLowEdges;
}

bool checkRoot(const char *stage) {
    File root = SD.open("/");
    const bool ok = root && root.isDirectory();
    root.close();
    Serial.printf("[SD] root after %s=%s\n", stage, ok ? "OK" : "FAIL");
    return ok;
}

void rawTftTraffic() {
    digitalWrite(kSdCs, HIGH);
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, LOW);
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_CS, LOW);
    for (unsigned i = 0; i < 128; ++i) SPI.transfer(0x00); // ST7735 NOP commands
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
}

void configSummary() {
    File file = SD.open("/bruce.conf", FILE_READ);
    Serial.printf("[CONFIG] SD bruce.conf=%s\n", file ? "present" : "missing");
    if (!file) return;
    // Never emit Wi-Fi credentials or other private configuration fields.
    char line[192];
    while (file.available()) {
        const size_t n = file.readBytesUntil('\n', line, sizeof(line) - 1);
        line[n] = '\0';
        const char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (strncmp(p, "\"priColor\":", 11) == 0 ||
            strncmp(p, "\"themeFile\":", 12) == 0 ||
            strncmp(p, "\"themeOnSd\":", 12) == 0) Serial.println(p);
    }
    file.close();
}

bool stress() {
    char path[48];
    bool unused = false;
    for (unsigned i = 0; i < 100; ++i) {
        snprintf(path, sizeof(path), "/.obsidia_shared_spi_%02u.tmp", i);
        if (!SD.exists(path)) { unused = true; break; }
    }
    if (!unused) { Serial.println("[FAIL] no unused diagnostic filename"); return false; }

    for (unsigned cycle = 0; cycle < 100; ++cycle) {
        display.fillScreen((cycle & 1U) ? TFT_ORANGE : TFT_BLACK);
        for (unsigned i = 0; i < sizeof(written); ++i) written[i] = (i * 31U) ^ cycle;
        File f = SD.open(path, FILE_WRITE);
        if (!f) { Serial.printf("[FAIL] cycle=%u open-write\n", cycle); return false; }
        const size_t n = f.write(written, sizeof(written));
        f.flush(); f.close();
        display.drawPixel(cycle, cycle, TFT_WHITE);
        f = SD.open(path, FILE_READ);
        const bool sizeOk = f && f.size() == sizeof(written);
        const size_t count = f ? f.read(received, sizeof(received)) : 0;
        f.close();
        if (n != sizeof(written) || !sizeOk || count != sizeof(received) ||
            memcmp(written, received, sizeof(written)) != 0) {
            Serial.printf("[FAIL] cycle=%u write=%u read=%u size=%d verify=%d\n",
                          cycle, unsigned(n), unsigned(count), sizeOk,
                          memcmp(written, received, sizeof(written)) == 0);
            return false;
        }
        if (!SD.remove(path)) { Serial.printf("[FAIL] cycle=%u delete\n", cycle); return false; }
        if ((cycle + 1) % 10 == 0) Serial.printf("[PROGRESS] %u/100\n", cycle + 1);
        delay(1);
    }
    return true;
}

void setup() {
    Serial.begin(115200);
    pinMode(TFT_CS, OUTPUT); digitalWrite(TFT_CS, HIGH);
    pinMode(kSdCs, OUTPUT); digitalWrite(kSdCs, HIGH);
    pinMode(kBacklight, OUTPUT); digitalWrite(kBacklight, HIGH);
    delay(300);
    Serial.printf("[DUAL-SPI] Bruce SDK + local TFT_eSPI, display before SD, TFT=%lu Hz\n",
                  static_cast<unsigned long>(SPI_FREQUENCY));
    Serial.println("[PINS] TFT SCK=12 MISO=13 MOSI=11 CS=10; SD SCK=18 MISO=46 MOSI=3 CS=17");
    SPI.begin(12, 13, 11, TFT_CS);

    // Always bring up the display first so a failed SD probe cannot leave a
    // misleading white panel. SD uses a separate controller and pins.
    display.init();
    display.setRotation(1);
    display.invertDisplay(false);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_ORANGE, TFT_BLACK);
    display.drawCentreString("OBSIDIA SD TEST", display.width() / 2, 20, 1);

    if (!sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs)) {
        result = "FAIL_SD_SPI_BEGIN";
        Serial.println(result);
        display.drawCentreString(result, display.width() / 2, 45, 1);
        return;
    }
    delay(500);
    if (!SD.begin(kSdCs, sdSpi, 4000000, "/sd", 5, false)) {
        result = "FAIL_SD_BEFORE_TFT";
        Serial.println(result);
        display.drawCentreString(result, display.width() / 2, 45, 1);
        return;
    }
    Serial.printf("[SD] INIT OK type=%u size=%llu MiB\n", SD.cardType(), SD.cardSize() / 1048576ULL);
    configSummary();
    if (!checkRoot("mount")) { result = "FAIL_AFTER_MOUNT"; return; }

    rawTftTraffic();
    if (!checkRoot("raw-tft-traffic")) {
        result = "FAIL_AFTER_RAW_TFT_TRAFFIC";
        Serial.println(result);
        return;
    }

    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH); delay(5);
    digitalWrite(TFT_RST, LOW); delay(20);
    digitalWrite(TFT_RST, HIGH); delay(150);
    if (!checkRoot("tft-reset-only")) {
        result = "FAIL_AFTER_TFT_RESET";
        Serial.println(result);
        return;
    }

    attachInterrupt(digitalPinToInterrupt(kSdCs), onSdCsChange, CHANGE);
    sdCsEdges = 0;
    sdCsLowEdges = 0;
    Serial.printf(
        "[GPIO] before TFT init: SD_CS=%d TFT_CS=%d SCK=%d MOSI=%d MISO=%d\n",
        digitalRead(kSdCs), digitalRead(TFT_CS), digitalRead(TFT_SCLK),
        digitalRead(TFT_MOSI), digitalRead(TFT_MISO)
    );
    display.init(); display.setRotation(1); display.invertDisplay(false);
    Serial.printf(
        "[GPIO] after TFT init: SD_CS=%d TFT_CS=%d SCK=%d MOSI=%d MISO=%d\n",
        digitalRead(kSdCs), digitalRead(TFT_CS), digitalRead(TFT_SCLK),
        digitalRead(TFT_MOSI), digitalRead(TFT_MISO)
    );
    Serial.printf("[GPIO] SD_CS edges during TFT init=%lu low-edges=%lu\n",
                  static_cast<unsigned long>(sdCsEdges),
                  static_cast<unsigned long>(sdCsLowEdges));
    detachInterrupt(digitalPinToInterrupt(kSdCs));
    // A static CS-level check distinguishes an accidentally selected card from
    // corruption that happened during the TFT transaction itself.
    pinMode(kSdCs, OUTPUT);
    digitalWrite(kSdCs, HIGH);
    Serial.printf("[GPIO] SD_CS reasserted=%d\n", digitalRead(kSdCs));
    const bool rootOk = checkRoot("TFT init");
    result = rootOk && stress() ? "PASS_SHARED_SPI_100_400KHZ" : "FAIL_AFTER_TFT";
    Serial.println(result);
    SD.end();
    sdSpi.end();
}

void loop() {
    static uint32_t last = 0;
    if (millis() - last >= 5000U) { last = millis(); Serial.println(result); }
    delay(20);
}
