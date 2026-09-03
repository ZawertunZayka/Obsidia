#include "RadioService.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <iostream>
#include <vector>

namespace {

class MockTransport final : public obsidia::RadioTransport {
public:
    bool write(const std::uint8_t *data, std::size_t length) override {
        if (failWrites) return false;
        writes.emplace_back(data, data + length);
        return true;
    }

    bool read(std::uint8_t &byte) override {
        if (rx.empty()) return false;
        byte = rx.front();
        rx.pop_front();
        return true;
    }

    void inject(obsidia::link::Command command, const std::uint8_t *payload, std::size_t length) {
        std::array<std::uint8_t, obsidia::link::kMaxFrameSize> frame{};
        std::size_t encodedLength = 0;
        assert(obsidia::link::encode(command, payload, length, frame.data(), frame.size(), encodedLength));
        rx.insert(rx.end(), frame.begin(), frame.begin() + static_cast<std::ptrdiff_t>(encodedLength));
    }

    obsidia::link::Command lastCommand() const {
        assert(!writes.empty() && writes.back().size() >= 3);
        return static_cast<obsidia::link::Command>(writes.back()[2]);
    }

    bool failWrites{false};
    std::deque<std::uint8_t> rx;
    std::vector<std::vector<std::uint8_t>> writes;
};

void completeDiscovery(MockTransport &transport, obsidia::RadioService &service, std::uint32_t now) {
    const std::uint8_t ack[] = {0x01, obsidia::link::kProtocolVersion};
    transport.inject(obsidia::link::Command::Ack, ack, sizeof(ack));
    service.poll(now++);
    assert(transport.lastCommand() == obsidia::link::Command::GetVersion);

    const std::uint8_t version[] = {0x02, obsidia::link::kProtocolVersion, 0, 1, 0};
    transport.inject(obsidia::link::Command::Data, version, sizeof(version));
    service.poll(now++);
    assert(transport.lastCommand() == obsidia::link::Command::GetStatus);

    const std::uint8_t status[] = {0x03, obsidia::link::kProtocolVersion, 0x05, 0x07};
    transport.inject(obsidia::link::Command::Data, status, sizeof(status));
    service.poll(now);
}

void testDiscoveryAndHeartbeat() {
    MockTransport transport;
    obsidia::RadioService service(transport);
    service.begin(0);
    assert(transport.lastCommand() == obsidia::link::Command::Ping);
    completeDiscovery(transport, service, 1);

    const auto &snapshot = service.snapshot();
    assert(snapshot.health == obsidia::RadioService::Health::Ready);
    assert(snapshot.firmwareVersion[1] == 1);
    assert(snapshot.readyMask == 0x05);
    assert(snapshot.supportedMask == 0x07);
    assert(snapshot.successfulDiscoveries == 1);

    service.poll(2003);
    assert(transport.lastCommand() == obsidia::link::Command::Ping);
}

void testTimeoutRetriesAndRecovery() {
    MockTransport transport;
    obsidia::RadioService service(transport);
    service.begin(100);
    service.poll(350);
    service.poll(600);
    assert(transport.writes.size() == 3);
    service.poll(850);
    assert(service.snapshot().health == obsidia::RadioService::Health::Offline);
    assert(service.snapshot().error == obsidia::RadioService::Error::Timeout);
    assert(service.snapshot().linkFailures == 1);

    service.poll(1850);
    assert(service.snapshot().health == obsidia::RadioService::Health::Discovering);
    assert(transport.writes.size() == 4);
    assert(transport.lastCommand() == obsidia::link::Command::Ping);
}

void testProtocolMismatchAndWriteFailure() {
    MockTransport transport;
    obsidia::RadioService service(transport);
    service.begin(0);
    const std::uint8_t wrongVersion[] = {0x01, 99};
    transport.inject(obsidia::link::Command::Ack, wrongVersion, sizeof(wrongVersion));
    service.poll(1);
    assert(service.snapshot().error == obsidia::RadioService::Error::ProtocolMismatch);

    MockTransport broken;
    broken.failWrites = true;
    obsidia::RadioService brokenService(broken);
    brokenService.begin(0);
    assert(brokenService.snapshot().health == obsidia::RadioService::Health::Offline);
    assert(brokenService.snapshot().error == obsidia::RadioService::Error::TransportWrite);
}

} // namespace

int main() {
    testDiscoveryAndHeartbeat();
    testTimeoutRetriesAndRecovery();
    testProtocolMismatchAndWriteFailure();
    std::cout << "RadioService tests passed\n";
    return 0;
}
