# Obsidia

Obsidia is a breadboard prototype of a portable electronics multitool. The
project stops at **OBSIDIA PROTOTYPE V1**; PCB, enclosure, CAD, Gerber and
manufacturing work are explicitly out of scope.

## Architecture

- ESP32-S3 master: Bruce UI, display, controls, storage, PN532, IR, native USB,
  Wi-Fi, BLE and links to both coprocessors.
- ESP32 DevKit V1 radio coprocessor: RDM6300, CC1101 and NRF24L01+ PA/LNA.
- Tang Nano 20K FPGA coprocessor: SPI slave, register map, IRQ and FIFO.

The UART link between both ESP32 devices is `ObsidiaLink`. The FPGA is attached
to a separate SPI master interface. No GPIO assignment is valid until recorded
as confirmed in `hardware/pinout/pinmap.md`.

## Repository

```text
firmware/main-s3/       Bruce 1.16.1 integration and ESP32-S3 diagnostics
firmware/radio-esp32/   radio coprocessor firmware
firmware/fpga/          Tang Nano 20K RTL and constraints
hardware/pinout/        authoritative pin and voltage map
protocol/               wire protocol specifications
docs/                   architecture, bring-up and status documentation
tests/                  host-side protocol and parser tests
```

See `docs/current-status.md` for verified status and `docs/bringup.md` before
connecting hardware. This project is for owned equipment, laboratory diagnosis
and authorized testing.
