#include <Arduino.h>

#include "Rdm6300Parser.hpp"

#ifndef OBSIDIA_RDM6300_RX_GPIO
#define OBSIDIA_RDM6300_RX_GPIO -1
#endif

namespace {

HardwareSerial rdmSerial(1);
obsidia::radio::Rdm6300Parser parser;
bool configured = false;

} // namespace

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("OBSIDIA RDM6300 STANDALONE DIAGNOSTIC");
    if (OBSIDIA_RDM6300_RX_GPIO < 0) {
        Serial.println("[ERROR] RDM6300 RX GPIO unconfigured; no hardware test performed");
        return;
    }
    rdmSerial.begin(9600, SERIAL_8N1, OBSIDIA_RDM6300_RX_GPIO, -1);
    configured = true;
    Serial.println("[READY] Present an owned EM4100-compatible tag");
}

void loop() {
    if (!configured) {
        delay(100);
        return;
    }

    obsidia::radio::Rdm6300Parser::Tag tag;
    obsidia::radio::Rdm6300Parser::Error error;
    while (rdmSerial.available() > 0) {
        const auto event = parser.feed(
            static_cast<std::uint8_t>(rdmSerial.read()), millis(), tag, error
        );
        if (event == obsidia::radio::Rdm6300Parser::Event::TagReady) {
            Serial.printf("[PASS] version=%02X tag=%08lX checksum=%02X\n",
                          tag.version, static_cast<unsigned long>(tag.id), tag.checksum);
        } else if (event == obsidia::radio::Rdm6300Parser::Event::Error) {
            Serial.printf("[ERROR] RDM6300 parser error=%u\n", static_cast<unsigned>(error));
        }
    }
    if (parser.tick(millis(), error) == obsidia::radio::Rdm6300Parser::Event::Error)
        Serial.printf("[ERROR] RDM6300 timeout error=%u\n", static_cast<unsigned>(error));
    delay(1);
}
