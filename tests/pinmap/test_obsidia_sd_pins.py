from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def require(text: str, expected: str, source: Path) -> None:
    if expected not in text:
        raise AssertionError(f"{source}: missing {expected!r}")


board = ROOT / "firmware/main-s3/boards/OBSIDIA_V1/OBSIDIA_V1.ini"
diagnostic = ROOT / "firmware/main-s3/diagnostics/shared-spi/src/main.cpp"
pinmap = ROOT / "hardware/pinout/pinmap.md"

board_text = board.read_text()
diagnostic_text = diagnostic.read_text()
pinmap_text = pinmap.read_text()

for expected in (
    "-DSDCARD_SCK=18",
    "-DSDCARD_MISO=46",
    "-DSDCARD_MOSI=3",
    "-DSDCARD_CS=17",
    "-DTXLED=-1",
    "-DIR_TX_PINS='{}'",
):
    require(board_text, expected, board)

for expected in (
    "constexpr int kSdSck = 18;",
    "constexpr int kSdMiso = 46;",
    "constexpr int kSdMosi = 3;",
    "constexpr int kSdCs = 17;",
):
    require(diagnostic_text, expected, diagnostic)

require(pinmap_text, "GPIO18/GPIO3/GPIO46/GPIO17", pinmap)
require(board_text, "-DTFT_DC=9", board)
interface = ROOT / "firmware/main-s3/boards/OBSIDIA_V1/interface.cpp"
require(interface.read_text(), "bruceConfigPins.setIrTxPin(-1);", interface)
if diagnostic_text.index("display.init()") > diagnostic_text.index("SD.begin("):
    raise AssertionError("diagnostic must initialize the display even when SD.begin fails")

print("OBSIDIA SD/TFT pin assignments are consistent")
