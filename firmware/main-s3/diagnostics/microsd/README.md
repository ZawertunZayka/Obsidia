# microSD standalone stress diagnostic

Target: photographed six-pin `MicroSD Card Adapter` with an onboard 1117-class
3.3 V regulator and an LVC125A-like buffer. Its `VCC` input is powered from the
board `5VIN` pin; SPI logic at the ESP32-S3 remains 3.3 V. Do not bypass the
module regulator and do not connect any signal to 5 V.

The diagnostic first attempts mounting three times at 400 kHz to make prior
`CMD8 timeout 0x108` / `ESP_ERR_INVALID_RESPONSE` failures explicit. It then
remounts at 4 MHz and performs exactly 100 create/write/read/verify/delete
cycles using uniquely named `/.obsidia_sd_NNN.bin` temporary files. It does not
format the card or touch other paths. Each cycle verifies a deterministic 2048
byte payload and reports the precise failed operation.

The ST7735 chip-select is held high because the display shares SCK/MOSI with the
local SPI bus. A physical pass requires the final serial marker
`[PASS] SD_STRESS_100 create/write/read/verify/delete`.
