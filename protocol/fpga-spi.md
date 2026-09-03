# ESP32-S3 to FPGA SPI protocol

Status: version 1 base register ABI implemented and behaviorally tested in SPI
mode 0. Electrical pins and final frequency remain unconfirmed until the exact
Tang Nano 20K revision and wiring are known.

The ESP32-S3 is master and the FPGA is slave. Each transaction begins with one
command byte: bit 7 is `1` for read and `0` for write; bits 6..0 are the register
address. The following bytes carry register data. CS delimits and resets the
transaction parser. Multi-byte registers use big-endian byte order so DEVICE_ID
is visibly transferred as `O`, `B`, `S`, `D`.

## Initial registers

| Address | Name | Width | Access | Reset / meaning |
|---:|---|---:|---|---|
| `0x00` | DEVICE_ID | 4 | R | ASCII `OBSD` (`0x4F425344`) |
| `0x04` | VERSION | 2 | R | major, minor |
| `0x06` | STATUS | 1 | R | health/event flags |
| `0x07` | CONTROL | 1 | R/W | control requests |

The four-byte DEVICE_ID occupies addresses `0x00..0x03`; this resolves the
ambiguity of treating `0x01` simultaneously as VERSION. Registers auto-increment
within a CS-low burst and invalid addresses read as zero while setting ERROR.

## Flags

STATUS bits: bit 0 `READY`, bit 1 `DATA_READY`, bit 2 `BUFFER_FULL`, bit 3
`EVENT`, bit 4 `ERROR`; remaining bits are zero. IRQ is asserted when any enabled
event bit is set and deasserted after the event is acknowledged. IRQ/FIFO details
will extend this ABI without moving initial registers.

CONTROL bit 0 requests event acknowledgement; bit 1 requests FIFO clear; bit 7
requests soft reset. Reserved bits must be written as zero. Request bits are
self-clearing once their operation exists; before IRQ/FIFO integration, bits
0..1 provide readback for register-interface diagnostics. Soft reset clears the
CONTROL readback and latched protocol error.

After reset, the master validates DEVICE_ID and VERSION before trusting state.
Identity mismatch, timeout or repeated invalid reads marks the FPGA unavailable
and starts bounded rediscovery.
