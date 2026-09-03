# Hardware inventory

| Role | Reported hardware | Revision / identity | Voltage status | Verification |
|---|---|---|---|---|
| Main MCU | ESP32-S3-WROOM N16R8, QFN56 rev v0.2, 16 MB quad flash, 8 MB embedded PSRAM | 44-pin dual-USB DevKitC-1-compatible carrier with QinHeng USB-UART; exact carrier vendor/revision unmarked | 3.3 V I/O; flash/PSRAM both 3.3 V | Photo, ROM probe, JEDEC probe and runtime diagnostic |
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

The carrier's right-hand USB-C connector (viewed with antenna at the top) is a
QinHeng `1A86:55D3` USB-UART bridge and enumerates as `/dev/ttyACM0`. The other
USB-C connector has not yet been electrically exercised. A full pre-Obsidia
16 MB flash backup is retained locally under `.device-backups/`; binary images
are excluded from Git. Its SHA-256 is
`ad355bae8e4dcb07fc12b1286ee2de1b5a7c09eed50fac493dfaf92b67960c74`.

The NRF24L01+ PA/LNA module must not be treated as 5 V tolerant. Its exact
module revision and regulator are unverified, so both supply and I/O remain
unassigned. Before a connected test, provide a stable verified 3.3 V rail and
local bulk plus ceramic decoupling suitable for PA current transients.
