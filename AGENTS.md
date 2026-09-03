# Obsidia engineering rules

These rules apply to the whole repository.

1. ESP32-S3 is the main MCU; ESP32 DevKit V1 is the radio coprocessor; Tang Nano 20K is the FPGA coprocessor.
2. Bruce runs only on the ESP32-S3.
3. The human does not write or repair project code, build configuration or Git state manually.
4. Never guess GPIO assignments. The only authoritative assignments live in `hardware/pinout/pinmap.md`.
5. Create and pass a standalone diagnostic before integrating any hardware module into Bruce.
6. Keep hardware code separate from UI; UI code must not implement SPI or UART framing.
7. Reuse sound Bruce abstractions and avoid unnecessary wrapper layers or common-code hardcoding.
8. Preserve practical upstream Bruce compatibility and add dependencies only when required.
9. Use fixed-size/bounded buffers in communication paths. Avoid allocation in hot paths.
10. ObsidiaLink must recover after radio-MCU reset. FPGA services should recover after FPGA reset where practical.
11. Never fail silently. Log initialization and protocol errors with actionable error codes.
12. Never claim a physical hardware test passed without observed evidence.
13. Before asking for wiring changes, write `ОТКЛЮЧИ ПИТАНИЕ`, then give an exact `MODULE PIN -> BOARD PIN` table.
14. Do not assume 5 V tolerance. Record module logic and supply voltages.
15. Ask for a clear photo or marking when hardware revision cannot be identified programmatically.
16. Make small logical commits. One module or feature is one logical change.
17. Update `docs/current-status.md` after every successful stage and push stable stages to `origin/main`.
18. Do not create PCB, enclosure, 3D CAD, Gerber or manufacturing work.
19. The project is limited to owned equipment, laboratory diagnostics and authorized testing.
