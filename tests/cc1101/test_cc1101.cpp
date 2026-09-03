#include "Cc1101Diagnostic.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

namespace {

using Diagnostic = obsidia::radio::Cc1101Diagnostic;

class MockBus final : public obsidia::radio::Cc1101RegisterBus {
public:
    bool reset() override { return resetOk; }

    bool readConfig(std::uint8_t address, std::uint8_t &value) override {
        if (!readsOk) return false;
        value = config[address & 0x3fU];
        return true;
    }

    bool readStatus(std::uint8_t address, std::uint8_t &value) override {
        if (!readsOk) return false;
        value = status[address & 0x3fU];
        return true;
    }

    bool writeConfig(std::uint8_t address, std::uint8_t value) override {
        ++writeCount;
        if (!writesOk) return false;
        if (!ignoreWrites) config[address & 0x3fU] = value;
        return true;
    }

    bool resetOk{true};
    bool readsOk{true};
    bool writesOk{true};
    bool ignoreWrites{false};
    unsigned writeCount{0};
    std::array<std::uint8_t, 64> config{};
    std::array<std::uint8_t, 64> status{};
};

MockBus validBus() {
    MockBus bus;
    bus.status[0x30] = 0x00;
    bus.status[0x31] = 0x14;
    bus.status[0x35] = 0x01;
    bus.config[0x06] = 0xff;
    return bus;
}

void testPassAndRegisterRestore() {
    auto bus = validBus();
    const auto result = Diagnostic(bus).run();
    assert(result.passed);
    assert(result.error == Diagnostic::Error::None);
    assert(result.partNumber == 0x00);
    assert(result.version == 0x14);
    assert(result.marcState == 0x01);
    assert(bus.config[0x06] == 0xff);
    assert(bus.writeCount == 2);
}

void testResetAndReadFailures() {
    auto bus = validBus();
    bus.resetOk = false;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::ResetTimeout);

    bus = validBus();
    bus.readsOk = false;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::SpiRead);
}

void testInvalidIdentity() {
    auto bus = validBus();
    bus.status[0x31] = 0xff;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidIdentity);

    bus = validBus();
    bus.status[0x31] = 0x00;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidIdentity);
}

void testWriteAndMismatchFailures() {
    auto bus = validBus();
    bus.writesOk = false;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::RegisterWrite);

    bus = validBus();
    bus.ignoreWrites = true;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::RegisterMismatch);
    assert(bus.writeCount == 2); // Test write plus best-effort restoration.
}

void testInvalidMarcState() {
    auto bus = validBus();
    bus.status[0x35] = 0x17;
    assert(Diagnostic(bus).run().error == Diagnostic::Error::InvalidState);
}

} // namespace

int main() {
    testPassAndRegisterRestore();
    testResetAndReadFailures();
    testInvalidIdentity();
    testWriteAndMismatchFailures();
    testInvalidMarcState();
    std::cout << "CC1101 diagnostic tests passed\n";
    return 0;
}
