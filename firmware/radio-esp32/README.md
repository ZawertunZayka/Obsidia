# obsidia-radio

Firmware for the ESP32 DevKit V1 radio/peripheral coprocessor. Bruce must never
run here. The current milestone implements the complete core ObsidiaLink command
set (`PING`, version, status and delayed reset) without dynamic allocation in the
communication path.

UART GPIO defaults are deliberately `-1`. Set them only after the exact board and
wiring are confirmed in `hardware/pinout/pinmap.md`; otherwise firmware logs a
configuration error and does not open the link UART.

RDM6300, CC1101 and NRF24 status bits remain clear until their standalone
diagnostics pass on hardware.
