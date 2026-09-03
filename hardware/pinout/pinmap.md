# Authoritative pin map

**No GPIO assignment is currently confirmed. Do not wire from example values in
upstream Bruce board profiles.**

The generic Bruce `esp32-s3-devkitc-1` profile contains illustrative/default
pins, but these are not Obsidia assignments and must not be copied blindly.

## ESP32-S3 master

| Function | Module pin | ESP32-S3 board pin/GPIO | Supply | Logic | Status |
|---|---|---|---|---|---|
| ST7735 SPI | SCLK/MOSI/CS/DC/RST/BL | TBD | TBD | TBD | Unconfirmed |
| microSD SPI | SCLK/MOSI/MISO/CS | TBD | TBD | TBD | Unconfirmed |
| PN532 | interface pins | TBD | TBD | TBD | Unconfirmed |
| IR | TX/RX | TBD | TBD | TBD | Unconfirmed |
| keypad 4x2 | six/eight interface pins | TBD | TBD | TBD | Topology unconfirmed |
| Radio UART | TX/RX | TBD | 3.3 V domain | TBD | Unconfirmed |
| FPGA SPI | SCLK/MOSI/MISO/CS/IRQ | TBD | 3.3 V domain expected | TBD | Unconfirmed |
| native USB | D-/D+ connector routing | TBD | USB | USB | Board unknown |

## Radio ESP32

| Function | Module pin | ESP32 DevKit pin/GPIO | Supply | Logic | Status |
|---|---|---|---|---|---|
| Master UART | RX/TX | TBD | 3.3 V domain | TBD | Unconfirmed |
| RDM6300 | TX/RX if present | TBD | TBD | TBD | Unconfirmed |
| CC1101 SPI | SCLK/MOSI/MISO/CS/GDO0/GDO2 | TBD | TBD | TBD | Unconfirmed |
| NRF24 SPI | SCLK/MOSI/MISO/CSN/CE/IRQ | TBD | TBD | TBD | Unconfirmed |

## FPGA

| Function | Tang Nano pin | ESP32-S3 board pin/GPIO | I/O standard | Status |
|---|---|---|---|---|
| SPI/IRQ | SCLK/MOSI/MISO/CS/IRQ | TBD | TBD | Board revision and bank voltage unconfirmed |
