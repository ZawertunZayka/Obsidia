#include "Nrf24Diagnostic.hpp"

namespace obsidia::radio {

Nrf24Diagnostic::Result Nrf24Diagnostic::run() {
    Result result;
    if (!bus_.readStatus(result.status) ||
        !bus_.readRegister(kRegisterConfig, result.config) ||
        !bus_.readRegister(kRegisterRfChannel, result.rfChannel) ||
        !bus_.readRegister(kRegisterRfSetup, result.rfSetup)) {
        result.error = Error::SpiRead;
        return result;
    }

    // STATUS bit 7 is reserved. An all-ones response is also the usual open MISO bus.
    if ((result.status & 0x80U) != 0U || result.status == 0xffU) {
        result.error = Error::InvalidStatus;
        return result;
    }
    // CONFIG bit 7 and RF_CH bit 7 are reserved on nRF24L01+.
    if ((result.config & 0x80U) != 0U || (result.rfChannel & 0x80U) != 0U) {
        result.error = Error::InvalidRegister;
        return result;
    }

    const std::uint8_t originalChannel = result.rfChannel;
    const std::uint8_t testChannel = static_cast<std::uint8_t>((originalChannel + 37U) & 0x7fU);
    if (!bus_.writeRegister(kRegisterRfChannel, testChannel)) {
        result.error = Error::RegisterWrite;
        return result;
    }

    std::uint8_t observed = 0;
    if (!bus_.readRegister(kRegisterRfChannel, observed) || observed != testChannel) {
        bus_.writeRegister(kRegisterRfChannel, originalChannel);
        result.error = Error::RegisterMismatch;
        return result;
    }
    if (!bus_.writeRegister(kRegisterRfChannel, originalChannel) ||
        !bus_.readRegister(kRegisterRfChannel, observed) || observed != originalChannel) {
        result.error = Error::RegisterRestore;
        return result;
    }

    result.passed = true;
    return result;
}

} // namespace obsidia::radio
