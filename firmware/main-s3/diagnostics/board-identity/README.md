# ESP32-S3 board identity diagnostic

This standalone image uses no peripheral GPIO. It reports the runtime chip
revision, flash geometry and PSRAM initialization state over both UART0 and
native USB CDC.

`identity-safe` deliberately leaves PSRAM disabled and is the first firmware
probe only after `esptool` confirms an ESP32-S3. A zero PSRAM result from that
environment means **disabled**, not absent. `identity-n16r8` enables the
visually confirmed 16 MB quad-flash and 8 MB octal-PSRAM topology.

Neither environment identifies which physical board pins are exposed. GPIO
assignment still requires the exact board model/revision and readable pin
labels; results belong in `hardware/pinout/pinmap.md`.

## Confirmed result (2026-09-03)

The connected N16R8 board reported ESP32-S3 revision 2, two cores at 240 MHz,
16,777,216 bytes of QIO flash at 80 MHz, and 8,388,608 bytes of initialized
PSRAM. The report was captured through the board's USB-UART connector.
