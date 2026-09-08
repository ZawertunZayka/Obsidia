# M5Stack CardKB standalone diagnostic

Target: photographed M5Stack Unit CardKB v1.1 (U035-B), 50 keys, I2C address
`0x5F`. The diagnostic uses the Obsidia assignment SDA GPIO40 and SCL GPIO41 at
100 kHz, probes the address once per second, repeats retained presence status
every five seconds, recovers after disconnect, and prints every non-zero key
byte without dynamic allocation.

The official Grove mapping is black GND, red 5 V, yellow SDA, white SCL. The
first Obsidia test deliberately connects red to 3.3 V because ESP32-S3 GPIO is
not 5 V tolerant and the keyboard's I2C pull-up topology is not documented. If
the keyboard does not acknowledge at 3.3 V, do not move red to 5 V directly;
use a validated bidirectional I2C level shifter.

A physical pass requires both `[OK] CARDKB_ADDRESS_ACK` and at least one
`[KEY]` line followed by `[PASS] CARDKB_I2C_KEY_RECEIVED`.
