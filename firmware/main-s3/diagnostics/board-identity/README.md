# ESP32-S3 board identity diagnostic

This standalone image uses no peripheral GPIO. It reports the runtime chip
revision, flash geometry and PSRAM initialization state over native USB CDC.

`identity-safe` deliberately leaves PSRAM disabled and is the first firmware
probe only after `esptool` confirms an ESP32-S3. A zero PSRAM result from that
environment means **disabled**, not absent. `identity-n8r8` enables the common
8 MB octal-PSRAM topology and must not be flashed until the memory/package
identity or module marking confirms N8R8 compatibility.

Neither environment identifies which physical board pins are exposed. GPIO
assignment still requires the exact board model/revision and readable pin
labels; results belong in `hardware/pinout/pinmap.md`.
