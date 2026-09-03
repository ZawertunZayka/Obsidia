# Bring-up policy

Every module is tested standalone before Bruce integration. A passing compiler
or simulator result is not a physical pass.

## Safe sequence

1. Record exact board and module markings.
2. Confirm supply and logic voltage from authoritative documentation or markings.
3. Allocate non-conflicting GPIO in `hardware/pinout/pinmap.md`.
4. **ОТКЛЮЧИ ПИТАНИЕ** before wiring or rewiring.
5. Check continuity, common ground, polarity and decoupling.
6. Flash only the standalone diagnostic for the module under test.
7. Capture serial output and physical observations.
8. Mark the test passed only after evidence is recorded.

## Diagnostic order

ESP32-S3 identity and memory, ST7735 color/geometry, controls, SD 100-cycle
stress, IR, PN532, ObsidiaLink PING, RDM6300, CC1101 register access, NRF24
presence, FPGA registers, FPGA IRQ and FPGA FIFO. Integration follows in the
same order.

## ESP32-S3 identity gate

The no-GPIO probe lives at `firmware/main-s3/diagnostics/board-identity`.
Connect only the main board by USB first. Inspect it with `lsusb`, PlatformIO
device enumeration and `esptool` before flashing. `identity-safe` does not
initialize PSRAM; its zero PSRAM value is therefore not evidence that PSRAM is
absent. Flash `identity-n8r8` only after the package/module marking or probe data
confirms 8 MB octal PSRAM compatibility. Runtime output and clear photos of both
board sides are required before assigning any GPIO.

## microSD notes

The module includes an LVC125A/SN74xx125-like buffer and has previously returned
`CMD8 timeout 0x108` / `ESP_ERR_INVALID_RESPONSE`. Validate actual module supply,
logic direction and buffer enable wiring before changing software. The standalone
test performs 100 create/write/read/verify/delete cycles and reports the exact
failed cycle and operation.
