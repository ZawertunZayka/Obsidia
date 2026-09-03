#include "Nrf24Diagnostic.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

namespace {

using Diagnostic = obsidia::radio::Nrf24Diagnostic;

class MockBus final : public obsidia::radio::Nrf24RegisterBus {
public:
    bool readStatus(std::uint8_t &value) override {
        if (!readsOk) return false;
        value = status;
        return true;
    }

    bool readRegister(std::uint8_t address, std::uint8_t &value) override {
        if (!readsOk) return false;
        value = registers[address & 0x1fU];
        return true;
    }

    bool writeRegister(std::uint8_t address, std::uint8_t value) override {
        ++writeCount;
        if (!writesOk || (failWriteNumber != 0 && writeCount == failWriteNumber)) return false;
        if (!ignoreWrites) registers[address & 0x1fU] = value;
        return true;
    }

    bool readsOk{true};
    bool writesOk{true};
    bool ignoreWrites{false};
    std::uint8_t status{0x0e};
    unsigned writeCount{0};
    unsigned failWriteNumber{0};
    std::array<std::uint8_t, 32> registers{};
};

MockBus validBus() {
    MockBus bus;
    bus.registers[0x00] = 0x08;
    bus.registers[0x05] = 0x02;
    bus.registers[0x06] = 0x0f;
    return bus;
}

void testPassAndRestore() {
    auto bus = validBus();
    const auto result = Diagnostic(bus).run();
    assert(result.passed);
    assert(result.status == 0x0e);
    assert(result.config == 0x08);
    assert(result.rfChannel == 0x02);
    assert(result.rfSetup == 0x0f);
    assert(bus.registers[0x05] == 0x02);
    assert(bus.writeCount == 2);
}

void testReadAndStatusFailures() {
    auto bus = validBus();
    bus.readsOk = false;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::SpiRead);

    bus = validBus();
    bus.status = 0xff;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidStatus);

    bus = validBus();
    bus.status = 0x80;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidStatus);
}

void testReservedRegisterBits() {
    auto bus = validBus();
    bus.registers[0x00] = 0x80;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidRegister);

    bus = validBus();
    bus.registers[0x05] = 0x80;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidRegister);
}

void testWriteAndMismatchFailures() {
    auto bus = validBus();
    bus.writesOk = false;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::RegisterWrite);

    bus = validBus();
    bus.ignoreWrites = true;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::RegisterMismatch);
    assert(bus.writeCount == 2);

    bus = validBus();
    bus.failWriteNumber = 2;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::RegisterRestore);
}

} // namespace

int main() {
    testPassAndRestore();
    testReadAndStatusFailures();
    testReservedRegisterBits();
    testWriteAndMismatchFailures();
    std::cout << "NRF24 diagnostic tests passed\n";
    return 0;
}
