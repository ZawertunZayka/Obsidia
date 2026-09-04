# Current status

Updated: 2026-09-04

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
- ST7735 VER 1.0 module identified from front/back photos. Its 3.3 V-only raw
  SPI standalone diagnostic was flashed and the user observed the final
  `OBSIDIA` / `DISPLAY OK` screen.
- Hardware-independent `InputService` provides debounced press, release and
  one-shot long-press events for eight keys with a fixed 16-event queue,
  overflow accounting and wrap-safe timing.
- The photographed controls are an `8 Push Buttons V1.1` passive common-ground
  board. A dedicated N16R8 standalone diagnostic builds for its assigned GPIO
  and requires all eight debounced presses before reporting pass.
- The buffered six-pin microSD adapter is identified and assigned to the shared
  local SPI bus with GPIO8 chip-select. Its non-formatting standalone diagnostic
  builds with explicit mount retries and 100 destructive-only-to-temp-file
  create/write/read/verify/delete cycles.
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
| S3 identity diagnostic | pass | connected | N16R8 image flashed; 16 MB QIO/80 MHz flash and initialized 8 MB PSRAM reported |
| Main bus adapters | pass | build | Arduino transport object compiles for the S3 control target |
| Radio UART transport | pending hardware | none | GPIO deliberately unset until board and wiring confirmation |
| FPGA base register RTL | pass | simulation | Icarus testbench, Yosys structural check and Verilator lint pass |
| FPGA IRQ RTL | pass | simulation | Pending/status/IRQ consistency and acknowledgement toggle tested |
| FPGA FIFO RTL | pass | simulation | CDC, ordered burst reads, clear, full/empty and overflow rejection tested |
| FPGA synthesis/bitstream | blocked on board revision | none | Device/package and pin constraints intentionally unset |
| Main module identity | pass | connected | ESP32-S3 QFN56 rev v0.2, 40 MHz crystal, N16R8 memory confirmed by ROM/JEDEC/runtime probes |
| Main carrier identity | partial | connected | DevKitC-1-compatible 44-pin layout; right USB-C confirmed QinHeng USB-UART, vendor/revision unmarked |
| GPIO map | partial | visual/design | Board-owned pins reserved; ST7735 assignment recorded, all later modules remain unset |
| ST7735 diagnostic | pass | build/connected | Bounded raw-SPI 128x160 diagnostic builds and runs at 10 MHz on N16R8 |
| ST7735 upload | pass | connected | Diagnostic written and flash hashes verified over native USB |
| ST7735 hardware | pass | user observation | Final black screen with `OBSIDIA` / `DISPLAY OK` is visible; write-only panel cannot provide readback |
| ST7735 color calibration | partial | programmed sequence | RGB/black/white sequence ran; individual color naming was not separately reported |
| InputService | pass | host | Debounce, release, long-press, simultaneous keys, bounded queue and timer rollover pass |
| Controls diagnostic | pass | build | N16R8 standalone build uses InputService and explicit K1-K8 mapping |
| Controls hardware | pass | connected/user action | K1-K8 debounced presses arrived in correct order; firmware emitted `[PASS] ALL 8 KEYS OBSERVED` |
| Controls long press | partial | host | 600 ms one-shot behavior passes unit tests; physical long-press event was not observed in the capture window |
| microSD diagnostic | pass | build/connected | Retained raw CMD0/CMD8 evidence plus 400 kHz mount retries and 100-cycle 4 MHz stress path |
| microSD hardware | fail under diagnosis | connected/isolation | Adapter drives MISO low with and without a card; isolated GPIO13 reads high and is healthy; CMD0 never reaches idle |
| SD stress | blocked by adapter/power | connected | Requires a 3.3 V regulator-output measurement or replacement adapter before rerun; no Bruce integration allowed |
| IR through final stress test | not started | none | Follows connected microSD stress pass |

## Gate

The main board, memory configuration and first external SPI assignment are now
established. New modules remain gated on an explicit pin-map assignment,
standalone diagnostic and physical observation before Bruce integration.
