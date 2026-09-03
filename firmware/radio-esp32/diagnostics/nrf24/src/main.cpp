#include <Arduino.h>
#include <SPI.h>

#include "Nrf24Diagnostic.hpp"

#ifndef OBSIDIA_NRF24_SCLK_GPIO
#define OBSIDIA_NRF24_SCLK_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_MISO_GPIO
#define OBSIDIA_NRF24_MISO_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_MOSI_GPIO
#define OBSIDIA_NRF24_MOSI_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_CSN_GPIO
#define OBSIDIA_NRF24_CSN_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_CE_GPIO
#define OBSIDIA_NRF24_CE_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_SPI_HZ
#define OBSIDIA_NRF24_SPI_HZ 1000000
#endif

namespace {

class ArduinoNrf24Bus final : public obsidia::radio::Nrf24RegisterBus {
public:
    ArduinoNrf24Bus(SPIClass &spi, int csn) : spi_(spi), csn_(csn) {}

    bool readStatus(std::uint8_t &status) override {
        beginTransaction();
        status = spi_.transfer(0xff); // NOP
        endTransaction();
        return true;
    }

    bool readRegister(std::uint8_t address, std::uint8_t &value) override {
        beginTransaction();
        spi_.transfer(address & 0x1fU); // R_REGISTER
        value = spi_.transfer(0xff);
        endTransaction();
        return true;
    }

    bool writeRegister(std::uint8_t address, std::uint8_t value) override {
        beginTransaction();
        spi_.transfer(0x20U | (address & 0x1fU)); // W_REGISTER
        spi_.transfer(value);
        endTransaction();
        return true;
    }

private:
    void beginTransaction() {
        spi_.beginTransaction(SPISettings(OBSIDIA_NRF24_SPI_HZ, MSBFIRST, SPI_MODE0));
        digitalWrite(csn_, LOW);
    }

    void endTransaction() {
        digitalWrite(csn_, HIGH);
        spi_.endTransaction();
    }

    SPIClass &spi_;
    int csn_;
};

SPIClass radioSpi(HSPI);

} // namespace

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("OBSIDIA NRF24 STANDALONE DIAGNOSTIC");
    if (OBSIDIA_NRF24_SCLK_GPIO < 0 || OBSIDIA_NRF24_MISO_GPIO < 0 ||
        OBSIDIA_NRF24_MOSI_GPIO < 0 || OBSIDIA_NRF24_CSN_GPIO < 0 ||
        OBSIDIA_NRF24_CE_GPIO < 0) {
        Serial.println("[ERROR] NRF24 SPI/CE GPIO unconfigured; no hardware test performed");
        return;
    }

    pinMode(OBSIDIA_NRF24_CSN_GPIO, OUTPUT);
    pinMode(OBSIDIA_NRF24_CE_GPIO, OUTPUT);
    digitalWrite(OBSIDIA_NRF24_CSN_GPIO, HIGH);
    digitalWrite(OBSIDIA_NRF24_CE_GPIO, LOW); // Keep the radio out of RX/TX modes.
    radioSpi.begin(
        OBSIDIA_NRF24_SCLK_GPIO,
        OBSIDIA_NRF24_MISO_GPIO,
        OBSIDIA_NRF24_MOSI_GPIO,
        OBSIDIA_NRF24_CSN_GPIO
    );
    delay(5);

    ArduinoNrf24Bus bus(radioSpi, OBSIDIA_NRF24_CSN_GPIO);
    const auto result = obsidia::radio::Nrf24Diagnostic(bus).run();
    if (result.passed) {
        Serial.printf("[PASS] STATUS=%02X CONFIG=%02X RF_CH=%02X RF_SETUP=%02X\n",
                      result.status, result.config, result.rfChannel, result.rfSetup);
    } else {
        Serial.printf("[ERROR] NRF24 diagnostic error=%u STATUS=%02X CONFIG=%02X RF_CH=%02X\n",
                      static_cast<unsigned>(result.error), result.status,
                      result.config, result.rfChannel);
    }
}

void loop() { delay(1000); }
