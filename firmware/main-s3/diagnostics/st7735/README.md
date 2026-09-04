# ST7735 standalone diagnostic

Target: photographed `1.8\" TFT 128*RGB*160 VER 1.0`, eight-pin write-only SPI
module. The diagnostic drives the controller directly through Arduino `SPI` at
10 MHz using bounded stack buffers. This keeps standalone bring-up independent
of the vendored Bruce display library.

At boot the program flashes the backlight three times, displays red, green,
blue, black and white for 1.2 seconds each, then leaves `OBSIDIA` / `DISPLAY OK`
on a black background. No readback is possible because this module exposes no
MISO pin.

The 2026-09-04 connected run completed and the user reported the final text as
visible. Exact individual color naming was not separately reported.
