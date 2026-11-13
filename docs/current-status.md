# Current status

Updated: 2026-11-13

## Current plan position

The physical ObsidiaLink link and remote NRF24 Spectrum work, including the new
Left/Escape exit behavior. **Next: Stage 11, RDM6300 standalone diagnostic.**
The host parser tests and standalone PlatformIO build pass. The diagnostic was
flashed and hash-verified on the WROOM, then the production radio image was
restored and hash-verified. The user photo identifies the reader as HW-205;
physical wiring and tag reading remain pending. GPIO32 is assigned for RX with
a 5.1 kΩ/10 kΩ TX divider. The user reports the microSD works on GPIO18/3/46/17.

Current firmware: Bruce 1.17 on the S3 and ObsidiaLink radio firmware on the
WROOM. The user confirms Spectrum works and exits after the Left/Escape fix.

## Completed

- Repository/Git audit: clean `main`, correct `origin`, initial history retained.
- Workspace structure and engineering policy established.
- Bruce 1.16.1 archive analyzed; generic ESP32-S3 PSRAM target selected as a
  structural, not electrical, base.
- Bruce 1.17 imported from upstream commit
  `2b6ba4d490bfcf94ce2574c6154537dbd5838c3d`; provenance, license and Obsidia
  extension paths are recorded in `firmware/main-s3/OBSIDIA_UPSTREAM.md`.
- The `OBSIDIA_V1` target now follows the upstream `esp32s3` generic board JSON
  while retaining confirmed N16R8 memory, display, CardKB, SD and UART settings.
- NRF Spectrum now uses Bruce 1.17's `SpectrumPlot` trace, peak-hold and
  waterfall/ruler renderer. The former bar-and-label renderer was too narrow
  and crowded the small display. The user confirms the updated graph and remote
  radio work on the device.
- The S3 radio runtime now guards the shared UART service with a FreeRTOS mutex
  so the background CardKB/input task cannot consume synchronous Spectrum
  replies. Host protocol tests pass, and the user confirms the Spectrum works.
- DOOM has been removed from the production Bruce firmware; the repository's
  `test_no_doom.py` guard passes.
- Generic Bruce `esp32-s3-devkitc-1-psram` control build passes after making the
  upstream net80211 patch step compatible with the pinned modern toolchain.
- UART and FPGA SPI protocol drafts documented with bounded framing and recovery
  requirements.
- Shared ObsidiaLink v1 codec/parser and radio-coprocessor core implemented;
  physical UART GPIO intentionally remains unset.
- Radio parser failures now produce bounded protocol `ERROR` responses: bad CRC
  is distinguished from invalid length and incomplete-frame timeout.
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
  For the photographed HW-394 radio ESP32 DevKit, pins are assigned as
  SCK=18, MISO=19, MOSI=23, CSN=21, CE=22 (IRQ unused). The diagnostic passed
  on hardware with STATUS=0x0E, CONFIG=0x08, RF_CH=0x02 and RF_SETUP=0x0F.
  The main radio firmware now runs this bounded diagnostic at boot and reports
  the NRF24 ready bit through ObsidiaLink.
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
  and previously passed all eight keys, but the user has now superseded it with
  M5Stack CardKB.
- M5Stack Unit CardKB v1.1 is the active control device. Its bounded I2C `0x5F`
  standalone diagnostic passed on GPIO40/GPIO41 at 3.3 V: address ACK, printable
  keys, Enter, Escape and direction bytes were observed.
- The permanent Bruce `OBSIDIA_V1` target now selects the confirmed ESP32-S3
  N16R8 memory layout, ST7735, CardKB, shared-bus microSD and native USB HID.
  The corrected image with CardKB text-entry support built and flashed
  successfully with verified hashes. Physical feedback showed that this panel
  needs the opposite state from Bruce's default. The follow-up build persists
  `colorInverted=0` after loading settings, applies `INVOFF`, and was flashed
  successfully with verified hashes; physical confirmation is pending.
- The Cyberpunk 2077 Bruce theme was validated and copied to the microSD as
  `/Cyberpunk_2077_v1.0.0`; all 15 files match the source archive.
- CardKB input now suppresses repeated reports of a held key until release,
  preventing the Enter used for `Theme -> SD Card` from immediately selecting
  the file browser's `>back` entry. The corrected Bruce image built and flashed
  successfully with verified hashes; physical confirmation is pending.
- The original buffered six-pin microSD adapter was isolated as faulty and is
  quarantined. The non-formatting standalone diagnostic retains explicit mount
  retries and 100 destructive-only-to-temp-file create/write/read/verify/delete
  cycles for the replacement hardware.
- Bruce issue #2445 reproduces the same visible `Files -> SD Card`/vanishing-icon
  symptom and was closed with release 1.16. The Obsidia tree is already based
  on 1.16.1 and includes that release's exFAT-capable framework package, so the
  upstream issue is useful corroboration but is not the missing local fix.
- Historical microSD bring-up on GPIO8/11/12/13 returned `0xFF` to CMD0 in both
  hardware and bit-banged SPI tests. The user later changed the passive
  eight-pin module wiring to GPIO18/3/46/17 and reported that the setup works.
  Treat the earlier raw-CMD0 failure as specific to the former assignment; the
  replacement mapping is recorded in `hardware/pinout/pinmap.md`.
- Photographed IR hardware is a KY-005-style transmitter-only board. GPIO17 is
  assigned for its protected `S` path and a bounded 38 kHz camera diagnostic is
  prepared; an IR receiver has not yet been identified.
- Arduino `Stream`/`SPIClass` adapters connect those services to Bruce buses
  while rejecting unset CS/frequency and retaining board-owned pin selection.
- The ESP32-S3 and ESP32-WROOM are connected by the assigned 115200 8N1 UART.
  Physical logs show repeated `PING`, `GET_VERSION` and `GET_STATUS` requests,
  bounded `ACK`/`DATA` replies, protocol v1, radio firmware 0.1.0 and supported
  mask `0x07`. Bruce automatically rediscovers the coprocessor and reports link
  state transitions without flooding the CardKB input task.
- FPGA SPI mode 0 slave, base `OBSD` register map, IRQ aggregation and bounded
  dual-clock FIFO implemented without pin constraints.

## Verification ledger

| Area | State | Evidence | Notes |
|---|---|---|---|
| Repository | pass | host | Git worktree and remote inspected |
| Bruce analysis | pass | host | Local source files inspected; archive is not Git |
| Bruce 1.17 `OBSIDIA_V1` build | pass | build | PlatformIO success; 4,132,042 B flash, 138,416 B RAM |
| ObsidiaLink codec/parser | pass | host | CRC vector, framing, bounds, timeout, error mapping and resynchronization tests pass |
| Radio MCU firmware | pass | build/connected | PlatformIO `obsidia-radio` success; 291,460 B flash, 22,308 B RAM; physical request/response logs observed |
| Main RadioService | pass | host/build | State-machine tests pass and complete Bruce control build succeeds |
| Main FpgaService | pass | host/build | Recovery/FIFO/control tests pass and complete Bruce control build succeeds |
| Diagnostics model | pass | host/build | Inventory/mapping/bounds tests pass; S3 object compiles |
| Diagnostics UI | pass build/upload | connected | `OBSIDIA_V1` menu is linked into the permanent Bruce image |
| RDM6300 parser/diagnostic | pass | host/build | Framing/checksum/resync/timeout tests and ESP32 build pass |
| RDM6300 hardware | pending wiring/tag test | user photo | HW-205 identified; GPIO32 RX and 5 V-to-3.3 V divider assigned; no physical tag evidence |
| CC1101 diagnostic | pass | host/build | Failure paths and register restoration tested; ESP32 build succeeds |
| CC1101 hardware | pending wiring | none | SPI GPIO unset; no physical register evidence |
| NRF24 diagnostic | pass | host/build | Presence/register failure paths pass; ESP32 build succeeds |
| NRF24 hardware | pass | connected | Standalone diagnostic passed on assigned GPIO18/19/23/21/22; register response observed; main firmware reports ready bit |
| Remote NRF24 Spectrum UI | pass | user observation + flash hash | Graph and remote radio work; user confirms Left/Escape exits |
| S3 identity diagnostic | pass | connected | N16R8 image flashed; 16 MB QIO/80 MHz flash and initialized 8 MB PSRAM reported |
| Main bus adapters | pass | build | Arduino transport object compiles for the S3 control target |
| Radio UART transport | pass | connected | Repeated PING/ACK, version 0.1.0 and status 0x00/0x07 observed in both MCU logs |
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
| CardKB replacement | pass | connected | Unit CardKB v1.1 ACKs at 0x5F and emitted printable, Enter, Escape and arrow bytes at 3.3 V |
| microSD diagnostic | pass | build/connected | Retained raw CMD0/CMD8 evidence plus 400 kHz mount retries and 100-cycle 4 MHz stress path |
| old microSD hardware | failed/quarantined | connected/isolation | Buffered adapter drives MISO low with and without a card; isolated GPIO13 reads high and is healthy; CMD0 never reaches idle |
| replacement microSD hardware | user-reported working | user observation | After changing the passive module to GPIO18/3/46/17, user reported the full setup works; this session did not repeat the physical stress test |
| SD stress | pending repeatable evidence | build | Existing bounded 100-cycle diagnostic remains available; earlier `0xFF` CMD0 readings were on the previous pin assignment |
| IR TX diagnostic | pass build/upload / inconclusive hardware | build/connected | N16R8 image and 38 kHz burst logs pass; user saw no light with an uncalibrated phone camera and has no reference remote |
| IR RX through final stress test | not started | none | Receiver hardware has not been identified |

## Gate

The main board, memory configuration and first external SPI assignment are now
established. New modules remain gated on an explicit pin-map assignment,
standalone diagnostic and physical observation before Bruce integration.
