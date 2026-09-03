#include "FpgaService.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <iostream>

namespace {

class MockTransport final : public obsidia::FpgaTransport {
public:
    bool read(std::uint8_t address, std::uint8_t *data, std::size_t length) override {
        if (failReads) return false;
        if (address == 0x08) {
            for (std::size_t i = 0; i < length; ++i) {
                data[i] = fifo.empty() ? 0 : fifo.front();
                if (!fifo.empty()) fifo.pop_front();
            }
            return true;
        }
        for (std::size_t i = 0; i < length; ++i) data[i] = registers[address + i];
        if (address == 0x09 && length == 1) data[0] = static_cast<std::uint8_t>(fifo.size());
        return true;
    }

    bool write(std::uint8_t address, std::uint8_t value) override {
        if (failWrites) return false;
        lastWriteAddress = address;
        lastWriteValue = value;
        if (address == 0x07 && (value & 0x02U)) fifo.clear();
        return true;
    }

    void validDevice() {
        registers[0] = 'O';
        registers[1] = 'B';
        registers[2] = 'S';
        registers[3] = 'D';
        registers[4] = 1;
        registers[5] = 0;
        registers[6] = 1;
    }

    std::array<std::uint8_t, 128> registers{};
    std::deque<std::uint8_t> fifo;
    bool failReads{false};
    bool failWrites{false};
    std::uint8_t lastWriteAddress{0};
    std::uint8_t lastWriteValue{0};
};

void testDiscoveryHeartbeatAndRecovery() {
    MockTransport transport;
    transport.validDevice();
    obsidia::FpgaService service(transport);
    assert(service.begin(0));
    assert(service.snapshot().health == obsidia::FpgaService::Health::Ready);
    assert(service.snapshot().version[0] == 1);

    transport.registers[0] = 0;
    service.poll(1000);
    assert(service.snapshot().health == obsidia::FpgaService::Health::Offline);
    assert(service.snapshot().error == obsidia::FpgaService::Error::IdentityMismatch);

    transport.validDevice();
    service.poll(1499);
    assert(service.snapshot().health == obsidia::FpgaService::Health::Offline);
    service.poll(1500);
    assert(service.snapshot().health == obsidia::FpgaService::Health::Ready);
    assert(service.snapshot().successfulDiscoveries == 2);
}

void testFifoAndControlBounds() {
    MockTransport transport;
    transport.validDevice();
    transport.fifo = {0x10, 0x20, 0x30};
    obsidia::FpgaService service(transport);
    assert(service.begin(0));

    std::array<std::uint8_t, 2> bytes{};
    std::size_t length = 99;
    assert(service.readFifo(bytes.data(), bytes.size(), length));
    assert(length == 2 && bytes[0] == 0x10 && bytes[1] == 0x20);
    assert(service.snapshot().fifoLevel == 1);
    assert(service.acknowledgeEvents());
    assert(transport.lastWriteAddress == 0x07 && transport.lastWriteValue == 0x01);
    assert(service.clearFifo());
    assert(transport.fifo.empty());
    assert(transport.lastWriteValue == 0x02);

    assert(!service.readFifo(nullptr, 0, length));
    assert(length == 0);
    assert(service.snapshot().error == obsidia::FpgaService::Error::InvalidArgument);
}

void testTransportAndReadyErrors() {
    MockTransport transport;
    transport.validDevice();
    transport.registers[6] = 0;
    obsidia::FpgaService service(transport);
    assert(!service.begin(0));
    assert(service.snapshot().error == obsidia::FpgaService::Error::NotReady);

    MockTransport broken;
    broken.failReads = true;
    obsidia::FpgaService brokenService(broken);
    assert(!brokenService.begin(0));
    assert(brokenService.snapshot().error == obsidia::FpgaService::Error::Transport);
}

} // namespace

int main() {
    testDiscoveryHeartbeatAndRecovery();
    testFifoAndControlBounds();
    testTransportAndReadyErrors();
    std::cout << "FpgaService tests passed\n";
    return 0;
}
