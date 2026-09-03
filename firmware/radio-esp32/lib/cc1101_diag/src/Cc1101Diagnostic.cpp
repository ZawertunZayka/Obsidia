#include "Cc1101Diagnostic.hpp"

namespace obsidia::radio {

Cc1101Diagnostic::Result Cc1101Diagnostic::run() {
    Result result;
    if (!bus_.reset()) {
        result.error = Error::ResetTimeout;
        return result;
    }
    if (!bus_.readStatus(kStatusPartNumber, result.partNumber) ||
        !bus_.readStatus(kStatusVersion, result.version)) {
        result.error = Error::SpiRead;
        return result;
    }
    // PARTNUM is normally zero, so VERSION is the useful floating-bus guard.
    if (result.version == 0x00 || result.version == 0xff || result.partNumber == 0xff) {
        result.error = Error::InvalidIdentity;
        return result;
    }

    std::uint8_t original = 0;
    if (!bus_.readConfig(kRegisterPacketLength, original)) {
        result.error = Error::SpiRead;
        return result;
    }
    const std::uint8_t pattern = static_cast<std::uint8_t>(original ^ 0x5aU);
    if (!bus_.writeConfig(kRegisterPacketLength, pattern)) {
        result.error = Error::RegisterWrite;
        return result;
    }
    std::uint8_t observed = 0;
    if (!bus_.readConfig(kRegisterPacketLength, observed) || observed != pattern) {
        bus_.writeConfig(kRegisterPacketLength, original);
        result.error = Error::RegisterMismatch;
        return result;
    }
    if (!bus_.writeConfig(kRegisterPacketLength, original) ||
        !bus_.readConfig(kRegisterPacketLength, observed) || observed != original) {
        result.error = Error::RegisterRestore;
        return result;
    }
    if (!bus_.readStatus(kStatusMarcState, result.marcState)) {
        result.error = Error::SpiRead;
        return result;
    }
    result.marcState &= 0x1fU;
    if (result.marcState > 0x16U) {
        result.error = Error::InvalidState;
        return result;
    }
    result.passed = true;
    return result;
}

} // namespace obsidia::radio
