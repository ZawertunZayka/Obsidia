#pragma once

#include "FpgaService.hpp"
#include "RadioService.hpp"

#include <Arduino.h>
#include <SPI.h>

namespace obsidia {

class ArduinoRadioTransport final : public RadioTransport {
public:
    explicit ArduinoRadioTransport(Stream &stream) : stream_(stream) {}

    bool write(const std::uint8_t *data, std::size_t length) override;
    bool read(std::uint8_t &byte) override;

private:
    Stream &stream_;
};

class ArduinoFpgaTransport final : public FpgaTransport {
public:
    static constexpr std::size_t kMaxTransactionLength = FpgaService::kMaxFifoRead;

    ArduinoFpgaTransport(SPIClass &spi, int chipSelectGpio, std::uint32_t frequencyHz);

    bool begin();
    bool read(std::uint8_t address, std::uint8_t *data, std::size_t length) override;
    bool write(std::uint8_t address, std::uint8_t value) override;

private:
    bool valid(std::uint8_t address) const;

    SPIClass &spi_;
    int chipSelectGpio_;
    std::uint32_t frequencyHz_;
    bool configured_{false};
};

} // namespace obsidia
