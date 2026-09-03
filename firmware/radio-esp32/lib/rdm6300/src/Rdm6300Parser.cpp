#include "Rdm6300Parser.hpp"

namespace obsidia::radio {

namespace {

bool hexDigit(std::uint8_t character, std::uint8_t &value) {
    if (character >= '0' && character <= '9') {
        value = static_cast<std::uint8_t>(character - '0');
        return true;
    }
    if (character >= 'A' && character <= 'F') {
        value = static_cast<std::uint8_t>(character - 'A' + 10);
        return true;
    }
    if (character >= 'a' && character <= 'f') {
        value = static_cast<std::uint8_t>(character - 'a' + 10);
        return true;
    }
    return false;
}

} // namespace

bool Rdm6300Parser::hexByte(const std::uint8_t *characters, std::uint8_t &value) {
    std::uint8_t high = 0;
    std::uint8_t low = 0;
    if (!hexDigit(characters[0], high) || !hexDigit(characters[1], low)) return false;
    value = static_cast<std::uint8_t>((high << 4U) | low);
    return true;
}

void Rdm6300Parser::reset() {
    position_ = 0;
    lastByteAtMs_ = 0;
}

bool Rdm6300Parser::timedOut(std::uint32_t nowMs) const {
    return position_ != 0 &&
           static_cast<std::uint32_t>(nowMs - lastByteAtMs_) > kInterByteTimeoutMs;
}

Rdm6300Parser::Event Rdm6300Parser::tick(std::uint32_t nowMs, Error &error) {
    error = Error::None;
    if (!timedOut(nowMs)) return Event::None;
    reset();
    error = Error::Timeout;
    return Event::Error;
}

Rdm6300Parser::Event Rdm6300Parser::finish(Tag &tag, Error &error) {
    if (frame_[kFrameLength - 1] != kEnd) {
        error = Error::InvalidEnd;
        reset();
        return Event::Error;
    }

    std::array<std::uint8_t, 5> payload{};
    for (std::size_t index = 0; index < payload.size(); ++index) {
        if (!hexByte(&frame_[1 + index * 2], payload[index])) {
            error = Error::InvalidHex;
            reset();
            return Event::Error;
        }
    }
    std::uint8_t receivedChecksum = 0;
    if (!hexByte(&frame_[11], receivedChecksum)) {
        error = Error::InvalidHex;
        reset();
        return Event::Error;
    }
    std::uint8_t calculatedChecksum = payload[0];
    for (std::size_t index = 1; index < payload.size(); ++index)
        calculatedChecksum ^= payload[index];
    if (calculatedChecksum != receivedChecksum) {
        error = Error::BadChecksum;
        reset();
        return Event::Error;
    }

    tag.version = payload[0];
    tag.id = 0;
    for (std::size_t index = 1; index < payload.size(); ++index)
        tag.id = (tag.id << 8U) | payload[index];
    tag.checksum = receivedChecksum;
    reset();
    return Event::TagReady;
}

Rdm6300Parser::Event Rdm6300Parser::feed(
    std::uint8_t byte,
    std::uint32_t nowMs,
    Tag &tag,
    Error &error
) {
    error = Error::None;
    if (timedOut(nowMs)) {
        reset();
        if (byte == kStart) {
            frame_[0] = byte;
            position_ = 1;
            lastByteAtMs_ = nowMs;
        }
        error = Error::Timeout;
        return Event::Error;
    }

    if (position_ == 0) {
        if (byte == kStart) {
            frame_[0] = byte;
            position_ = 1;
            lastByteAtMs_ = nowMs;
        }
        return Event::None;
    }

    if (byte == kStart) {
        frame_[0] = byte;
        position_ = 1;
        lastByteAtMs_ = nowMs;
        return Event::None;
    }

    frame_[position_++] = byte;
    lastByteAtMs_ = nowMs;
    if (position_ == kFrameLength) return finish(tag, error);
    return Event::None;
}

} // namespace obsidia::radio
