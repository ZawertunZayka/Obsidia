#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia {

class FpgaTransport {
public:
    virtual ~FpgaTransport() = default;
    virtual bool read(std::uint8_t address, std::uint8_t *data, std::size_t length) = 0;
    virtual bool write(std::uint8_t address, std::uint8_t value) = 0;
};

class FpgaService {
public:
    enum class Health : std::uint8_t { Offline, Ready, Degraded };
    enum class Error : std::uint8_t {
        None,
        Transport,
        IdentityMismatch,
        NotReady,
        InvalidArgument,
    };

    struct Snapshot {
        Health health{Health::Offline};
        Error error{Error::None};
        std::array<std::uint8_t, 2> version{};
        std::uint8_t status{0};
        std::uint8_t fifoLevel{0};
        std::uint32_t successfulDiscoveries{0};
        std::uint32_t failures{0};
    };

    static constexpr std::size_t kMaxFifoRead = 64;

    explicit FpgaService(FpgaTransport &transport);

    bool begin(std::uint32_t nowMs);
    void poll(std::uint32_t nowMs, bool irqAsserted = false);
    bool acknowledgeEvents();
    bool clearFifo();
    bool readFifo(
        std::uint8_t *output,
        std::size_t outputCapacity,
        std::size_t &outputLength
    );
    const Snapshot &snapshot() const { return snapshot_; }

private:
    static constexpr std::uint8_t kRegisterDeviceId = 0x00;
    static constexpr std::uint8_t kRegisterVersion = 0x04;
    static constexpr std::uint8_t kRegisterStatus = 0x06;
    static constexpr std::uint8_t kRegisterControl = 0x07;
    static constexpr std::uint8_t kRegisterFifoData = 0x08;
    static constexpr std::uint8_t kRegisterFifoLevel = 0x09;
    static constexpr std::uint8_t kStatusReady = 1U << 0U;
    static constexpr std::uint8_t kControlAcknowledge = 1U << 0U;
    static constexpr std::uint8_t kControlFifoClear = 1U << 1U;
    static constexpr std::uint32_t kHeartbeatIntervalMs = 1000;
    static constexpr std::uint32_t kRediscoveryIntervalMs = 500;

    bool discover(std::uint32_t nowMs);
    bool refreshStatus(std::uint32_t nowMs);
    void fail(Error error, std::uint32_t nowMs);
    static bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval);

    FpgaTransport &transport_;
    Snapshot snapshot_{};
    std::uint32_t lastProbeAtMs_{0};
};

} // namespace obsidia
