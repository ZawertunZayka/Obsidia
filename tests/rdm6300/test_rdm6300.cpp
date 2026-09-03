#include "Rdm6300Parser.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

namespace {

using Parser = obsidia::radio::Rdm6300Parser;

struct Result {
    Parser::Event event{Parser::Event::None};
    Parser::Error error{Parser::Error::None};
    Parser::Tag tag{};
};

Result feed(Parser &parser, const std::uint8_t *data, std::size_t length, std::uint32_t start = 0) {
    Result result;
    for (std::size_t index = 0; index < length; ++index) {
        const auto event = parser.feed(
            data[index], start + static_cast<std::uint32_t>(index), result.tag, result.error
        );
        if (event != Parser::Event::None) result.event = event;
    }
    return result;
}

void testValidFrameAndNoise() {
    constexpr std::array<std::uint8_t, 3> noise{{0x00, 0x55, 0x03}};
    constexpr std::array<std::uint8_t, 14> frame{{
        0x02, '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '1', 0x03
    }};
    Parser parser;
    assert(feed(parser, noise.data(), noise.size()).event == Parser::Event::None);
    const auto result = feed(parser, frame.data(), frame.size(), 10);
    assert(result.event == Parser::Event::TagReady);
    assert(result.tag.version == 0x01);
    assert(result.tag.id == 0x02030405U);
    assert(result.tag.checksum == 0x01);
}

void testBadChecksumHexAndEnd() {
    std::array<std::uint8_t, 14> frame{{
        0x02, '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '0', 0x03
    }};
    Parser parser;
    auto result = feed(parser, frame.data(), frame.size());
    assert(result.event == Parser::Event::Error && result.error == Parser::Error::BadChecksum);

    frame[11] = 'Z';
    result = feed(parser, frame.data(), frame.size(), 20);
    assert(result.event == Parser::Event::Error && result.error == Parser::Error::InvalidHex);

    frame[11] = '0';
    frame[12] = '1';
    frame[13] = 0x04;
    result = feed(parser, frame.data(), frame.size(), 40);
    assert(result.event == Parser::Event::Error && result.error == Parser::Error::InvalidEnd);
}

void testResyncAndTimeout() {
    constexpr std::array<std::uint8_t, 14> frame{{
        0x02, '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '1', 0x03
    }};
    Parser parser;
    Parser::Tag tag;
    Parser::Error error;
    assert(parser.feed(0x02, 0, tag, error) == Parser::Event::None);
    assert(parser.feed('F', 1, tag, error) == Parser::Event::None);
    assert(parser.feed(0x02, 2, tag, error) == Parser::Event::None);
    const auto result = feed(parser, frame.data() + 1, frame.size() - 1, 3);
    assert(result.event == Parser::Event::TagReady);

    assert(parser.feed(0x02, 100, tag, error) == Parser::Event::None);
    assert(parser.tick(151, error) == Parser::Event::Error);
    assert(error == Parser::Error::Timeout);
}

} // namespace

int main() {
    testValidFrameAndNoise();
    testBadChecksumHexAndEnd();
    testResyncAndTimeout();
    std::cout << "RDM6300 parser tests passed\n";
    return 0;
}
