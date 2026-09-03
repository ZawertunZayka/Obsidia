#pragma once

#include "obsidia_link.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia {

class RadioTransport {
public:
    virtual ~RadioTransport() = default;
    virtual bool write(const std::uint8_t *data, std::size_t length) = 0;
    virtual bool read(std::uint8_t &byte) = 0;
};

class RadioService {
public:
    enum class Health : std::uint8_t { Offline, Discovering, Ready, Degraded };
    enum class Error : std::uint8_t {
        None,
        TransportWrite,
        Timeout,
        MalformedFrame,
        UnexpectedResponse,
        ProtocolMismatch,
        PeerError,
    };

    struct Snapshot {
        Health health{Health::Offline};
        Error error{Error::None};
        std::uint8_t protocolVersion{0};
        std::array<std::uint8_t, 3> firmwareVersion{};
        std::uint8_t readyMask{0};
        std::uint8_t supportedMask{0};
        std::uint32_t successfulDiscoveries{0};
        std::uint32_t linkFailures{0};
    };

    explicit RadioService(RadioTransport &transport);

    void begin(std::uint32_t nowMs);
    void poll(std::uint32_t nowMs);
    const Snapshot &snapshot() const { return snapshot_; }

private:
    enum class Phase : std::uint8_t { Idle, Ping, Version, Status };

    static constexpr std::uint32_t kResponseTimeoutMs = 250;
    static constexpr std::uint32_t kHeartbeatIntervalMs = 2000;
    static constexpr std::uint32_t kRediscoveryIntervalMs = 1000;
    static constexpr std::uint8_t kMaxAttempts = 3;
    static constexpr std::size_t kMaxRxBytesPerPoll = 128;

    bool send(Phase phase, std::uint32_t nowMs, bool resetAttempts);
    void handleFrame(const link::Frame &frame, std::uint32_t nowMs);
    void fail(Error error, std::uint32_t nowMs);
    static link::Command commandFor(Phase phase);
    static bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval);

    RadioTransport &transport_;
    link::Parser parser_;
    Snapshot snapshot_{};
    Phase phase_{Phase::Idle};
    std::array<std::uint8_t, link::kMaxFrameSize> txBuffer_{};
    std::uint32_t requestStartedAtMs_{0};
    std::uint32_t lastActivityAtMs_{0};
    std::uint8_t attempts_{0};
};

} // namespace obsidia
