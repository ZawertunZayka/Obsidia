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
| `0x08` | FIFO_DATA | 1 | R | oldest byte; completed read pops it |
| `0x09` | FIFO_LEVEL | 1 | R | synchronized readable byte count, 0..16 |
| `0x0A` | FIFO_CAPACITY | 1 | R | `16` |

The four-byte DEVICE_ID occupies addresses `0x00..0x03`; this resolves the
ambiguity of treating `0x01` simultaneously as VERSION. Registers auto-increment
within a CS-low burst and invalid addresses read as zero while setting ERROR.

## Flags

STATUS bits: bit 0 `READY`, bit 1 `DATA_READY`, bit 2 `BUFFER_FULL`, bit 3
`EVENT`, bit 4 `ERROR`; remaining bits are zero. IRQ is asserted when any enabled
event bit is set and deasserted after the event is acknowledged or FIFO data is
drained.

IRQ sources are level-sensitive. The owning producer holds its pending level
until it observes a change on the `event_ack_toggle` signal; this avoids relying
on a narrow pulse across clock domains. Protocol/register errors are latched in
the SPI domain and cleared directly by acknowledgement. IRQ is the OR of status
bits 1..4 and therefore cannot silently disagree with STATUS.

CONTROL bit 0 requests event acknowledgement; bit 1 requests FIFO clear; bit 7
requests soft reset. Reserved bits must be written as zero. Request bits are
self-clearing requests and always read as zero. Bit 0 clears the local protocol
error and changes `event_ack_toggle`; each external event producer synchronizes
that toggle before clearing its own pending level. Soft reset clears the local
protocol error.

`FIFO_DATA` is the sole auto-increment exception: while CS stays low, successive
data bytes continue reading address `0x08` and pop consecutive FIFO entries.
Reading an empty FIFO returns zero without underflowing. The FIFO is 16 bytes,
rejects writes while full and crosses from the FPGA system clock into the SPI
clock through Gray-coded pointers and two-stage synchronizers. CONTROL bit 1
changes a clear toggle synchronized independently into both FIFO clock domains.

After reset, the master validates DEVICE_ID and VERSION before trusting state.
Identity mismatch, timeout or repeated invalid reads marks the FPGA unavailable
and starts bounded rediscovery.
