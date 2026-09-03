# ST7735 standalone diagnostic

Target: photographed `1.8\" TFT 128*RGB*160 VER 1.0`, eight-pin write-only SPI
module. The first candidate profile is ST7735 black-tab, portrait rotation 0,
BGR panel order and 10 MHz SPI. These remain candidates until the required
physical color, geometry and text observation passes.

The program displays red, green, blue, black and white for 1.5 seconds each,
then leaves `OBSIDIA` / `DISPLAY OK` on a black background. No readback is
possible because this module exposes no MISO pin.
