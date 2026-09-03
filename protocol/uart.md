# ObsidiaLink UART protocol

Status: protocol version 1; codec/parser host tests pass, hardware transport is
pending confirmed UART GPIO and physical loop testing.

## Frame

All multibyte integers are little-endian.

| Field | Size | Meaning |
|---|---:|---|
| SOF | 1 | `0xAA` |
| LENGTH | 1 | bytes from `COMMAND` through the end of `PAYLOAD`; range 1..65 |
| COMMAND | 1 | command/response identifier |
| PAYLOAD | 0..64 | command-specific data |
| CRC16 | 2 | CRC-16/CCITT-FALSE over `LENGTH`, `COMMAND`, `PAYLOAD` |

CRC parameters: polynomial `0x1021`, initial value `0xFFFF`, refin/refout false,
xorout `0x0000`. Maximum encoded frame size is 69 bytes. Receivers use fixed
buffers and reject zero or oversized lengths.

## Identifiers

| Name | Value | Direction |
|---|---:|---|
| `PING` | `0x01` | master -> radio |
| `GET_VERSION` | `0x02` | master -> radio |
| `GET_STATUS` | `0x03` | master -> radio |
| `RESET` | `0x04` | master -> radio |
| `ACK` | `0x80` | radio -> master |
| `ERROR` | `0x81` | radio -> master |
| `DATA` | `0x82` | radio -> master |

`PING` receives `ACK` with payload `[request-command, protocol-version]`.
`GET_VERSION` receives `DATA` with `[request-command, protocol-version,
firmware-major, firmware-minor, firmware-patch]`. `GET_STATUS` receives `DATA`
whose first two bytes are request command and protocol version. `RESET` must be
acknowledged before a delayed restart.

## Errors

`ERROR` payload is `[request-command-or-0, protocol-version, error-code]`.

| Code | Name |
|---:|---|
| `0x01` | unsupported command |
| `0x02` | invalid payload |
| `0x03` | bad CRC |
| `0x04` | malformed frame |
| `0x05` | busy |
| `0x06` | peripheral unavailable |
| `0x07` | internal failure |

## Parser and recovery

The receiver consumes bytes incrementally. Outside a frame it ignores bytes
until SOF. Invalid length immediately returns to SOF search. CRC or malformed
frames are reported and discarded. A SOF encountered after a framing error may
start a new frame. An incomplete frame is abandoned after 100 ms from its most
recent byte. The master retries idempotent discovery requests after timeout and
re-runs version/status discovery when the peer restarts or stops responding.

`GET_STATUS` version 1 returns `[request-command, protocol-version, ready-bits,
supported-bits]`. Bits 0..2 represent RDM6300, CC1101 and NRF24 respectively.
A supported bit describes compiled capability; a ready bit is set only after the
peripheral's standalone diagnostic has passed during the current boot.
