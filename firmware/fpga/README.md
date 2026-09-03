# obsidia-fpga

Synthesizable RTL for the Tang Nano 20K FPGA coprocessor. The current milestone
implements an SPI mode 0 slave, the base register ABI, level-sensitive IRQ
aggregation and a bounded 16-byte asynchronous FIFO from
`protocol/fpga-spi.md`.

No physical pin constraints are supplied yet: the exact Tang Nano 20K revision,
bank voltage and master wiring are unconfirmed. Adding guessed constraints is
forbidden by `AGENTS.md`.

Run the behavioral test with an OSS CAD Suite on `PATH`:

```sh
make test
```

The tests cover burst reads, `OBSD` identity, version/status, IRQ assertion and
clear, CONTROL acknowledgement, invalid-address error latching, FIFO clock-domain
crossing, ordering, clear, full/empty and overflow rejection. A simulator pass
is not evidence of a physical hardware pass.
