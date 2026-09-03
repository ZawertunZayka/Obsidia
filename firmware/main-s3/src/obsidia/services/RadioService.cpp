#include "RadioService.hpp"

namespace obsidia {

RadioService::RadioService(RadioTransport &transport) : transport_(transport) {}

bool RadioService::elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t interval) {
    return static_cast<std::uint32_t>(now - since) >= interval;
}

link::Command RadioService::commandFor(Phase phase) {
    switch (phase) {
    case Phase::Ping: return link::Command::Ping;
    case Phase::Version: return link::Command::GetVersion;
    case Phase::Status: return link::Command::GetStatus;
    case Phase::Idle: return link::Command::Ping;
    }
    return link::Command::Ping;
}

bool RadioService::send(Phase phase, std::uint32_t nowMs, bool resetAttempts) {
    std::size_t encodedLength = 0;
    if (!link::encode(
            commandFor(phase), nullptr, 0, txBuffer_.data(), txBuffer_.size(), encodedLength
        ) || !transport_.write(txBuffer_.data(), encodedLength)) {
        fail(Error::TransportWrite, nowMs);
        return false;
    }

    phase_ = phase;
    requestStartedAtMs_ = nowMs;
    lastActivityAtMs_ = nowMs;
    if (resetAttempts) attempts_ = 1;
    return true;
}

void RadioService::begin(std::uint32_t nowMs) {
    parser_.reset();
    snapshot_.health = Health::Discovering;
    snapshot_.error = Error::None;
    send(Phase::Ping, nowMs, true);
}

void RadioService::fail(Error error, std::uint32_t nowMs) {
    snapshot_.health = Health::Offline;
    snapshot_.error = error;
    ++snapshot_.linkFailures;
    phase_ = Phase::Idle;
    attempts_ = 0;
    lastActivityAtMs_ = nowMs;
    parser_.reset();
}

void RadioService::handleFrame(const link::Frame &frame, std::uint32_t nowMs) {
    if (frame.command == link::Command::Error) {
        fail(Error::PeerError, nowMs);
        return;
    }

    const auto expectedRequest = static_cast<std::uint8_t>(commandFor(phase_));
    if (phase_ == Phase::Idle || frame.payloadLength < 2 ||
        frame.payload[0] != expectedRequest) {
        fail(Error::UnexpectedResponse, nowMs);
        return;
    }
    if (frame.payload[1] != link::kProtocolVersion) {
        fail(Error::ProtocolMismatch, nowMs);
        return;
    }

    switch (phase_) {
    case Phase::Ping:
        if (frame.command != link::Command::Ack || frame.payloadLength != 2) {
            fail(Error::UnexpectedResponse, nowMs);
            return;
        }
        send(Phase::Version, nowMs, true);
        return;

    case Phase::Version:
        if (frame.command != link::Command::Data || frame.payloadLength != 5) {
            fail(Error::UnexpectedResponse, nowMs);
            return;
        }
        snapshot_.protocolVersion = frame.payload[1];
        snapshot_.firmwareVersion = {frame.payload[2], frame.payload[3], frame.payload[4]};
        send(Phase::Status, nowMs, true);
        return;

    case Phase::Status:
        if (frame.command != link::Command::Data || frame.payloadLength != 4) {
            fail(Error::UnexpectedResponse, nowMs);
            return;
        }
        snapshot_.readyMask = frame.payload[2];
        snapshot_.supportedMask = frame.payload[3];
        snapshot_.health = Health::Ready;
        snapshot_.error = Error::None;
        ++snapshot_.successfulDiscoveries;
        phase_ = Phase::Idle;
        attempts_ = 0;
        lastActivityAtMs_ = nowMs;
        return;

    case Phase::Idle:
        fail(Error::UnexpectedResponse, nowMs);
        return;
    }
}

void RadioService::poll(std::uint32_t nowMs) {
    std::uint8_t byte = 0;
    link::Frame frame;
    link::ParseError parseError = link::ParseError::None;
    std::size_t consumed = 0;
    while (consumed < kMaxRxBytesPerPoll && transport_.read(byte)) {
        ++consumed;
        const auto event = parser_.feed(byte, nowMs, frame, parseError);
        if (event == link::ParseEvent::FrameReady) {
            handleFrame(frame, nowMs);
            if (snapshot_.health == Health::Offline) return;
        } else if (event == link::ParseEvent::Error) {
            snapshot_.error = Error::MalformedFrame;
            if (snapshot_.health == Health::Ready) snapshot_.health = Health::Degraded;
        }
    }

    if (parser_.tick(nowMs, parseError) == link::ParseEvent::Error) {
        snapshot_.error = Error::MalformedFrame;
        if (snapshot_.health == Health::Ready) snapshot_.health = Health::Degraded;
    }

    if (phase_ != Phase::Idle && elapsed(nowMs, requestStartedAtMs_, kResponseTimeoutMs)) {
        if (attempts_ < kMaxAttempts) {
            ++attempts_;
            send(phase_, nowMs, false);
        } else {
            fail(Error::Timeout, nowMs);
        }
        return;
    }

    if (phase_ == Phase::Idle) {
        const auto interval = snapshot_.health == Health::Offline ? kRediscoveryIntervalMs
                                                                  : kHeartbeatIntervalMs;
        if (elapsed(nowMs, lastActivityAtMs_, interval)) {
            if (snapshot_.health == Health::Offline) snapshot_.health = Health::Discovering;
            send(Phase::Ping, nowMs, true);
        }
    }
}

} // namespace obsidia
