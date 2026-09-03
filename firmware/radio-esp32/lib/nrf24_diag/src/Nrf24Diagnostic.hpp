#pragma once

#include <cstdint>

namespace obsidia::radio {

class Nrf24RegisterBus {
public:
    virtual ~Nrf24RegisterBus() = default;
    virtual bool readStatus(std::uint8_t &status) = 0;
    virtual bool readRegister(std::uint8_t address, std::uint8_t &value) = 0;
    virtual bool writeRegister(std::uint8_t address, std::uint8_t value) = 0;
};

class Nrf24Diagnostic {
public:
    enum class Error : std::uint8_t {
        None,
        SpiRead,
        InvalidStatus,
        InvalidRegister,
        RegisterWrite,
        RegisterMismatch,
        RegisterRestore,
    };

    struct Result {
        bool passed{false};
        Error error{Error::None};
        std::uint8_t status{0};
        std::uint8_t config{0};
        std::uint8_t rfChannel{0};
        std::uint8_t rfSetup{0};
    };

    explicit Nrf24Diagnostic(Nrf24RegisterBus &bus) : bus_(bus) {}
    Result run();

private:
    static constexpr std::uint8_t kRegisterConfig = 0x00;
    static constexpr std::uint8_t kRegisterRfChannel = 0x05;
    static constexpr std::uint8_t kRegisterRfSetup = 0x06;

    Nrf24RegisterBus &bus_;
};

} // namespace obsidia::radio
