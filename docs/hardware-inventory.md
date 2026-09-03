# Hardware inventory

| Role | Reported hardware | Revision / identity | Voltage status | Verification |
|---|---|---|---|---|
| Main MCU | ESP32-S3-WROOM N16R8, 16 MB flash, 8 MB octal PSRAM | 44-pin dual-USB DevKitC-1-compatible carrier; exact vendor/revision unknown | 3.3 V I/O; carrier power paths unverified | Module marking supplied by user; carrier front photo |
| Radio MCU | ESP32 DevKit V1, 38 pin | Exact module/revision unknown | 3.3 V logic expected; unverified | User report only |
| FPGA | Tang Nano 20K, GW2AR-18 | Exact board revision unknown | Unverified | User report only |
| Display | ST7735, 1.8 in, 128x160 SPI | Tab/controller variant unknown | Unverified | User report only |
| Storage | SPI microSD module with LVC125A-like buffer | Exact schematic/revision unknown | Unverified | User report; prior CMD8 failures |
| NFC | PN532, 13.56 MHz | Interface selector/revision unknown | Unverified | User report only |
| LF RFID | RDM6300, 125 kHz EM4100 | Revision unknown | Unverified | User report only |
| Sub-GHz | CC1101, 433 MHz | Module revision unknown | Unverified | User report only |
| 2.4 GHz | NRF24L01+ PA/LNA | Module revision unknown | Supply quality critical; unverified | User report only |
| Infrared | IR TX/RX module | Parts/revision unknown | Unverified | User report only |
| Controls | 4x2 buttons/keypad | Matrix/common topology unknown | Unverified | User report only |

No USB serial device was visible from the current sandbox during the initial
audit, so chip, flash and PSRAM probing has not yet produced hardware evidence.

The NRF24L01+ PA/LNA module must not be treated as 5 V tolerant. Its exact
module revision and regulator are unverified, so both supply and I/O remain
unassigned. Before a connected test, provide a stable verified 3.3 V rail and
local bulk plus ceramic decoupling suitable for PA current transients.
