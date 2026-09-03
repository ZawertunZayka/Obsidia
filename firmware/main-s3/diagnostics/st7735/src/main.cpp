#include <Arduino.h>
#include <TFT_eSPI.h>

#include <cstdint>

namespace {

TFT_eSPI display;

struct TestFrame {
    std::uint16_t color;
    const char *name;
};

constexpr TestFrame kFrames[] = {
    {TFT_RED, "RED"},
    {TFT_GREEN, "GREEN"},
    {TFT_BLUE, "BLUE"},
    {TFT_BLACK, "BLACK"},
    {TFT_WHITE, "WHITE"},
};

void showFinalScreen() {
    display.fillScreen(TFT_BLACK);
    display.setTextDatum(MC_DATUM);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    display.drawString("OBSIDIA", display.width() / 2, display.height() / 2 - 14);
    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(1);
    display.drawString("DISPLAY OK", display.width() / 2, display.height() / 2 + 14);
    Serial.printf("[INFO] final screen width=%d height=%d rotation=0 profile=BLACKTAB\n",
                  display.width(), display.height());
    Serial.println("[WAIT] Physical confirmation required; software cannot read pixels back");
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("OBSIDIA ST7735 STANDALONE DIAGNOSTIC");

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    display.init();
    display.setRotation(0);
    display.invertDisplay(false);

    for (const auto &frame : kFrames) {
        Serial.printf("[TEST] %s\n", frame.name);
        display.fillScreen(frame.color);
        delay(1500);
    }
    showFinalScreen();
}

void loop() { delay(1000); }
