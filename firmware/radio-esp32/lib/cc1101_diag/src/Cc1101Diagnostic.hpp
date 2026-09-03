#pragma once

#include <cstdint>

namespace obsidia::radio {

class Cc1101RegisterBus {
public:
    virtual ~Cc1101RegisterBus() = default;
    virtual bool reset() = 0;
    virtual bool readConfig(std::uint8_t address, std::uint8_t &value) = 0;
    virtual bool readStatus(std::uint8_t address, std::uint8_t &value) = 0;
    virtual bool writeConfig(std::uint8_t address, std::uint8_t value) = 0;
};

class Cc1101Diagnostic {
public:
    enum class Error : std::uint8_t {
        None,
        ResetTimeout,
        SpiRead,
        InvalidIdentity,
        RegisterWrite,
        RegisterMismatch,
        RegisterRestore,
        InvalidState,
    };

    struct Result {
        bool passed{false};
        Error error{Error::None};
        std::uint8_t partNumber{0};
        std::uint8_t version{0};
        std::uint8_t marcState{0};
    };

    explicit Cc1101Diagnostic(Cc1101RegisterBus &bus) : bus_(bus) {}
    Result run();

private:
    static constexpr std::uint8_t kRegisterPacketLength = 0x06;
    static constexpr std::uint8_t kStatusPartNumber = 0x30;
    static constexpr std::uint8_t kStatusVersion = 0x31;
    static constexpr std::uint8_t kStatusMarcState = 0x35;

    Cc1101RegisterBus &bus_;
};

} // namespace obsidia::radio
