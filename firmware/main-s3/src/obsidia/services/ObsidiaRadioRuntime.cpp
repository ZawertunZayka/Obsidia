#include "ObsidiaRadioRuntime.hpp"

#ifdef OBSIDIA_V1

#include "ArduinoTransports.hpp"

#include <Arduino.h>

#ifndef OBSIDIA_RADIO_RX_GPIO
#error "OBSIDIA_RADIO_RX_GPIO must be defined for OBSIDIA_V1"
#endif
#ifndef OBSIDIA_RADIO_TX_GPIO
#error "OBSIDIA_RADIO_TX_GPIO must be defined for OBSIDIA_V1"
#endif

namespace obsidia {
namespace {

constexpr std::uint32_t kRadioBaud = 115200;
HardwareSerial radioSerial(1);
ArduinoRadioTransport radioTransport(radioSerial);
RadioService radioService(radioTransport);
bool started = false;
std::uint32_t reportedDiscoveries = 0;
bool failureReported = false;
bool readyReported = false;

} // namespace

void beginRadioRuntime() {
    if (started) return;
    radioSerial.setRxBufferSize(link::kMaxFrameSize * 4U);
    radioSerial.begin(
        kRadioBaud, SERIAL_8N1, OBSIDIA_RADIO_RX_GPIO, OBSIDIA_RADIO_TX_GPIO
    );
    started = true;
    radioService.begin(millis());
    Serial.printf(
        "[OBSIDIA][RADIO] ObsidiaLink v%u UART ready RX=GPIO%d TX=GPIO%d baud=%u\n",
        static_cast<unsigned>(link::kProtocolVersion),
        OBSIDIA_RADIO_RX_GPIO,
        OBSIDIA_RADIO_TX_GPIO,
        static_cast<unsigned>(kRadioBaud)
    );
}

void pollRadioRuntime() {
    if (!started) return;
    radioService.poll(millis());
    const auto &snapshot = radioService.snapshot();
    if (snapshot.successfulDiscoveries != reportedDiscoveries) {
        reportedDiscoveries = snapshot.successfulDiscoveries;
        failureReported = false;
    }
    if (snapshot.successfulDiscoveries != 0 && !readyReported) {
        readyReported = true;
        Serial.printf(
            "[OBSIDIA][RADIO][OK] PING ACK; link=v%u firmware=%u.%u.%u status=0x%02X/0x%02X discoveries=%u\n",
            static_cast<unsigned>(snapshot.protocolVersion),
            static_cast<unsigned>(snapshot.firmwareVersion[0]),
            static_cast<unsigned>(snapshot.firmwareVersion[1]),
            static_cast<unsigned>(snapshot.firmwareVersion[2]),
            static_cast<unsigned>(snapshot.readyMask),
            static_cast<unsigned>(snapshot.supportedMask),
            static_cast<unsigned>(snapshot.successfulDiscoveries)
        );
    }
    if (snapshot.linkFailures != 0 && !failureReported) {
        failureReported = true;
        readyReported = false;
        Serial.printf(
            "[OBSIDIA][RADIO][ERROR] link failure=%u count=%u; rediscovery active\n",
            static_cast<unsigned>(snapshot.error),
            static_cast<unsigned>(snapshot.linkFailures)
        );
    }
}

const RadioService::Snapshot &radioRuntimeSnapshot() {
    return radioService.snapshot();
}

namespace {
bool remoteRequest(
    link::Command command,
    const std::uint8_t *payload,
    std::size_t payloadLength,
    link::Frame &response
) {
    return started && radioService.snapshot().health == RadioService::Health::Ready &&
           radioService.request(command, payload, payloadLength, response, millis());
}
}

bool remoteNrf24Status(RemoteNrf24Status &status) {
    link::Frame response;
    if (!remoteRequest(link::Command::Nrf24Status, nullptr, 0, response) ||
        response.command != link::Command::Data || response.payloadLength != 6 ||
        response.payload[0] != static_cast<std::uint8_t>(link::Command::Nrf24Status) ||
        response.payload[1] != link::kProtocolVersion) return false;
    status.status = response.payload[2];
    status.config = response.payload[3];
    status.rfChannel = response.payload[4];
    status.rfSetup = response.payload[5];
    return true;
}

bool remoteNrf24SetChannel(std::uint8_t channel) {
    const std::uint8_t payload[] = {channel};
    link::Frame response;
    return remoteRequest(link::Command::Nrf24SetChannel, payload, sizeof(payload), response) &&
           response.command == link::Command::Ack && response.payloadLength == 2;
}

bool remoteNrf24ReadRpd(std::uint8_t channel, std::uint8_t &rpd) {
    const std::uint8_t payload[] = {channel};
    link::Frame response;
    if (!remoteRequest(link::Command::Nrf24Rpd, payload, sizeof(payload), response) ||
        response.command != link::Command::Data || response.payloadLength != 3) return false;
    rpd = response.payload[2];
    return true;
}

bool remoteNrf24StartCarrier(std::uint8_t channel, std::uint8_t power) {
    const std::uint8_t payload[] = {channel, power};
    link::Frame response;
    return remoteRequest(link::Command::Nrf24Carrier, payload, sizeof(payload), response) &&
           response.command == link::Command::Ack && response.payloadLength == 2;
}

bool remoteNrf24Stop() {
    link::Frame response;
    return remoteRequest(link::Command::Nrf24Stop, nullptr, 0, response) &&
           response.command == link::Command::Ack && response.payloadLength == 2;
}

} // namespace obsidia

#endif
