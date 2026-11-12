# Bruce upstream snapshot

- Upstream project: https://github.com/BruceDevices/firmware
- Imported release: 1.17
- Upstream commit: `2b6ba4d490bfcf94ce2574c6154537dbd5838c3d`
- Imported on: 2026-11-12
- Previous baseline: supplied Bruce 1.16.1 archive, imported 2026-09-03
- License: GNU AGPL-3.0; see `LICENSE`

The buildable firmware source follows upstream Bruce 1.17. Git metadata, CI
configuration, Docker files, media, PCB files and sample SD payloads remain
excluded. Upstream submodule contents needed for PlatformIO are vendored as
regular source files so this project stays self-contained.

Obsidia-specific code remains in `boards/OBSIDIA_V1`, `src/obsidia`, the shared
`../common/obsidialink` library, and the standalone diagnostic folders. Changes
to upstream files are reviewed merges against the 1.16.1 baseline; preserve
those board integrations when refreshing Bruce again.
