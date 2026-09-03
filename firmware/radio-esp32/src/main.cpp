#include <Arduino.h>

#include "obsidia_link.hpp"

#ifndef OBSIDIA_RADIO_RX_GPIO
#define OBSIDIA_RADIO_RX_GPIO -1
#endif
#ifndef OBSIDIA_RADIO_TX_GPIO
#define OBSIDIA_RADIO_TX_GPIO -1
#endif

namespace {

using obsidia::link::Command;
using obsidia::link::ErrorCode;
using obsidia::link::Frame;
using obsidia::link::ParseError;
using obsidia::link::ParseEvent;

constexpr std::uint32_t kLinkBaud = 115200;
constexpr std::uint8_t kFirmwareMajor = 0;
constexpr std::uint8_t kFirmwareMinor = 1;
constexpr std::uint8_t kFirmwarePatch = 0;
constexpr std::uint32_t kResetDelayMs = 100;

// GET_STATUS flags. A bit is set only after its standalone hardware diagnostic
// has passed during this boot.
constexpr std::uint8_t kStatusRdm6300 = 1U << 0U;
constexpr std::uint8_t kStatusCc1101 = 1U << 1U;
constexpr std::uint8_t kStatusNrf24 = 1U << 2U;

HardwareSerial linkSerial(2);
obsidia::link::Parser parser;
std::array<std::uint8_t, obsidia::link::kMaxFrameSize> txBuffer{};
bool linkConfigured = false;
bool resetPending = false;
std::uint32_t resetAtMs = 0;
std::uint8_t peripheralStatus = 0;

void sendFrame(Command command, const std::uint8_t *payload, std::size_t payloadLength) {
    std::size_t encodedLength = 0;
    if (!obsidia::link::encode(
            command, payload, payloadLength, txBuffer.data(), txBuffer.size(), encodedLength
        )) {
        Serial.println("[ERROR] ObsidiaLink response encoding failed");
        return;
    }
    linkSerial.write(txBuffer.data(), encodedLength);
}

void sendError(std::uint8_t requestCommand, ErrorCode error) {
    const std::uint8_t payload[] = {
        requestCommand,
        obsidia::link::kProtocolVersion,
        static_cast<std::uint8_t>(error),
    };
    sendFrame(Command::Error, payload, sizeof(payload));
}

void handleRequest(const Frame &frame) {
    const auto request = static_cast<std::uint8_t>(frame.command);
    if (!obsidia::link::isRequest(frame.command)) {
        sendError(request, ErrorCode::UnsupportedCommand);
        return;
    }
    if (frame.payloadLength != 0) {
        sendError(request, ErrorCode::InvalidPayload);
        return;
    }

    switch (frame.command) {
    case Command::Ping: {
        const std::uint8_t payload[] = {request, obsidia::link::kProtocolVersion};
        sendFrame(Command::Ack, payload, sizeof(payload));
        break;
    }
    case Command::GetVersion: {
        const std::uint8_t payload[] = {
            request,
            obsidia::link::kProtocolVersion,
            kFirmwareMajor,
            kFirmwareMinor,
            kFirmwarePatch,
        };
        sendFrame(Command::Data, payload, sizeof(payload));
        break;
    }
    case Command::GetStatus: {
        const std::uint8_t payload[] = {
            request,
            obsidia::link::kProtocolVersion,
            peripheralStatus,
            static_cast<std::uint8_t>(
                kStatusRdm6300 | kStatusCc1101 | kStatusNrf24
            ),
        };
        sendFrame(Command::Data, payload, sizeof(payload));
        break;
    }
    case Command::Reset: {
        const std::uint8_t payload[] = {request, obsidia::link::kProtocolVersion};
        sendFrame(Command::Ack, payload, sizeof(payload));
        linkSerial.flush();
        resetPending = true;
        resetAtMs = millis() + kResetDelayMs;
        break;
    }
    default:
        sendError(request, ErrorCode::UnsupportedCommand);
        break;
    }
}

void pollLink() {
    Frame frame;
    ParseError parseError = ParseError::None;
    while (linkSerial.available() > 0) {
        const auto event = parser.feed(
            static_cast<std::uint8_t>(linkSerial.read()), millis(), frame, parseError
        );
        if (event == ParseEvent::FrameReady) {
            handleRequest(frame);
        } else if (event == ParseEvent::Error) {
            Serial.printf("[ERROR] ObsidiaLink parser error=%u\n", static_cast<unsigned>(parseError));
        }
    }

    if (parser.tick(millis(), parseError) == ParseEvent::Error) {
        Serial.printf("[ERROR] ObsidiaLink parser timeout=%u\n", static_cast<unsigned>(parseError));
    }
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("obsidia-radio 0.1.0 / ObsidiaLink v1");

    if (OBSIDIA_RADIO_RX_GPIO < 0 || OBSIDIA_RADIO_TX_GPIO < 0) {
        Serial.println("[ERROR] ObsidiaLink UART GPIO unconfigured; see hardware/pinout/pinmap.md");
        return;
    }

    linkSerial.begin(kLinkBaud, SERIAL_8N1, OBSIDIA_RADIO_RX_GPIO, OBSIDIA_RADIO_TX_GPIO);
    linkConfigured = true;
    Serial.println("[OK] ObsidiaLink UART ready");
}

void loop() {
    if (linkConfigured) pollLink();
    if (resetPending && static_cast<std::int32_t>(millis() - resetAtMs) >= 0) ESP.restart();
    delay(1);
}
