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
absent. The reported `N16R8` marking selects `identity-n16r8` for the runtime
probe. Runtime output and clear photos of both board sides are required before
assigning any GPIO.

## microSD notes

The module includes an LVC125A/SN74xx125-like buffer and has previously returned
`CMD8 timeout 0x108` / `ESP_ERR_INVALID_RESPONSE`. Validate actual module supply,
logic direction and buffer enable wiring before changing software. The standalone
test performs 100 create/write/read/verify/delete cycles and reports the exact
failed cycle and operation.

The photographed adapter includes a 1117-class regulator, so its header `VCC`
is assigned to `5VIN`; feeding that input from 3.3 V would pass through the
regulator dropout and can undervolt the card. The reported LVC125A-class buffer
is operated from the regulated side: ESP32 bus pins remain strictly 3.3 V. See
the manufacturer specifications for the [SN74LVC125A supply/input limits](https://www.ti.com/product/SN74LVC125A)
and [AP1117 dropout behavior](https://www.diodes.com/part/view/AP1117).

Connected evidence on 2026-09-04 was `idle=00 CMD0=00 CMD8=FF`, followed by
three mount failures. This is not a filesystem or SPI-clock tuning result: MISO
is held low before a valid SD idle response. Inspect VCC/GND/MISO continuity and
card seating before any further software change; a valid CMD0 idle response is
`01`.

A second run enabled the GPIO13 internal 3.3 V pull-up and allowed 500 ms power
settling; the retained result remained `idle=00 CMD0=00`. The line is therefore
externally driven/clamped low rather than merely floating. The next isolation
step is to sample the same pin with only the module MISO lead disconnected.
