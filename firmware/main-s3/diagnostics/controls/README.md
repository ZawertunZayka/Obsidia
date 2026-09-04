# Controls standalone diagnostic

Target: photographed `8 Push Buttons V1.1` passive board with eight independent
normally-open K1-K8 contacts and one common `G` contact. No VCC is connected.
Each key input uses the ESP32-S3 internal 3.3 V pull-up and becomes active-low
when its button connects it to common ground.

The diagnostic uses the shared hardware-independent `InputService`: 25 ms
debounce, press/release events and a one-shot long-press event after 600 ms.
It reports `[PASS] ALL 8 KEYS OBSERVED` only after seeing a debounced press on
every input. Queue overflow is explicit and prevents silent input loss.

The assignment is authoritative in `hardware/pinout/pinmap.md`. Do not infer a
different order from physical button placement; use the K1-K8 silkscreen.
The confirmed connector mapping is K1/K2/K3/K4/K5/K6/K7/K8 to
GPIO16/GPIO15/GPIO7/GPIO6/GPIO5/GPIO4/GPIO2/GPIO1 respectively.
