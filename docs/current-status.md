# Current status

Updated: 2026-09-03

## Completed

- Repository/Git audit: clean `main`, correct `origin`, initial history retained.
- Workspace structure and engineering policy established.
- Bruce 1.16.1 archive analyzed; generic ESP32-S3 PSRAM target selected as a
  structural, not electrical, base.
- Buildable Bruce 1.16.1 source snapshot imported into `firmware/main-s3` with
  provenance, license and explicit Obsidia extension paths.
- Generic Bruce `esp32-s3-devkitc-1-psram` control build passes after making the
  upstream net80211 patch step compatible with the pinned modern toolchain.
- UART and FPGA SPI protocol drafts documented with bounded framing and recovery
  requirements.
- Shared ObsidiaLink v1 codec/parser and radio-coprocessor core implemented;
  physical UART GPIO intentionally remains unset.
- Main-S3 `RadioService` implements bounded non-blocking discovery, heartbeat,
  timeout retries and automatic rediscovery without UI or GPIO coupling.
- Main-S3 `FpgaService` validates identity/version/readiness, detects resets,
  rediscovers the FPGA and exposes bounded FIFO/control operations.
- `OBSIDIA DIAGNOSTICS` has a fixed 20-component model plus an `OBSIDIA_V1`-only
  Bruce menu; unknown hardware remains visibly UNTESTED/N/A rather than passing.
- RDM6300 standalone diagnostic and fixed-frame EM4100 parser implemented;
  hardware pass remains pending confirmed radio-board UART wiring and a real tag.
- CC1101 standalone diagnostic implements bounded raw-SPI reset/ready timing,
  identity reads, reversible register verification and MARCSTATE validation;
  no RF operation is enabled and all GPIO remain unset.
- NRF24L01+ standalone diagnostic validates STATUS/register reserved bits and a
  reversible RF_CH write with CE held low; it performs no receive/transmit work.
- ESP32-S3 board-identity firmware is prepared in safe/no-PSRAM and N16R8 probe
  variants; both compile and use no peripheral GPIO.
- Arduino `Stream`/`SPIClass` adapters connect those services to Bruce buses
  while rejecting unset CS/frequency and retaining board-owned pin selection.
- FPGA SPI mode 0 slave, base `OBSD` register map, IRQ aggregation and bounded
  dual-clock FIFO implemented without pin constraints.

## Verification ledger

| Area | State | Evidence | Notes |
|---|---|---|---|
| Repository | pass | host | Git worktree and remote inspected |
| Bruce analysis | pass | host | Local source files inspected; archive is not Git |
| Bruce control build | pass | build | PlatformIO success; 3,701,374 B flash, 128,320 B RAM |
| ObsidiaLink codec/parser | pass | host | CRC vector, framing, bounds, timeout and resynchronization tests pass |
| Radio MCU firmware | pass | build | PlatformIO `obsidia-radio` success; 289,780 B flash, 22,244 B RAM |
| Main RadioService | pass | host/build | State-machine tests pass and complete Bruce control build succeeds |
| Main FpgaService | pass | host/build | Recovery/FIFO/control tests pass and complete Bruce control build succeeds |
| Diagnostics model | pass | host/build | Inventory/mapping/bounds tests pass; S3 object compiles |
| Diagnostics UI | pending target | build guard | Menu is isolated behind `OBSIDIA_V1`; target awaits board identity |
| RDM6300 parser/diagnostic | pass | host/build | Framing/checksum/resync/timeout tests and ESP32 build pass |
| RDM6300 hardware | pending wiring | none | RX GPIO unset; no physical tag evidence |
| CC1101 diagnostic | pass | host/build | Failure paths and register restoration tested; ESP32 build succeeds |
| CC1101 hardware | pending wiring | none | SPI GPIO unset; no physical register evidence |
| NRF24 diagnostic | pass | host/build | Presence/register failure paths pass; ESP32 build succeeds |
| NRF24 hardware | pending wiring/power | none | GPIO unset; PA/LNA supply and decoupling unverified |
| S3 identity diagnostic | pass | build | Safe and confirmed-memory N16R8 variants compile; neither has been flashed |
| Main bus adapters | pass | build | Arduino transport object compiles for the S3 control target |
| Radio UART transport | pending hardware | none | GPIO deliberately unset until board and wiring confirmation |
| FPGA base register RTL | pass | simulation | Icarus testbench, Yosys structural check and Verilator lint pass |
| FPGA IRQ RTL | pass | simulation | Pending/status/IRQ consistency and acknowledgement toggle tested |
| FPGA FIFO RTL | pass | simulation | CDC, ordered burst reads, clear, full/empty and overflow rejection tested |
| FPGA synthesis/bitstream | blocked on board revision | none | Device/package and pin constraints intentionally unset |
| Main module identity | partial | visual | User reports N16R8 marking; USB runtime confirmation pending |
| Main carrier identity | blocked on physical visibility | visual/host | Front pin order matches DevKitC-1; exact clone/revision and USB roles unconfirmed; USB rescan found no device |
| GPIO map | intentionally unset | none | Exact board/revision unknown |
| ST7735 through final stress test | not started | none | Must follow board identification |

## Gate

Software-only protocol, coprocessor and RTL work may proceed. No target GPIO,
wiring, flashing or physical pass claim is permitted until the exact ESP32-S3
board and pin exposure are established.
