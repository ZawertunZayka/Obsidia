# Testing

## Evidence levels

- `host`: native unit test or RTL simulation passed.
- `build`: firmware/bitstream compiled for a named target.
- `connected`: device identity or register access observed over a physical bus.
- `physical`: requested visible or electrical behavior was confirmed.
- `stress`: required repeated operation completed on hardware.

Only the last three levels establish hardware success. Each result in
`current-status.md` names its evidence level.

## Required suites

- ObsidiaLink: CRC vectors, round-trip, fragmented input, concatenated frames,
  bad CRC, oversized length, invalid command, timeout/reset and resynchronization.
- FPGA SPI: register reads/writes, invalid address behavior, reset defaults,
  burst boundaries, IRQ flags and FIFO overflow/underflow.
- ESP32-S3: standalone diagnostics for every local hardware service.
- Radio ESP32: protocol loopback/PING and standalone peripheral probes.
- CC1101 standalone: reset-ready timeout, PARTNUM/VERSION identity, reversible
  PKTLEN register write/read/restore and valid MARCSTATE without RF transmission.
- Final: cold boot, repeated reset, USB reconnect, long runtime, concurrent
  initialization and independent coprocessor resets.
