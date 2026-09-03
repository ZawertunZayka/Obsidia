#include "ArduinoTransports.hpp"

namespace obsidia {

bool ArduinoRadioTransport::write(const std::uint8_t *data, std::size_t length) {
    return data != nullptr && length != 0 && stream_.write(data, length) == length;
}

bool ArduinoRadioTransport::read(std::uint8_t &byte) {
    if (stream_.available() <= 0) return false;
    const int value = stream_.read();
    if (value < 0) return false;
    byte = static_cast<std::uint8_t>(value);
    return true;
}

ArduinoFpgaTransport::ArduinoFpgaTransport(
    SPIClass &spi,
    int chipSelectGpio,
    std::uint32_t frequencyHz
) : spi_(spi), chipSelectGpio_(chipSelectGpio), frequencyHz_(frequencyHz) {}

bool ArduinoFpgaTransport::valid(std::uint8_t address) const {
    return configured_ && address <= 0x7f && frequencyHz_ != 0;
}

bool ArduinoFpgaTransport::begin() {
    if (chipSelectGpio_ < 0 || frequencyHz_ == 0) {
        configured_ = false;
        return false;
    }
    pinMode(chipSelectGpio_, OUTPUT);
    digitalWrite(chipSelectGpio_, HIGH);
    configured_ = true;
    return true;
}

bool ArduinoFpgaTransport::read(
    std::uint8_t address,
    std::uint8_t *data,
    std::size_t length
) {
    if (!valid(address) || data == nullptr || length == 0 ||
        length > kMaxTransactionLength) {
        return false;
    }

    spi_.beginTransaction(SPISettings(frequencyHz_, MSBFIRST, SPI_MODE0));
    digitalWrite(chipSelectGpio_, LOW);
    spi_.transfer(static_cast<std::uint8_t>(0x80U | address));
    for (std::size_t i = 0; i < length; ++i) data[i] = spi_.transfer(0x00);
    digitalWrite(chipSelectGpio_, HIGH);
    spi_.endTransaction();
    return true;
}

bool ArduinoFpgaTransport::write(std::uint8_t address, std::uint8_t value) {
    if (!valid(address)) return false;
    spi_.beginTransaction(SPISettings(frequencyHz_, MSBFIRST, SPI_MODE0));
    digitalWrite(chipSelectGpio_, LOW);
    spi_.transfer(address);
    spi_.transfer(value);
    digitalWrite(chipSelectGpio_, HIGH);
    spi_.endTransaction();
    return true;
}

} // namespace obsidia
