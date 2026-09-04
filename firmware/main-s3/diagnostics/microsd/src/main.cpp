#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr int kPinSdCs = 8;
constexpr int kPinSck = 12;
constexpr int kPinMosi = 11;
constexpr int kPinMiso = 13;
constexpr int kPinDisplayCs = 10;
constexpr std::uint32_t kMountFrequencyHz = 400000;
constexpr std::uint32_t kStressFrequencyHz = 4000000;
constexpr std::size_t kPayloadSize = 2048;
constexpr std::uint16_t kCycleCount = 100;

std::array<std::uint8_t, kPayloadSize> writeBuffer{};
std::array<std::uint8_t, kPayloadSize> readBuffer{};
bool completed = false;

void logLine(const char *message) {
    Serial0.println(message);
    Serial.println(message);
}

void logFailure(std::uint16_t cycle, const char *operation, const char *detail) {
    Serial0.printf("[FAIL] cycle=%u operation=%s detail=%s\n", cycle, operation, detail);
    Serial.printf("[FAIL] cycle=%u operation=%s detail=%s\n", cycle, operation, detail);
}

void fillPattern(std::uint16_t cycle) {
    std::uint32_t state = 0x4F425344U ^ cycle;
    for (std::size_t index = 0; index < writeBuffer.size(); ++index) {
        state ^= state << 13U;
        state ^= state >> 17U;
        state ^= state << 5U;
        writeBuffer[index] = static_cast<std::uint8_t>(state ^ index ^ cycle);
    }
}

bool mountCard() {
    for (std::uint8_t attempt = 1; attempt <= 3; ++attempt) {
        Serial0.printf("[MOUNT] attempt=%u frequency=%lu Hz\n", attempt,
                       static_cast<unsigned long>(kMountFrequencyHz));
        Serial.printf("[MOUNT] attempt=%u frequency=%lu Hz\n", attempt,
                      static_cast<unsigned long>(kMountFrequencyHz));
        digitalWrite(kPinSdCs, HIGH);
        if (SD.begin(kPinSdCs, SPI, kMountFrequencyHz, "/sd", 5, false)) return true;
        SD.end();
        delay(300);
    }
    return false;
}

bool runCycle(std::uint16_t cycle) {
    char path[32];
    snprintf(path, sizeof(path), "/.obsidia_sd_%03u.bin", cycle);
    if (SD.exists(path) && !SD.remove(path)) {
        logFailure(cycle, "pre_remove", path);
        return false;
    }

    fillPattern(cycle);
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        logFailure(cycle, "open_write", path);
        return false;
    }
    const std::size_t written = file.write(writeBuffer.data(), writeBuffer.size());
    file.flush();
    file.close();
    if (written != writeBuffer.size()) {
        logFailure(cycle, "write", "short write");
        SD.remove(path);
        return false;
    }

    readBuffer.fill(0);
    file = SD.open(path, FILE_READ);
    if (!file) {
        logFailure(cycle, "open_read", path);
        SD.remove(path);
        return false;
    }
    if (file.size() != writeBuffer.size()) {
        logFailure(cycle, "size", "unexpected file size");
        file.close();
        SD.remove(path);
        return false;
    }
    const std::size_t read = file.read(readBuffer.data(), readBuffer.size());
    file.close();
    if (read != readBuffer.size()) {
        logFailure(cycle, "read", "short read");
        SD.remove(path);
        return false;
    }
    if (std::memcmp(writeBuffer.data(), readBuffer.data(), writeBuffer.size()) != 0) {
        logFailure(cycle, "verify", "content mismatch");
        SD.remove(path);
        return false;
    }
    if (!SD.remove(path) || SD.exists(path)) {
        logFailure(cycle, "delete", path);
        return false;
    }
    return true;
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    delay(300);
    logLine("OBSIDIA MICROSD STANDALONE STRESS DIAGNOSTIC");

    pinMode(kPinDisplayCs, OUTPUT);
    digitalWrite(kPinDisplayCs, HIGH);
    pinMode(kPinSdCs, OUTPUT);
    digitalWrite(kPinSdCs, HIGH);
    SPI.begin(kPinSck, kPinMiso, kPinMosi, kPinSdCs);

    if (!mountCard()) {
        logLine("[FAIL] SD_MOUNT_FAILED after 3 attempts; check VCC/regulator/card/buffer wiring");
        completed = true;
        return;
    }

    const std::uint8_t type = SD.cardType();
    if (type == CARD_NONE) {
        logLine("[FAIL] SD_CARD_NONE");
        SD.end();
        completed = true;
        return;
    }
    Serial0.printf("[CARD] type=%u size=%llu MiB\n", type, SD.cardSize() / (1024ULL * 1024ULL));
    Serial.printf("[CARD] type=%u size=%llu MiB\n", type, SD.cardSize() / (1024ULL * 1024ULL));

    SD.end();
    delay(50);
    if (!SD.begin(kPinSdCs, SPI, kStressFrequencyHz, "/sd", 5, false)) {
        logLine("[FAIL] SD_REMOUNT_4MHZ_FAILED");
        completed = true;
        return;
    }

    for (std::uint16_t cycle = 0; cycle < kCycleCount; ++cycle) {
        if (!runCycle(cycle)) {
            completed = true;
            return;
        }
        if ((cycle + 1U) % 10U == 0U) {
            Serial0.printf("[PROGRESS] %u/%u cycles\n", cycle + 1U, kCycleCount);
            Serial.printf("[PROGRESS] %u/%u cycles\n", cycle + 1U, kCycleCount);
        }
    }
    logLine("[PASS] SD_STRESS_100 create/write/read/verify/delete");
    SD.end();
    completed = true;
}

void loop() {
    static std::uint32_t lastHeartbeat = 0;
    if (completed && static_cast<std::uint32_t>(millis() - lastHeartbeat) >= 5000U) {
        lastHeartbeat = millis();
        logLine("[IDLE] microSD diagnostic complete; reset to rerun");
    }
    delay(20);
}
