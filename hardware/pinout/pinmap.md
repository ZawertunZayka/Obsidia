# Authoritative pin map

**No GPIO assignment is currently confirmed. Do not wire from example values in
upstream Bruce board profiles.**

The generic Bruce `esp32-s3-devkitc-1` profile contains illustrative/default
pins, but these are not Obsidia assignments and must not be copied blindly.

The main module is now confirmed as ESP32-S3 N16R8. Its photographed 44-pin
carrier follows the ESP32-S3-DevKitC-1 header order and the right-hand USB-C is
a working QinHeng USB-UART bridge. These board-owned signals are reserved:

| GPIO / signal | Board use | Status |
|---|---|---|
| GPIO43 / U0TXD | USB-UART console TX | Confirmed by successful ROM/flash access |
| GPIO44 / U0RXD | USB-UART console RX | Confirmed by successful ROM/flash access |
| GPIO19 / USB D- | Native USB connector | Confirmed by native USB Serial/JTAG flashing |
| GPIO20 / USB D+ | Native USB connector | Confirmed by native USB Serial/JTAG flashing |
| GPIO0 | BOOT strap/button | Visible on carrier; reserve from peripherals |
| GPIO3, GPIO45, GPIO46 | Strapping pins | Reserve until boot-level requirements are reviewed |
| GPIO48 | Onboard RGB candidate | Visible beside RGB circuit; electrical mapping untested |

The first external assignment is the standalone ST7735 bring-up bus. SCLK and
MOSI are reserved as a future shared local-peripheral SPI bus; GPIO13 is held
for its MISO even though this display does not expose one.

## ESP32-S3 master

| Function | Module pin | ESP32-S3 board pin/GPIO | Supply | Logic | Status |
|---|---|---|---|---|---|
| ST7735 SPI | SCK/SDA/CS/DC/RES/BL | GPIO12/GPIO11/GPIO10/GPIO9/GPIO14/GPIO21 | 3.3 V | 3.3 V | Confirmed: final diagnostic screen visible |
| Shared local SPI | MISO | GPIO13 | 3.3 V | 3.3 V | Reserved for later SD/PN532 use |
| microSD SPI | SCLK/MOSI/MISO/CS | TBD | TBD | TBD | Unconfirmed |
| PN532 | interface pins | TBD | TBD | TBD | Unconfirmed |
| IR | TX/RX | TBD | TBD | TBD | Unconfirmed |
| keypad 4x2 | six/eight interface pins | TBD | TBD | TBD | Topology unconfirmed |
| Radio UART | TX/RX | TBD | 3.3 V domain | TBD | Unconfirmed |
| FPGA SPI | SCLK/MOSI/MISO/CS/IRQ | TBD | 3.3 V domain expected | TBD | Unconfirmed |
| native USB | GPIO19/GPIO20 connector routing | GPIO19/GPIO20 | USB | USB | Confirmed by flash transport |

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
