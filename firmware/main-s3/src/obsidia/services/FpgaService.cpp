#include "FpgaService.hpp"

namespace obsidia {

FpgaService::FpgaService(FpgaTransport &transport) : transport_(transport) {}

bool FpgaService::elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval) {
    return static_cast<std::uint32_t>(now - since) >= interval;
}

void FpgaService::fail(Error error, std::uint32_t nowMs) {
    snapshot_.health = Health::Offline;
    snapshot_.error = error;
    snapshot_.status = 0;
    snapshot_.fifoLevel = 0;
    ++snapshot_.failures;
    lastProbeAtMs_ = nowMs;
}

bool FpgaService::discover(std::uint32_t nowMs) {
    std::array<std::uint8_t, 4> identity{};
    if (!transport_.read(kRegisterDeviceId, identity.data(), identity.size())) {
        fail(Error::Transport, nowMs);
        return false;
    }
    constexpr std::array<std::uint8_t, 4> expected{{'O', 'B', 'S', 'D'}};
    if (identity != expected) {
        fail(Error::IdentityMismatch, nowMs);
        return false;
    }
    if (!transport_.read(kRegisterVersion, snapshot_.version.data(), snapshot_.version.size())) {
        fail(Error::Transport, nowMs);
        return false;
    }
    if (!refreshStatus(nowMs)) return false;
    ++snapshot_.successfulDiscoveries;
    return true;
}

bool FpgaService::refreshStatus(std::uint32_t nowMs) {
    std::uint8_t status = 0;
    if (!transport_.read(kRegisterStatus, &status, 1)) {
        fail(Error::Transport, nowMs);
        return false;
    }
    snapshot_.status = status;
    lastProbeAtMs_ = nowMs;
    if ((status & kStatusReady) == 0) {
        fail(Error::NotReady, nowMs);
        return false;
    }
    snapshot_.health = Health::Ready;
    snapshot_.error = Error::None;
    return true;
}

bool FpgaService::begin(std::uint32_t nowMs) {
    return discover(nowMs);
}

void FpgaService::poll(std::uint32_t nowMs, bool irqAsserted) {
    if (snapshot_.health == Health::Offline) {
        if (elapsed(nowMs, lastProbeAtMs_, kRediscoveryIntervalMs)) discover(nowMs);
        return;
    }
    if (irqAsserted || elapsed(nowMs, lastProbeAtMs_, kHeartbeatIntervalMs)) {
        // Identity is revalidated on every heartbeat so an FPGA reset or a bus
        // responder mismatch cannot retain stale READY state.
        discover(nowMs);
    }
}

bool FpgaService::acknowledgeEvents() {
    if (!transport_.write(kRegisterControl, kControlAcknowledge)) {
        fail(Error::Transport, lastProbeAtMs_);
        return false;
    }
    return true;
}

bool FpgaService::clearFifo() {
    if (!transport_.write(kRegisterControl, kControlFifoClear)) {
        fail(Error::Transport, lastProbeAtMs_);
        return false;
    }
    snapshot_.fifoLevel = 0;
    return true;
}

bool FpgaService::readFifo(
    std::uint8_t *output,
    std::size_t outputCapacity,
    std::size_t &outputLength
) {
    outputLength = 0;
    if (output == nullptr || outputCapacity == 0) {
        snapshot_.error = Error::InvalidArgument;
        return false;
    }

    std::uint8_t level = 0;
    if (!transport_.read(kRegisterFifoLevel, &level, 1)) {
        fail(Error::Transport, lastProbeAtMs_);
        return false;
    }
    if (level > kMaxFifoRead) level = static_cast<std::uint8_t>(kMaxFifoRead);
    const auto requested = level < outputCapacity ? static_cast<std::size_t>(level) : outputCapacity;
    snapshot_.fifoLevel = level;
    if (requested == 0) return true;
    if (!transport_.read(kRegisterFifoData, output, requested)) {
        fail(Error::Transport, lastProbeAtMs_);
        return false;
    }
    outputLength = requested;
    snapshot_.fifoLevel = static_cast<std::uint8_t>(level - requested);
    snapshot_.error = Error::None;
    return true;
}

} // namespace obsidia
