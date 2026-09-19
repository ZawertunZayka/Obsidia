# obsidia-radio

Firmware for the ESP32 DevKit V1 radio/peripheral coprocessor. Bruce must never
run here. The current milestone implements the complete core ObsidiaLink command
set (`PING`, version, status and delayed reset) without dynamic allocation in the
communication path.

The confirmed project assignment uses UART2 RX GPIO16 and TX GPIO17 at 115200
8N1. The ESP32-S3 side uses RX GPIO15 and TX GPIO16. The two boards share GND;
their power rails are not connected while both are powered over USB.

RDM6300, CC1101 and NRF24 status bits remain clear until their standalone
diagnostics pass on hardware.
