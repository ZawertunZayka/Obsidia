# obsidia-fpga

Synthesizable RTL for the Tang Nano 20K FPGA coprocessor. The current milestone
implements an SPI mode 0 slave and the base register ABI from
`protocol/fpga-spi.md`.

No physical pin constraints are supplied yet: the exact Tang Nano 20K revision,
bank voltage and master wiring are unconfirmed. Adding guessed constraints is
forbidden by `AGENTS.md`.

Run the behavioral test with an OSS CAD Suite on `PATH`:

```sh
make test
```

The test covers burst reads, `OBSD` identity, version/status, CONTROL readback,
invalid-address error latching and soft reset. A simulator pass is not evidence
of a physical hardware pass.
