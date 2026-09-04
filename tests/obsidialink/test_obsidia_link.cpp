#include "obsidia_link.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

using namespace obsidia::link;

namespace {

struct Result {
    ParseEvent event{ParseEvent::None};
    ParseError error{ParseError::None};
    Frame frame{};
};

Result feedAll(Parser &parser, const std::uint8_t *bytes, std::size_t length, std::uint32_t start = 0) {
    Result result;
    for (std::size_t i = 0; i < length; ++i) {
        const auto event = parser.feed(bytes[i], start + static_cast<std::uint32_t>(i), result.frame, result.error);
        if (event != ParseEvent::None) result.event = event;
    }
    return result;
}

void testCrcVector() {
    constexpr std::uint8_t text[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    assert(crc16CcittFalse(text, sizeof(text)) == 0x29B1);
}

void testRoundTripAndFragmentation() {
    const std::uint8_t payload[] = {0x00, 0xAA, 0xFF, 0x42};
    std::array<std::uint8_t, kMaxFrameSize> bytes{};
    std::size_t length = 0;
    assert(encode(Command::GetStatus, payload, sizeof(payload), bytes.data(), bytes.size(), length));

    Parser parser;
    Frame frame;
    ParseError error;
    for (std::size_t i = 0; i + 1 < length; ++i) {
        assert(parser.feed(bytes[i], static_cast<std::uint32_t>(i), frame, error) == ParseEvent::None);
    }
    assert(parser.feed(bytes[length - 1], static_cast<std::uint32_t>(length), frame, error) ==
           ParseEvent::FrameReady);
    assert(frame.command == Command::GetStatus);
    assert(frame.payloadLength == sizeof(payload));
    assert(std::memcmp(frame.payload.data(), payload, sizeof(payload)) == 0);
}

void testConcatenatedFrames() {
    std::array<std::uint8_t, kMaxFrameSize> ping{};
    std::array<std::uint8_t, kMaxFrameSize> version{};
    std::size_t pingLength = 0;
    std::size_t versionLength = 0;
    assert(encode(Command::Ping, nullptr, 0, ping.data(), ping.size(), pingLength));
    assert(encode(Command::GetVersion, nullptr, 0, version.data(), version.size(), versionLength));

    Parser parser;
    int frames = 0;
    Frame frame;
    ParseError error;
    for (std::size_t i = 0; i < pingLength; ++i) {
        if (parser.feed(ping[i], static_cast<std::uint32_t>(i), frame, error) == ParseEvent::FrameReady) {
            assert(frame.command == Command::Ping);
            ++frames;
        }
    }
    for (std::size_t i = 0; i < versionLength; ++i) {
        if (parser.feed(version[i], static_cast<std::uint32_t>(pingLength + i), frame, error) ==
            ParseEvent::FrameReady) {
            assert(frame.command == Command::GetVersion);
            ++frames;
        }
    }
    assert(frames == 2);
}

void testBadCrcAndNoiseResync() {
    std::array<std::uint8_t, kMaxFrameSize> bytes{};
    std::size_t length = 0;
    assert(encode(Command::Ping, nullptr, 0, bytes.data(), bytes.size(), length));
    bytes[length - 1] ^= 0x01;

    Parser parser;
    const std::uint8_t noise[] = {0x00, 0x55, 0xFE};
    Result ignored = feedAll(parser, noise, sizeof(noise));
    assert(ignored.event == ParseEvent::None);
    Result bad = feedAll(parser, bytes.data(), length, 10);
    assert(bad.event == ParseEvent::Error);
    assert(bad.error == ParseError::BadCrc);

    assert(encode(Command::Ping, nullptr, 0, bytes.data(), bytes.size(), length));
    Result good = feedAll(parser, bytes.data(), length, 30);
    assert(good.event == ParseEvent::FrameReady);
    assert(good.frame.command == Command::Ping);
}

void testInvalidLengthAndRepeatedSof() {
    Parser parser;
    Frame frame;
    ParseError error;
    assert(parser.feed(kSof, 0, frame, error) == ParseEvent::None);
    assert(parser.feed(0, 1, frame, error) == ParseEvent::Error);
    assert(error == ParseError::InvalidLength);
    assert(parser.feed(kSof, 2, frame, error) == ParseEvent::None);
    assert(parser.feed(kSof, 3, frame, error) == ParseEvent::Error);
    assert(error == ParseError::InvalidLength);

    std::array<std::uint8_t, kMaxFrameSize> bytes{};
    std::size_t length = 0;
    assert(encode(Command::Ping, nullptr, 0, bytes.data(), bytes.size(), length));
    Result result = feedAll(parser, bytes.data() + 1, length - 1, 4);
    assert(result.event == ParseEvent::FrameReady);
}

void testTimeout() {
    Parser parser(10);
    Frame frame;
    ParseError error;
    assert(parser.feed(kSof, 100, frame, error) == ParseEvent::None);
    assert(parser.feed(1, 105, frame, error) == ParseEvent::None);
    assert(parser.tick(116, error) == ParseEvent::Error);
    assert(error == ParseError::Timeout);

    std::array<std::uint8_t, kMaxFrameSize> bytes{};
    std::size_t length = 0;
    assert(encode(Command::GetVersion, nullptr, 0, bytes.data(), bytes.size(), length));
    Result result = feedAll(parser, bytes.data(), length, 120);
    assert(result.event == ParseEvent::FrameReady);
}

void testBoundsAndCommandClasses() {
    std::array<std::uint8_t, kMaxPayloadSize + 1> payload{};
    std::array<std::uint8_t, kMaxFrameSize> output{};
    std::size_t length = 123;
    assert(!encode(Command::Ping, payload.data(), payload.size(), output.data(), output.size(), length));
    assert(length == 0);
    assert(!encode(Command::Ping, nullptr, 1, output.data(), output.size(), length));
    assert(!encode(Command::Ping, nullptr, 0, output.data(), 4, length));
    assert(isRequest(Command::Reset));
    assert(!isRequest(Command::Data));
    assert(isResponse(Command::Error));
    assert(errorCodeFor(ParseError::BadCrc) == ErrorCode::BadCrc);
    assert(errorCodeFor(ParseError::InvalidLength) == ErrorCode::MalformedFrame);
    assert(errorCodeFor(ParseError::Timeout) == ErrorCode::MalformedFrame);
}

} // namespace

int main() {
    testCrcVector();
    testRoundTripAndFragmentation();
    testConcatenatedFrames();
    testBadCrcAndNoiseResync();
    testInvalidLengthAndRepeatedSof();
    testTimeout();
    testBoundsAndCommandClasses();
    std::cout << "ObsidiaLink tests passed\n";
    return 0;
}
