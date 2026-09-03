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
- FPGA SPI mode 0 slave, base `OBSD` register map and IRQ aggregation implemented
  without pin constraints; FIFO remains a separate stage.

## Verification ledger

| Area | State | Evidence | Notes |
|---|---|---|---|
| Repository | pass | host | Git worktree and remote inspected |
| Bruce analysis | pass | host | Local source files inspected; archive is not Git |
| Bruce control build | pass | build | PlatformIO success; 3,701,374 B flash, 128,320 B RAM |
| ObsidiaLink codec/parser | pass | host | CRC vector, framing, bounds, timeout and resynchronization tests pass |
| Radio MCU firmware | pass | build | PlatformIO `obsidia-radio` success; 289,780 B flash, 22,244 B RAM |
| Radio UART transport | pending hardware | none | GPIO deliberately unset until board and wiring confirmation |
| FPGA base register RTL | pass | simulation | Icarus testbench, Yosys structural check and Verilator lint pass |
| FPGA IRQ RTL | pass | simulation | Pending/status/IRQ consistency and acknowledgement toggle tested |
| FPGA synthesis/bitstream | blocked on board revision | none | Device/package and pin constraints intentionally unset |
| Main board identity | blocked on physical visibility | none | No USB device exposed to current sandbox |
| GPIO map | intentionally unset | none | Exact board/revision unknown |
| ST7735 through final stress test | not started | none | Must follow board identification |

## Gate

Software-only protocol, coprocessor and RTL work may proceed. No target GPIO,
wiring, flashing or physical pass claim is permitted until the exact ESP32-S3
board and pin exposure are established.
