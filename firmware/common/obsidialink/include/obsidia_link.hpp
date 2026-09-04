#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia::link {

constexpr std::uint8_t kSof = 0xAA;
constexpr std::uint8_t kProtocolVersion = 1;
constexpr std::size_t kMaxPayloadSize = 64;
constexpr std::size_t kMaxBodySize = 1 + kMaxPayloadSize;
constexpr std::size_t kMaxFrameSize = 1 + 1 + kMaxBodySize + 2;
constexpr std::uint32_t kDefaultInterByteTimeoutMs = 100;

enum class Command : std::uint8_t {
    Ping = 0x01,
    GetVersion = 0x02,
    GetStatus = 0x03,
    Reset = 0x04,
    Ack = 0x80,
    Error = 0x81,
    Data = 0x82,
};

enum class ErrorCode : std::uint8_t {
    UnsupportedCommand = 0x01,
    InvalidPayload = 0x02,
    BadCrc = 0x03,
    MalformedFrame = 0x04,
    Busy = 0x05,
    PeripheralUnavailable = 0x06,
    InternalFailure = 0x07,
};

enum class ParseError : std::uint8_t {
    None,
    InvalidLength,
    BadCrc,
    Timeout,
};

enum class ParseEvent : std::uint8_t { None, FrameReady, Error };

struct Frame {
    Command command{Command::Error};
    std::uint8_t payloadLength{0};
    std::array<std::uint8_t, kMaxPayloadSize> payload{};
};

std::uint16_t crc16CcittFalse(const std::uint8_t *data, std::size_t length);

bool encode(
    Command command,
    const std::uint8_t *payload,
    std::size_t payloadLength,
    std::uint8_t *output,
    std::size_t outputCapacity,
    std::size_t &outputLength
);

bool isRequest(Command command);
bool isResponse(Command command);
ErrorCode errorCodeFor(ParseError error);

class Parser {
public:
    explicit Parser(std::uint32_t interByteTimeoutMs = kDefaultInterByteTimeoutMs);

    ParseEvent feed(std::uint8_t byte, std::uint32_t nowMs, Frame &frame, ParseError &error);
    ParseEvent tick(std::uint32_t nowMs, ParseError &error);
    void reset();

private:
    enum class State : std::uint8_t { SeekSof, ReadLength, ReadBody, ReadCrcLow, ReadCrcHigh };

    bool timedOut(std::uint32_t nowMs) const;
    void restartFrom(std::uint8_t byte);

    State state_{State::SeekSof};
    std::uint8_t bodyLength_{0};
    std::uint8_t bodyIndex_{0};
    std::array<std::uint8_t, kMaxBodySize> body_{};
    std::uint16_t receivedCrc_{0};
    std::uint32_t lastByteAtMs_{0};
    std::uint32_t timeoutMs_;
};

} // namespace obsidia::link
