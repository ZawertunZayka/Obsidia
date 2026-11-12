#include <Arduino.h>
#include <SPI.h>

#include "Nrf24Diagnostic.hpp"
#include "obsidia_link.hpp"

#ifndef OBSIDIA_RADIO_RX_GPIO
#define OBSIDIA_RADIO_RX_GPIO -1
#endif
#ifndef OBSIDIA_RADIO_TX_GPIO
#define OBSIDIA_RADIO_TX_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_SCLK_GPIO
#define OBSIDIA_NRF24_SCLK_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_MISO_GPIO
#define OBSIDIA_NRF24_MISO_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_MOSI_GPIO
#define OBSIDIA_NRF24_MOSI_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_CSN_GPIO
#define OBSIDIA_NRF24_CSN_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_CE_GPIO
#define OBSIDIA_NRF24_CE_GPIO -1
#endif
#ifndef OBSIDIA_NRF24_SPI_HZ
#define OBSIDIA_NRF24_SPI_HZ 1000000
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
constexpr std::uint8_t kNrf24Config = 0x00;
constexpr std::uint8_t kNrf24RfChannel = 0x05;
constexpr std::uint8_t kNrf24RfSetup = 0x06;
constexpr std::uint8_t kNrf24Rpd = 0x09;

HardwareSerial linkSerial(2);
obsidia::link::Parser parser;
std::array<std::uint8_t, obsidia::link::kMaxFrameSize> txBuffer{};
bool linkConfigured = false;
bool resetPending = false;
std::uint32_t resetAtMs = 0;
std::uint8_t peripheralStatus = 0;

class ArduinoNrf24Bus final : public obsidia::radio::Nrf24RegisterBus {
public:
    ArduinoNrf24Bus(SPIClass &spi, int csn) : spi_(spi), csn_(csn) {}

    bool readStatus(std::uint8_t &status) override {
        beginTransaction();
        status = spi_.transfer(0xffU);
        endTransaction();
        return true;
    }

    bool readRegister(std::uint8_t address, std::uint8_t &value) override {
        beginTransaction();
        spi_.transfer(address & 0x1fU);
        value = spi_.transfer(0xffU);
        endTransaction();
        return true;
    }

    bool writeRegister(std::uint8_t address, std::uint8_t value) override {
        beginTransaction();
        spi_.transfer(0x20U | (address & 0x1fU));
        spi_.transfer(value);
        endTransaction();
        return true;
    }

private:
    void beginTransaction() {
        spi_.beginTransaction(SPISettings(OBSIDIA_NRF24_SPI_HZ, MSBFIRST, SPI_MODE0));
        digitalWrite(csn_, LOW);
    }

    void endTransaction() {
        digitalWrite(csn_, HIGH);
        spi_.endTransaction();
    }

    SPIClass &spi_;
    int csn_;
};

SPIClass nrf24Spi(HSPI);
ArduinoNrf24Bus nrf24Bus(nrf24Spi, OBSIDIA_NRF24_CSN_GPIO);
bool nrf24Configured = false;

void sendFrame(Command command, const std::uint8_t *payload, std::size_t payloadLength);
void sendError(std::uint8_t requestCommand, ErrorCode error);

void runNrf24Diagnostic() {
    if (OBSIDIA_NRF24_SCLK_GPIO < 0 || OBSIDIA_NRF24_MISO_GPIO < 0 ||
        OBSIDIA_NRF24_MOSI_GPIO < 0 || OBSIDIA_NRF24_CSN_GPIO < 0 ||
        OBSIDIA_NRF24_CE_GPIO < 0) {
        Serial.println("[NRF24] GPIO unconfigured; ready bit remains clear");
        return;
    }

    pinMode(OBSIDIA_NRF24_CSN_GPIO, OUTPUT);
    pinMode(OBSIDIA_NRF24_CE_GPIO, OUTPUT);
    digitalWrite(OBSIDIA_NRF24_CSN_GPIO, HIGH);
    digitalWrite(OBSIDIA_NRF24_CE_GPIO, LOW);
    nrf24Spi.begin(
        OBSIDIA_NRF24_SCLK_GPIO,
        OBSIDIA_NRF24_MISO_GPIO,
        OBSIDIA_NRF24_MOSI_GPIO,
        OBSIDIA_NRF24_CSN_GPIO
    );
    delay(5);

    nrf24Configured = true;
    const auto result = obsidia::radio::Nrf24Diagnostic(nrf24Bus).run();
    if (result.passed) {
        peripheralStatus |= kStatusNrf24;
        Serial.printf(
            "[NRF24][PASS] STATUS=%02X CONFIG=%02X RF_CH=%02X RF_SETUP=%02X\n",
            result.status,
            result.config,
            result.rfChannel,
            result.rfSetup
        );
    } else {
        Serial.printf(
            "[NRF24][ERROR] diagnostic=%u STATUS=%02X CONFIG=%02X RF_CH=%02X\n",
            static_cast<unsigned>(result.error),
            result.status,
            result.config,
            result.rfChannel
        );
    }
}

void sendData(std::uint8_t request, const std::uint8_t *data, std::size_t length) {
    std::array<std::uint8_t, obsidia::link::kMaxPayloadSize> payload{};
    if (length + 2U > payload.size()) {
        sendError(request, ErrorCode::InternalFailure);
        return;
    }
    payload[0] = request;
    payload[1] = obsidia::link::kProtocolVersion;
    for (std::size_t i = 0; i < length; ++i) payload[i + 2U] = data[i];
    sendFrame(Command::Data, payload.data(), length + 2U);
}

bool validNrf24Request(const Frame &frame) {
    switch (frame.command) {
    case Command::Nrf24Status:
    case Command::Nrf24Stop:
        return frame.payloadLength == 0;
    case Command::Nrf24SetChannel:
    case Command::Nrf24Rpd:
        return frame.payloadLength == 1 && frame.payload[0] <= 125;
    case Command::Nrf24Carrier:
        return frame.payloadLength == 2 && frame.payload[0] <= 125 && frame.payload[1] <= 3;
    default:
        return frame.payloadLength == 0;
    }
}

void handleNrf24Request(const Frame &frame) {
    const auto request = static_cast<std::uint8_t>(frame.command);
    if (!nrf24Configured || (peripheralStatus & kStatusNrf24) == 0U) {
        sendError(request, ErrorCode::PeripheralUnavailable);
        return;
    }
    if (frame.command == Command::Nrf24Status) {
        std::uint8_t status = 0;
        std::uint8_t config = 0;
        std::uint8_t channel = 0;
        std::uint8_t setup = 0;
        if (!nrf24Bus.readStatus(status) || !nrf24Bus.readRegister(kNrf24Config, config) ||
            !nrf24Bus.readRegister(kNrf24RfChannel, channel) ||
            !nrf24Bus.readRegister(kNrf24RfSetup, setup)) {
            sendError(request, ErrorCode::InternalFailure);
            return;
        }
        const std::uint8_t data[] = {status, config, channel, setup};
        sendData(request, data, sizeof(data));
        return;
    }
    if (frame.command == Command::Nrf24SetChannel) {
        if (!nrf24Bus.writeRegister(kNrf24RfChannel, frame.payload[0])) {
            sendError(request, ErrorCode::InternalFailure);
            return;
        }
        const std::uint8_t ack[] = {request, obsidia::link::kProtocolVersion};
        sendFrame(Command::Ack, ack, sizeof(ack));
        return;
    }
    if (frame.command == Command::Nrf24Rpd) {
        digitalWrite(OBSIDIA_NRF24_CE_GPIO, LOW);
        if (!nrf24Bus.writeRegister(kNrf24Config, 0x03U) ||
            !nrf24Bus.writeRegister(kNrf24RfChannel, frame.payload[0])) {
            sendError(request, ErrorCode::InternalFailure);
            return;
        }
        digitalWrite(OBSIDIA_NRF24_CE_GPIO, HIGH);
        delayMicroseconds(140);
        std::uint8_t rpd = 0;
        const bool ok = nrf24Bus.readRegister(kNrf24Rpd, rpd);
        digitalWrite(OBSIDIA_NRF24_CE_GPIO, LOW);
        nrf24Bus.writeRegister(kNrf24Config, 0x08U);
        if (!ok) {
            sendError(request, ErrorCode::InternalFailure);
            return;
        }
        sendData(request, &rpd, 1);
        return;
    }
    if (frame.command == Command::Nrf24Carrier) {
        const std::uint8_t rfPower = static_cast<std::uint8_t>(0x80U | 0x10U |
            static_cast<std::uint8_t>((frame.payload[1] & 0x03U) << 1U));
        if (!nrf24Bus.writeRegister(kNrf24RfChannel, frame.payload[0]) ||
            !nrf24Bus.writeRegister(kNrf24RfSetup, rfPower) ||
            !nrf24Bus.writeRegister(kNrf24Config, 0x0AU)) {
            sendError(request, ErrorCode::InternalFailure);
            return;
        }
        digitalWrite(OBSIDIA_NRF24_CE_GPIO, HIGH);
        const std::uint8_t ack[] = {request, obsidia::link::kProtocolVersion};
        sendFrame(Command::Ack, ack, sizeof(ack));
        return;
    }
    if (frame.command == Command::Nrf24Stop) {
        digitalWrite(OBSIDIA_NRF24_CE_GPIO, LOW);
        nrf24Bus.writeRegister(kNrf24Config, 0x08U);
        nrf24Bus.writeRegister(kNrf24RfSetup, 0x0FU);
        const std::uint8_t ack[] = {request, obsidia::link::kProtocolVersion};
        sendFrame(Command::Ack, ack, sizeof(ack));
    }
}

void sendFrame(Command command, const std::uint8_t *payload, std::size_t payloadLength) {
    std::size_t encodedLength = 0;
    if (!obsidia::link::encode(
            command, payload, payloadLength, txBuffer.data(), txBuffer.size(), encodedLength
        )) {
        Serial.println("[ERROR] ObsidiaLink response encoding failed");
        return;
    }
    linkSerial.write(txBuffer.data(), encodedLength);
#ifdef OBSIDIA_LINK_TRACE
    Serial.printf(
        "[LINK][TX] command=0x%02X payload=%u bytes=%u\n",
        static_cast<unsigned>(command),
        static_cast<unsigned>(payloadLength),
        static_cast<unsigned>(encodedLength)
    );
#endif
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
    if (!validNrf24Request(frame)) {
        sendError(request, ErrorCode::InvalidPayload);
        return;
    }

    const auto commandValue = static_cast<std::uint8_t>(frame.command);
    if (commandValue >= static_cast<std::uint8_t>(Command::Nrf24Status) &&
        commandValue <= static_cast<std::uint8_t>(Command::Nrf24Stop)) {
        handleNrf24Request(frame);
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
#ifdef OBSIDIA_LINK_TRACE
            Serial.printf(
                "[LINK][RX] command=0x%02X payload=%u\n",
                static_cast<unsigned>(frame.command),
                static_cast<unsigned>(frame.payloadLength)
            );
#endif
            handleRequest(frame);
        } else if (event == ParseEvent::Error) {
            Serial.printf("[ERROR] ObsidiaLink parser error=%u\n", static_cast<unsigned>(parseError));
            sendError(0, obsidia::link::errorCodeFor(parseError));
        }
    }

    if (parser.tick(millis(), parseError) == ParseEvent::Error) {
        Serial.printf("[ERROR] ObsidiaLink parser timeout=%u\n", static_cast<unsigned>(parseError));
        sendError(0, obsidia::link::errorCodeFor(parseError));
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
    runNrf24Diagnostic();
}

void loop() {
    if (linkConfigured) pollLink();
    if (resetPending && static_cast<std::int32_t>(millis() - resetAtMs) >= 0) ESP.restart();
    delay(1);
}
