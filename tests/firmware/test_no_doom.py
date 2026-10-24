#!/usr/bin/env python3
"""Regression guard: production Bruce must not contain the Obsidia DOOM port."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
FW = ROOT / "firmware" / "main-s3"


def main() -> None:
    forbidden_paths = (
        FW / "lib" / "ObsidiaDoom",
        FW / "src" / "obsidia" / "doom" / "ObsidiaDoom.cpp",
        FW / "src" / "core" / "menu_items" / "GamesMenu.cpp",
        FW / "src" / "core" / "menu_items" / "GamesMenu.h",
    )
    existing = [str(path.relative_to(ROOT)) for path in forbidden_paths if path.exists()]
    assert not existing, f"DOOM files still present: {existing}"

    forbidden_tokens = {
        FW / "build.py": ("doom_wad", "doom1-cut", "upload-doomwad", "Obsidia Doom"),
        FW / "src" / "main.cpp": ("obsidia_doom", "[DOOM]"),
        FW / "src" / "core" / "main_menu.cpp": ("gamesMenu",),
        FW / "src" / "core" / "main_menu.h": ("GamesMenu", "gamesMenu"),
        FW / "custom_16Mb.csv": ("doomwad",),
    }
    found = []
    for path, tokens in forbidden_tokens.items():
        content = path.read_text(encoding="utf-8")
        found.extend(f"{path.relative_to(ROOT)}: {token}" for token in tokens if token in content)
    assert not found, "DOOM integration remains:\n" + "\n".join(found)

    print("Obsidia Bruce contains no DOOM integration")


if __name__ == "__main__":
    main()
