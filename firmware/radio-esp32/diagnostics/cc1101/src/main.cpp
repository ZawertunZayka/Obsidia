#include <Arduino.h>
#include <SPI.h>

#include "Cc1101Diagnostic.hpp"

#ifndef OBSIDIA_CC1101_SCLK_GPIO
#define OBSIDIA_CC1101_SCLK_GPIO -1
#endif
#ifndef OBSIDIA_CC1101_MISO_GPIO
#define OBSIDIA_CC1101_MISO_GPIO -1
#endif
#ifndef OBSIDIA_CC1101_MOSI_GPIO
#define OBSIDIA_CC1101_MOSI_GPIO -1
#endif
#ifndef OBSIDIA_CC1101_CS_GPIO
#define OBSIDIA_CC1101_CS_GPIO -1
#endif
#ifndef OBSIDIA_CC1101_SPI_HZ
#define OBSIDIA_CC1101_SPI_HZ 1000000
#endif

namespace {

class ArduinoCc1101Bus final : public obsidia::radio::Cc1101RegisterBus {
public:
    ArduinoCc1101Bus(SPIClass &spi, int cs) : spi_(spi), cs_(cs) {}

    bool reset() override {
        digitalWrite(cs_, HIGH);
        delayMicroseconds(5);
        digitalWrite(cs_, LOW);
        delayMicroseconds(10);
        digitalWrite(cs_, HIGH);
        delayMicroseconds(50);
        if (!select()) return false;
        spi_.transfer(0x30); // SRES strobe
        if (!waitReady()) {
            deselect();
            return false;
        }
        deselect();
        delay(1);
        return true;
    }

    bool readConfig(std::uint8_t address, std::uint8_t &value) override {
        return read(address | 0x80U, value);
    }

    bool readStatus(std::uint8_t address, std::uint8_t &value) override {
        return read(address | 0xc0U, value);
    }

    bool writeConfig(std::uint8_t address, std::uint8_t value) override {
        if (!select()) return false;
        spi_.transfer(address & 0x3fU);
        spi_.transfer(value);
        deselect();
        return true;
    }

private:
    bool waitReady() {
        const std::uint32_t started = micros();
        while (digitalRead(OBSIDIA_CC1101_MISO_GPIO) != LOW) {
            if (static_cast<std::uint32_t>(micros() - started) >= 10000U) return false;
        }
        return true;
    }

    bool select() {
        spi_.beginTransaction(SPISettings(OBSIDIA_CC1101_SPI_HZ, MSBFIRST, SPI_MODE0));
        digitalWrite(cs_, LOW);
        if (waitReady()) return true;
        deselect();
        return false;
    }

    void deselect() {
        digitalWrite(cs_, HIGH);
        spi_.endTransaction();
    }

    bool read(std::uint8_t command, std::uint8_t &value) {
        if (!select()) return false;
        spi_.transfer(command);
        value = spi_.transfer(0x00);
        deselect();
        return true;
    }

    SPIClass &spi_;
    int cs_;
};

SPIClass radioSpi(HSPI);

} // namespace

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("OBSIDIA CC1101 STANDALONE DIAGNOSTIC");
    if (OBSIDIA_CC1101_SCLK_GPIO < 0 || OBSIDIA_CC1101_MISO_GPIO < 0 ||
        OBSIDIA_CC1101_MOSI_GPIO < 0 || OBSIDIA_CC1101_CS_GPIO < 0) {
        Serial.println("[ERROR] CC1101 SPI GPIO unconfigured; no hardware test performed");
        return;
    }
    pinMode(OBSIDIA_CC1101_CS_GPIO, OUTPUT);
    digitalWrite(OBSIDIA_CC1101_CS_GPIO, HIGH);
    radioSpi.begin(
        OBSIDIA_CC1101_SCLK_GPIO,
        OBSIDIA_CC1101_MISO_GPIO,
        OBSIDIA_CC1101_MOSI_GPIO,
        OBSIDIA_CC1101_CS_GPIO
    );
    ArduinoCc1101Bus bus(radioSpi, OBSIDIA_CC1101_CS_GPIO);
    const auto result = obsidia::radio::Cc1101Diagnostic(bus).run();
    if (result.passed) {
        Serial.printf("[PASS] PARTNUM=%02X VERSION=%02X MARCSTATE=%02X\n",
                      result.partNumber, result.version, result.marcState);
    } else {
        Serial.printf("[ERROR] CC1101 diagnostic error=%u PARTNUM=%02X VERSION=%02X\n",
                      static_cast<unsigned>(result.error), result.partNumber, result.version);
    }
}

void loop() { delay(1000); }
