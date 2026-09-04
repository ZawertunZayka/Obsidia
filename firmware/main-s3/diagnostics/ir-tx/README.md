# KY-005 IR TX standalone diagnostic

This diagnostic exercises only the photographed KY-005-style transmitter on
the authoritative Obsidia GPIO17 assignment. It generates a 38 kHz carrier at
25% duty for one second, followed by one second off. The low duty and 50% burst
cycle limit average LED/GPIO current through the fitted `101` (100 ohm)
resistor while making the camera test easy to observe.

The module has no receiver or transistor driver. The fitted resistor makes the
middle header the protected ground path; do not connect it to 3.3 V or 5 V, and
do not use the `-` header because that bypasses the onboard resistor.

The diagnostic never declares a hardware pass automatically. A pass requires
observing the IR LED alternating on/off once per second through a camera known to
show IR (test the camera with a normal remote control first).
