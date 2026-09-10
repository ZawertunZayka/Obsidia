#include <Arduino.h>
#include <Wire.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <freertos/idf_additions.h>
#include <obsidia_doom.h>

#include "core/display.h"

namespace {
constexpr uint8_t kCardKbAddress = 0x5f;
TaskHandle_t doomTaskHandle = nullptr;

extern "C" int doom_main(int argc, const char *const *argv);

void doomTask(void *) {
    Serial.println("[DOOM] engine task starting; WAD source=internal flash");
    static const char *argv[] = {"doom", "-cout", "ICWEFDA", "-nosound", "-nomusic", nullptr};
    doom_main(6, argv);
    doomTaskHandle = nullptr;
    vTaskDelete(nullptr);
}
} // namespace

extern "C" bool obsidia_doom_wad_present(void) {
    const esp_partition_t *part = esp_partition_find_first(
        static_cast<esp_partition_type_t>(0x42), static_cast<esp_partition_subtype_t>(0x06), "doomwad"
    );
    if (!part || part->size < 4) return false;
    char magic[4] = {};
    return esp_partition_read(part, 0, magic, sizeof(magic)) == ESP_OK &&
           (memcmp(magic, "IWAD", 4) == 0 || memcmp(magic, "PWAD", 4) == 0);
}

extern "C" void obsidia_doom_start(void) {
    if (doomTaskHandle) return;
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("DOOM", tft.width() / 2, tft.height() / 2);
    const BaseType_t created = xTaskCreatePinnedToCoreWithCaps(
        doomTask, "obsidia-doom", 32768, nullptr, 3, &doomTaskHandle, 0, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );
    if (created != pdPASS) {
        doomTaskHandle = nullptr;
        Serial.println("[DOOM] ERROR: unable to create engine task");
        displayError("DOOM task failed", true);
        return;
    }
    while (doomTaskHandle) delay(100);
}

extern "C" int obsidia_doom_poll_key(void) {
    Wire.requestFrom(kCardKbAddress, static_cast<uint8_t>(1));
    return Wire.available() ? Wire.read() : 0;
}

extern "C" void obsidia_doom_present(const unsigned char *pixels, const short *palette) {
    // Doom renders 320x240. Every second source pixel produces a native
    // 160x120 frame, centred vertically on the 160x128 Obsidia display.
    static uint16_t line[160];
    const int x0 = (tft.width() - 160) / 2;
    const int y0 = (tft.height() - 120) / 2;
    for (int y = 0; y < 120; ++y) {
        const unsigned char *src = pixels + (y * 2 * 320);
        for (int x = 0; x < 160; ++x) line[x] = static_cast<uint16_t>(palette[src[x * 2]]);
        tft.pushImage(x0, y0 + y, 160, 1, line);
    }
}
