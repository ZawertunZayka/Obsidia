# Architecture

## Responsibilities

The ESP32-S3 is the system master and owns the user experience. Bruce, the
ST7735 display, 4x2 controls, microSD, PN532, IR, native USB, Wi-Fi and BLE all
run locally. `RadioService` talks to the ESP32 DevKit over ObsidiaLink/UART;
`FpgaService` talks to the Tang Nano 20K over SPI. UI code consumes service
state and never frames bus transactions.

The ESP32 DevKit runs only `obsidia-radio`. It owns RDM6300 UART and the CC1101
and NRF24 SPI devices. It must remain responsive to protocol status and reset
requests even when a peripheral is absent or faulty.

The Tang Nano 20K is an SPI slave. The first milestone exposes the immutable
device ID `OBSD`, version, status and control registers. IRQ and FIFO are later
extensions that preserve the original register ABI.

## Service boundaries

- `DisplayService`: Bruce display backend or the smallest board-specific adaptation.
- `InputService`: debounced logical events from the 4x2 controls.
- `StorageService`: SD initialization, diagnostics and filesystem operations.
- `NfcService`: PN532 discovery and operations.
- `IrService`: IR transmit/receive diagnostics and Bruce integration.
- `RadioService`: typed ObsidiaLink client; no UI responsibilities.
- `FpgaService`: register, health, IRQ and FIFO access; no UI responsibilities.

Existing Bruce abstractions take precedence where they already satisfy these
boundaries.

## Recovery and observability

Both coprocessor links use bounded operations and explicit health state. The
master re-runs version/capability discovery after a peer restart. CRC, timeout,
malformed-frame and peripheral initialization errors are visible to diagnostics;
absence of hardware is not converted to success.
