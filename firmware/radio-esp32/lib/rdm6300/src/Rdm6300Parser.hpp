#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia::radio {

class Rdm6300Parser {
public:
    static constexpr std::uint8_t kStart = 0x02;
    static constexpr std::uint8_t kEnd = 0x03;
    static constexpr std::size_t kFrameLength = 14;
    static constexpr std::uint32_t kInterByteTimeoutMs = 50;

    enum class Event : std::uint8_t { None, TagReady, Error };
    enum class Error : std::uint8_t { None, InvalidEnd, InvalidHex, BadChecksum, Timeout };

    struct Tag {
        std::uint8_t version{0};
        std::uint32_t id{0};
        std::uint8_t checksum{0};
    };

    Event feed(std::uint8_t byte, std::uint32_t nowMs, Tag &tag, Error &error);
    Event tick(std::uint32_t nowMs, Error &error);
    void reset();

private:
    static bool hexByte(const std::uint8_t *characters, std::uint8_t &value);
    bool timedOut(std::uint32_t nowMs) const;
    Event finish(Tag &tag, Error &error);

    std::array<std::uint8_t, kFrameLength> frame_{};
    std::size_t position_{0};
    std::uint32_t lastByteAtMs_{0};
};

} // namespace obsidia::radio
