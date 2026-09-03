# Bruce integration analysis

## Source and version

The supplied source is the extracted directory
`/home/zawe/Загрузки/firmware-1.16.1(1)/firmware-1.16.1`. It is not a Git working
tree. The archive name identifies Bruce 1.16.1; the source itself uses the
build-time `BRUCE_VERSION` macro and defaults it to `dev`, so the archive name is
currently the only local release-version evidence.

Bruce is AGPL-3.0 licensed. The controlled integration will retain its license
and notices and record the source path/version. Obsidia-specific commits and
board files remain identifiable in this repository.

## Build and layout

Bruce uses PlatformIO with Arduino-ESP32. `platformio.ini` loads board fragments
from `boards/*.ini` and `boards/*/*.ini`; board-local `interface.cpp` implements
hardware hooks while flags define display, storage and peripheral pins. Core code
is under `src/core`, feature implementations under `src/modules`, bundled
libraries under `lib`, and custom PlatformIO board JSON files under
`boards/_boards_json`.

Shared headers live in the small top-level `include` directory, alongside
core/modules and board-local headers.

## Best starting target

`boards/ESP-General` / environment `esp32-s3-devkitc-1-psram` is the correct
structural base because it is a generic ESP32-S3 target, uses an 8 MB partition
table, enables native USB HID, and has an explicit 8 MB octal-PSRAM variant.
It is not an electrical pin map: all example GPIO values must be replaced only
after the physical main board is identified. The LilyGo T-Display S3 backend is
useful as an input/display implementation example but is too board-specific to
inherit directly.

## Existing architecture

- Display: global TFT_eSPI-style display object and helpers in
  `src/core/display.*`; controller/pin flags are board-specific.
- Input: board `interface.cpp` populates Bruce logical press flags through
  `InputHandler`; this is the right extension point for the 4x2 controls.
- Storage: `src/core/sd_functions.*`, filesystem globals and bus setup in the
  HAL; SD SPI pins are configuration flags.
- Bus arbitration: `src/core/bus_HAL.*` and pin structures in
  `src/core/configPins.*` centralize SPI/I2C access.
- NFC/RFID: menu in `src/core/menu_items/RFIDMenu.*`; implementations under
  `src/modules/rfid`, including Adafruit PN532 support over configured buses.
- IR: menu in `src/core/menu_items/IRMenu.*` and modules under
  `src/modules/ir`, using IRremoteESP8266 and configured TX/RX choices.
- Sub-GHz: `src/core/menu_items/RFMenu.*` and `src/modules/rf`; CC1101 is already
  supported through the SmartRC driver and configurable SPI pins.
- NRF24: menu and `src/modules/NRF24`; RF24 dependency and configurable SPI pins
  already exist.
- Wi-Fi/BLE: native modules and menu integration exist under `src/modules/wifi`,
  `src/modules/ble` and the corresponding core menus.
- USB: ESP32-S3 native USB HID paths use `USB_as_HID`; mass storage support uses
  Arduino `USB`/`USBMSC` when USB OTG is supported.
- Settings/menu: persisted settings live in `src/core/config.*` and
  `settings.*`; menu composition lives in `src/core/main_menu.*` and typed menu
  item classes.

## Obsidia extension points

Create an `OBSIDIA_V1` board directory and PlatformIO environment derived from
the generic PSRAM target only after board identity and pins are confirmed. Its
`interface.cpp` should own display/input initialization and delegate radio/FPGA
transport to board-specific services. Local PN532, IR and SD should reuse Bruce
bus and feature modules. CC1101 and NRF24 execution must be redirected through
`RadioService` instead of instantiating local SPI drivers. Diagnostics should be
a board-specific menu item consuming service health snapshots.

Do not fork the Bruce menu, display, settings, Wi-Fi, BLE or USB subsystems merely
to rename them. Keep Obsidia transport and diagnostics isolated so upstream
updates remain reviewable.

The transport-independent `RadioService` and `FpgaService` live under
`src/obsidia/services`. `ArduinoTransports` adapts Bruce/Arduino `Stream` and
`SPIClass` without owning bus pin selection. The future `OBSIDIA_V1` board
interface must supply confirmed serial/SPI instances, CS GPIO and SPI frequency;
an unset CS or zero frequency is rejected instead of silently starting a bus.

## Controlled copy

The buildable snapshot lives directly in `firmware/main-s3`. Its
`OBSIDIA_UPSTREAM.md` records provenance and exclusions. Build-relevant upstream
source is vendored so a local archive disappearing does not break the project;
Obsidia additions are constrained to clearly named board/service paths and
logical commits so future release updates remain reviewable.

The imported `patch.py` required one compatibility fix: it now resolves
`objcopy` from PlatformIO's installed modern toolchain package instead of
invoking a removed legacy package name. It also fails before replacing
`libnet80211.a` if patch generation fails. This is an Obsidia-maintained build
fix, not a board-specific hardware change.
