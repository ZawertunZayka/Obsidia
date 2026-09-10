# Doom integration

Obsidia includes the PrBoom 2.5.0 engine from the user-supplied local copy of
Espressif's `esp32-doom` proof of concept. The engine is isolated in
`firmware/main-s3/lib/ObsidiaDoom`; Bruce-specific display and CardKB adapters
live in `src/obsidia/doom`.

## Internal flash layout

The `OBSIDIA_V1` partition table reserves the 3 MiB read-only `doomwad`
partition at `0x600000`. The bundled `doom1-cut.wad` is 3,114,603 bytes and is
written directly to that partition. Doom does not read the microSD card.

The combined `Bruce-OBSIDIA_V1.bin` contains the bootloader, partition table,
Bruce application and WAD. For development uploads, flash the application and
then run:

```sh
pio run -e OBSIDIA_V1 -t upload-doomwad --upload-port /dev/ttyACM0
```

## Display and controls

The 320x240 indexed Doom framebuffer is downscaled 2:1 to 160x120 and centered
on the 160x128 landscape ST7735 display.

CardKB mappings:

| CardKB key | Doom action |
|---|---|
| Arrows or W/A/S/D | Move/turn |
| Space or F | Fire |
| E | Use/open |
| Enter | Menu confirm |
| Esc or Q | Doom menu/back |
| M | Map |
| R | Run |
| 1-9 | Weapon selection |

The upstream proof of concept has no sound, saved games, or multiplayer. Its
PrBoom code remains GPL-2.0-only; the included `COPYING` and `AUTHORS` files
are preserved. The bundled WAD is the cut/shareware asset supplied with the
local Espressif source, not a commercial game WAD.
