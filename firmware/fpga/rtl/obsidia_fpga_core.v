`timescale 1ns/1ps
`default_nettype none

module obsidia_fpga_core (
    input  wire       reset_n,
    input  wire       system_clk,
    input  wire       fifo_write_enable,
    input  wire [7:0] fifo_write_data,
    output wire       fifo_write_full,
    input  wire       event_pending,
    input  wire       external_error_pending,
    input  wire       spi_sclk,
    input  wire       spi_cs_n,
    input  wire       spi_mosi,
    output wire       spi_miso,
    output wire       irq,
    output wire       event_ack_toggle
);
    wire fifo_empty;
    wire fifo_full_read_domain;
    wire [7:0] fifo_data;
    wire [4:0] fifo_level;
    wire fifo_pop;
    wire fifo_clear_toggle;

    obsidia_async_fifo fifo (
        .reset_n(reset_n),
        .clear_toggle_async(fifo_clear_toggle),
        .wr_clk(system_clk),
        .wr_enable(fifo_write_enable),
        .wr_data(fifo_write_data),
        .wr_full(fifo_write_full),
        .rd_clk(spi_sclk),
        .rd_enable(fifo_pop),
        .rd_data(fifo_data),
        .rd_empty(fifo_empty),
        .rd_full(fifo_full_read_domain),
        .rd_level(fifo_level)
    );

    obsidia_spi_regs registers (
        .reset_n(reset_n),
        .spi_sclk(spi_sclk),
        .spi_cs_n(spi_cs_n),
        .spi_mosi(spi_mosi),
        .spi_miso(spi_miso),
        .data_ready_pending(!fifo_empty),
        .buffer_full_pending(fifo_full_read_domain),
        .event_pending(event_pending),
        .external_error_pending(external_error_pending),
        .fifo_empty(fifo_empty),
        .fifo_data(fifo_data),
        .fifo_level(fifo_level),
        .fifo_pop(fifo_pop),
        .fifo_clear_toggle(fifo_clear_toggle),
        .irq(irq),
        .event_ack_toggle(event_ack_toggle)
    );
endmodule

`default_nettype wire
