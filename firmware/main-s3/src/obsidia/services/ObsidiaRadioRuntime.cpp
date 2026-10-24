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

} // namespace obsidia

#endif
