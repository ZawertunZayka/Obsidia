#include "obsidia_link.hpp"

namespace obsidia::link {

std::uint16_t crc16CcittFalse(const std::uint8_t *data, std::size_t length) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= static_cast<std::uint16_t>(data[i]) << 8;
        for (std::uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000U) ? static_cast<std::uint16_t>((crc << 1U) ^ 0x1021U)
                                  : static_cast<std::uint16_t>(crc << 1U);
        }
    }
    return crc;
}

bool encode(
    Command command,
    const std::uint8_t *payload,
    std::size_t payloadLength,
    std::uint8_t *output,
    std::size_t outputCapacity,
    std::size_t &outputLength
) {
    outputLength = 0;
    if (payloadLength > kMaxPayloadSize || output == nullptr ||
        (payloadLength != 0 && payload == nullptr)) {
        return false;
    }

    const std::size_t encodedLength = payloadLength + 5;
    if (outputCapacity < encodedLength) return false;

    output[0] = kSof;
    output[1] = static_cast<std::uint8_t>(payloadLength + 1);
    output[2] = static_cast<std::uint8_t>(command);
    for (std::size_t i = 0; i < payloadLength; ++i) output[3 + i] = payload[i];

    const std::uint16_t crc = crc16CcittFalse(output + 1, payloadLength + 2);
    output[3 + payloadLength] = static_cast<std::uint8_t>(crc & 0xFFU);
    output[4 + payloadLength] = static_cast<std::uint8_t>(crc >> 8U);
    outputLength = encodedLength;
    return true;
}

bool isRequest(Command command) {
    return command == Command::Ping || command == Command::GetVersion ||
           command == Command::GetStatus || command == Command::Reset;
}

bool isResponse(Command command) {
    return command == Command::Ack || command == Command::Error || command == Command::Data;
}

ErrorCode errorCodeFor(ParseError error) {
    return error == ParseError::BadCrc ? ErrorCode::BadCrc : ErrorCode::MalformedFrame;
}

Parser::Parser(std::uint32_t interByteTimeoutMs) : timeoutMs_(interByteTimeoutMs) {}

void Parser::reset() {
    state_ = State::SeekSof;
    bodyLength_ = 0;
    bodyIndex_ = 0;
    receivedCrc_ = 0;
    lastByteAtMs_ = 0;
}

bool Parser::timedOut(std::uint32_t nowMs) const {
    return state_ != State::SeekSof &&
           static_cast<std::uint32_t>(nowMs - lastByteAtMs_) > timeoutMs_;
}

void Parser::restartFrom(std::uint8_t byte) {
    reset();
    if (byte == kSof) state_ = State::ReadLength;
}

ParseEvent Parser::tick(std::uint32_t nowMs, ParseError &error) {
    error = ParseError::None;
    if (!timedOut(nowMs)) return ParseEvent::None;
    reset();
    error = ParseError::Timeout;
    return ParseEvent::Error;
}

ParseEvent Parser::feed(std::uint8_t byte, std::uint32_t nowMs, Frame &frame, ParseError &error) {
    error = ParseError::None;
    if (timedOut(nowMs)) {
        restartFrom(byte);
        lastByteAtMs_ = nowMs;
        error = ParseError::Timeout;
        return ParseEvent::Error;
    }

    lastByteAtMs_ = nowMs;
    switch (state_) {
    case State::SeekSof:
        if (byte == kSof) state_ = State::ReadLength;
        return ParseEvent::None;

    case State::ReadLength:
        if (byte == 0 || byte > kMaxBodySize) {
            restartFrom(byte);
            error = ParseError::InvalidLength;
            return ParseEvent::Error;
        }
        bodyLength_ = byte;
        bodyIndex_ = 0;
        state_ = State::ReadBody;
        return ParseEvent::None;

    case State::ReadBody:
        body_[bodyIndex_++] = byte;
        if (bodyIndex_ == bodyLength_) state_ = State::ReadCrcLow;
        return ParseEvent::None;

    case State::ReadCrcLow:
        receivedCrc_ = byte;
        state_ = State::ReadCrcHigh;
        return ParseEvent::None;

    case State::ReadCrcHigh: {
        receivedCrc_ |= static_cast<std::uint16_t>(byte) << 8U;
        std::array<std::uint8_t, 1 + kMaxBodySize> crcInput{};
        crcInput[0] = bodyLength_;
        for (std::uint8_t i = 0; i < bodyLength_; ++i) crcInput[1 + i] = body_[i];
        const std::uint16_t expected = crc16CcittFalse(crcInput.data(), bodyLength_ + 1);
        if (receivedCrc_ != expected) {
            restartFrom(byte);
            error = ParseError::BadCrc;
            return ParseEvent::Error;
        }

        frame.command = static_cast<Command>(body_[0]);
        frame.payloadLength = static_cast<std::uint8_t>(bodyLength_ - 1);
        for (std::uint8_t i = 0; i < frame.payloadLength; ++i) frame.payload[i] = body_[1 + i];
        reset();
        return ParseEvent::FrameReady;
    }
    }
    return ParseEvent::None;
}

} // namespace obsidia::link
