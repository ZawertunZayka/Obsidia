IVERILOG ?= iverilog
VVP ?= vvp
BUILD_DIR ?= build

.PHONY: test clean

test:
	mkdir -p $(BUILD_DIR)
	$(IVERILOG) -g2012 -Wall -Wimplicit -s obsidia_spi_regs_tb \
		-o $(BUILD_DIR)/obsidia_spi_regs_tb.vvp \
		rtl/obsidia_spi_regs.v tb/obsidia_spi_regs_tb.v
	$(VVP) $(BUILD_DIR)/obsidia_spi_regs_tb.vvp

clean:
	rm -rf $(BUILD_DIR)
