# Bruce upstream snapshot

- Upstream project: Bruce firmware (`BruceDevices/firmware`)
- Supplied release: 1.16.1
- Imported on: 2026-09-03
- Local source: `/home/zawe/Загрузки/firmware-1.16.1(1)/firmware-1.16.1`
- Source Git metadata: unavailable (the supplied archive is not a Git tree)
- License: GNU AGPL-3.0; see `LICENSE`

This directory is a buildable vendor snapshot. The import intentionally excludes
upstream `media/`, `pcbs/`, `.github/`, `.vscode/`, Docker files and SD sample
payloads because they are not needed to build Obsidia and PCB work is out of
scope. `BRUCE_README.md` is the upstream README.

Obsidia changes use the board directory `boards/obsidia-v1` and named service
code under `src/obsidia`. Update the vendor snapshot by comparing a new release
against this commit, then reapplying/reviewing only those paths. Never overwrite
Obsidia paths blindly.
