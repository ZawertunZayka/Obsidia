#include <Arduino.h>
#include <esp_chip_info.h>
#include <esp_heap_caps.h>

#include <cstdint>

namespace {

const char *flashModeName(FlashMode_t mode) {
    switch (mode) {
        case FM_QIO: return "QIO";
        case FM_QOUT: return "QOUT";
        case FM_DIO: return "DIO";
        case FM_DOUT: return "DOUT";
        case FM_FAST_READ: return "FAST_READ";
        case FM_SLOW_READ: return "SLOW_READ";
        default: return "UNKNOWN";
    }
}

void reportIdentity(Print &output) {
    esp_chip_info_t chip{};
    esp_chip_info(&chip);

    output.println("OBSIDIA ESP32-S3 BOARD IDENTITY");
    output.printf("chip_model=%s\n", ESP.getChipModel());
    output.printf("chip_revision=%u\n", ESP.getChipRevision());
    output.printf("chip_cores=%u\n", ESP.getChipCores());
    output.printf("chip_features=0x%08lX\n", static_cast<unsigned long>(chip.features));
    output.printf("cpu_mhz=%u\n", ESP.getCpuFreqMHz());
    output.printf("flash_bytes=%lu\n", static_cast<unsigned long>(ESP.getFlashChipSize()));
    output.printf("flash_speed_hz=%lu\n", static_cast<unsigned long>(ESP.getFlashChipSpeed()));
    output.printf("flash_mode=%s\n", flashModeName(ESP.getFlashChipMode()));
    output.printf("psram_initialized=%s\n", psramFound() ? "yes" : "no");
    output.printf("psram_bytes=%lu\n", static_cast<unsigned long>(ESP.getPsramSize()));
    output.printf("psram_heap_bytes=%lu\n",
                  static_cast<unsigned long>(heap_caps_get_total_size(MALLOC_CAP_SPIRAM)));
    output.println("identity_report_end");
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    const std::uint32_t started = millis();
    while (!Serial && static_cast<std::uint32_t>(millis() - started) < 3000U) delay(10);
    reportIdentity(Serial0);
    reportIdentity(Serial);
}

void loop() { delay(1000); }
