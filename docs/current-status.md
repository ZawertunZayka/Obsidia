# Current status

Updated: 2026-09-03

## Completed

- Repository/Git audit: clean `main`, correct `origin`, initial history retained.
- Workspace structure and engineering policy established.
- Bruce 1.16.1 archive analyzed; generic ESP32-S3 PSRAM target selected as a
  structural, not electrical, base.
- UART and FPGA SPI protocol drafts documented with bounded framing and recovery
  requirements.

## Verification ledger

| Area | State | Evidence | Notes |
|---|---|---|---|
| Repository | pass | host | Git worktree and remote inspected |
| Bruce analysis | pass | host | Local source files inspected; archive is not Git |
| Main board identity | blocked on physical visibility | none | No USB device exposed to current sandbox |
| GPIO map | intentionally unset | none | Exact board/revision unknown |
| ST7735 through final stress test | not started | none | Must follow board identification |

## Gate

Software-only protocol, coprocessor and RTL work may proceed. No target GPIO,
wiring, flashing or physical pass claim is permitted until the exact ESP32-S3
board and pin exposure are established.
