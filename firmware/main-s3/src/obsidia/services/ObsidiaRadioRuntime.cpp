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
    if (started) radioService.poll(millis());
}

const RadioService::Snapshot &radioRuntimeSnapshot() {
    return radioService.snapshot();
}

} // namespace obsidia

#endif
