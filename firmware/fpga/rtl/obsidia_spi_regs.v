`timescale 1ns/1ps
`default_nettype none

module obsidia_spi_regs (
    input  wire reset_n,
    input  wire spi_sclk,
    input  wire spi_cs_n,
    input  wire spi_mosi,
    output reg  spi_miso,
    input  wire data_ready_pending,
    input  wire buffer_full_pending,
    input  wire event_pending,
    input  wire external_error_pending,
    output wire irq,
    output reg  event_ack_toggle
);
    localparam [7:0] VERSION_MAJOR = 8'd1;
    localparam [7:0] VERSION_MINOR = 8'd0;

    reg [2:0] bit_count;
    reg [6:0] rx_shift;
    reg [7:0] tx_shift;
    reg [6:0] address;
    reg       data_phase;
    reg       read_mode;
    reg       error_latched;
    wire [7:0] current_register_data;
    wire       transaction_reset;

    function [7:0] read_register;
        input [6:0] register_address;
        begin
            case (register_address)
                7'h00: read_register = 8'h4f; // O
                7'h01: read_register = 8'h42; // B
                7'h02: read_register = 8'h53; // S
                7'h03: read_register = 8'h44; // D
                7'h04: read_register = VERSION_MAJOR;
                7'h05: read_register = VERSION_MINOR;
                7'h06: read_register = {
                    3'b000,
                    (error_latched | external_error_pending),
                    event_pending,
                    buffer_full_pending,
                    data_ready_pending,
                    1'b1
                };
                7'h07: read_register = 8'h00;
                default: read_register = 8'h00;
            endcase
        end
    endfunction

    assign current_register_data = read_register(address);
    assign transaction_reset = spi_cs_n | ~reset_n;
    assign irq = data_ready_pending | buffer_full_pending | event_pending |
                 external_error_pending | error_latched;

    function valid_address;
        input [6:0] register_address;
        begin
            valid_address = (register_address <= 7'h07);
        end
    endfunction

    // MOSI is sampled on the rising edge (SPI mode 0). CS resets the
    // transaction parser without clearing persistent status registers.
    always @(posedge spi_sclk or posedge transaction_reset) begin
        if (transaction_reset) begin
            bit_count     <= 3'd0;
            rx_shift      <= 7'h00;
            address       <= 7'h00;
            data_phase    <= 1'b0;
            read_mode     <= 1'b0;
        end else begin
            rx_shift <= {rx_shift[5:0], spi_mosi};

            if (!data_phase) begin
                if (bit_count == 3'd7) begin
                    read_mode  <= rx_shift[6];
                    address    <= {rx_shift[5:0], spi_mosi};
                    data_phase <= 1'b1;
                    bit_count  <= 3'd0;
                end else begin
                    bit_count <= bit_count + 1'b1;
                end
            end else begin
                if (bit_count == 3'd7) begin
                    address   <= address + 1'b1;
                    bit_count <= 3'd0;
                end else begin
                    bit_count <= bit_count + 1'b1;
                end
            end
        end
    end

    // Persistent registers live in the SPI clock domain but survive CS edges.
    always @(posedge spi_sclk or negedge reset_n) begin
        if (!reset_n) begin
            error_latched <= 1'b0;
            event_ack_toggle <= 1'b0;
        end else if (!spi_cs_n) begin
            if (!data_phase && bit_count == 3'd7 && rx_shift[6] &&
                !valid_address({rx_shift[5:0], spi_mosi}))
                error_latched <= 1'b1;

            if (data_phase && read_mode && bit_count == 3'd0 && !valid_address(address))
                error_latched <= 1'b1;

            if (data_phase && !read_mode && bit_count == 3'd7) begin
                if (address == 7'h07) begin
                    if (rx_shift[6]) begin
                        error_latched <= 1'b0;
                    end else begin
                        if (spi_mosi) begin
                            error_latched <= 1'b0;
                            event_ack_toggle <= ~event_ack_toggle;
                        end
                        if (({rx_shift, spi_mosi} & 8'h7c) != 8'h00)
                            error_latched <= 1'b1;
                    end
                end else begin
                    error_latched <= 1'b1;
                end
            end
        end
    end

    // MISO changes on falling edges so the master sees stable data on rising
    // edges. tx_shift is intentionally owned by this clocked block only.
    always @(negedge spi_sclk or posedge transaction_reset) begin
        if (transaction_reset) begin
            spi_miso <= 1'b0;
            tx_shift <= 8'h00;
        end else if (data_phase && read_mode) begin
            if (bit_count == 3'd0) begin
                spi_miso <= current_register_data[7];
                tx_shift <= {current_register_data[6:0], 1'b0};
            end else begin
                spi_miso <= tx_shift[7];
                tx_shift <= {tx_shift[6:0], 1'b0};
            end
        end else begin
            spi_miso <= 1'b0;
            tx_shift <= 8'h00;
        end
    end
endmodule

`default_nettype wire
