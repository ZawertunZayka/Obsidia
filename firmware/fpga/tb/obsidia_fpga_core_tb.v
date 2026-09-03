`timescale 1ns/1ps
`default_nettype none

module obsidia_fpga_core_tb;
    reg reset_n = 1'b1;
    reg system_clk = 1'b0;
    reg fifo_write_enable = 1'b0;
    reg [7:0] fifo_write_data = 8'h00;
    wire fifo_write_full;
    reg event_pending = 1'b0;
    reg external_error_pending = 1'b0;
    reg spi_sclk = 1'b0;
    reg spi_cs_n = 1'b1;
    reg spi_mosi = 1'b0;
    wire spi_miso;
    wire irq;
    wire event_ack_toggle;

    reg [7:0] received;
    integer failures = 0;
    integer fifo_index;

    always #3 system_clk = ~system_clk;

    obsidia_fpga_core dut (
        .reset_n(reset_n),
        .system_clk(system_clk),
        .fifo_write_enable(fifo_write_enable),
        .fifo_write_data(fifo_write_data),
        .fifo_write_full(fifo_write_full),
        .event_pending(event_pending),
        .external_error_pending(external_error_pending),
        .spi_sclk(spi_sclk),
        .spi_cs_n(spi_cs_n),
        .spi_mosi(spi_mosi),
        .spi_miso(spi_miso),
        .irq(irq),
        .event_ack_toggle(event_ack_toggle)
    );

    task fifo_write;
        input [7:0] value;
        begin
            @(negedge system_clk);
            fifo_write_data = value;
            fifo_write_enable = 1'b1;
            @(negedge system_clk);
            fifo_write_enable = 1'b0;
        end
    endtask

    task spi_byte;
        input [7:0] transmitted;
        output [7:0] captured;
        integer bit_index;
        begin
            for (bit_index = 7; bit_index >= 0; bit_index = bit_index - 1) begin
                spi_mosi = transmitted[bit_index];
                #5 spi_sclk = 1'b1;
                #5 captured[bit_index] = spi_miso;
                #5 spi_sclk = 1'b0;
                #5;
            end
        end
    endtask

    task begin_transaction;
        begin
            spi_sclk = 1'b0;
            spi_cs_n = 1'b0;
            #5;
        end
    endtask

    task end_transaction;
        begin
            spi_cs_n = 1'b1;
            spi_mosi = 1'b0;
            #10;
        end
    endtask

    task expect_byte;
        input [7:0] actual;
        input [7:0] expected;
        input [8*24-1:0] label_text;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: got %02x expected %02x", label_text, actual, expected);
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        #5 reset_n = 1'b0;
        #20 reset_n = 1'b1;
        #5 spi_cs_n = 1'b0;
        #5 spi_cs_n = 1'b1;

        fifo_write(8'h12);
        fifo_write(8'h34);
        fifo_write(8'h56);

        begin_transaction();
        spi_byte(8'h89, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h03, "FIFO level");
        end_transaction();
        if (irq !== 1'b1) begin
            $display("FAIL FIFO DATA_READY did not assert IRQ");
            failures = failures + 1;
        end

        begin_transaction();
        spi_byte(8'h88, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h12, "FIFO byte 0");
        spi_byte(8'h00, received); expect_byte(received, 8'h34, "FIFO byte 1");
        spi_byte(8'h00, received); expect_byte(received, 8'h56, "FIFO byte 2");
        end_transaction();

        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "FIFO empty status");
        end_transaction();
        if (irq !== 1'b0) begin
            $display("FAIL FIFO empty did not clear IRQ");
            failures = failures + 1;
        end

        fifo_write(8'haa);
        fifo_write(8'hbb);
        begin_transaction();
        spi_byte(8'h07, received);
        spi_byte(8'h02, received);
        end_transaction();
        repeat (6) @(posedge system_clk);
        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "FIFO clear status");
        end_transaction();

        for (fifo_index = 0; fifo_index < 16; fifo_index = fifo_index + 1)
            fifo_write(8'ha0 + fifo_index[7:0]);
        #12;
        if (fifo_write_full !== 1'b1) begin
            $display("FAIL FIFO full did not assert at capacity");
            failures = failures + 1;
        end
        fifo_write(8'hff); // Must be rejected while full.

        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h07, "FIFO full status");
        end_transaction();
        begin_transaction();
        spi_byte(8'h88, received);
        for (fifo_index = 0; fifo_index < 16; fifo_index = fifo_index + 1) begin
            spi_byte(8'h00, received);
            expect_byte(received, 8'ha0 + fifo_index[7:0], "FIFO capacity data");
        end
        end_transaction();
        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "FIFO drained status");
        end_transaction();

        if (failures == 0) begin
            $display("PASS obsidia_fpga_core_tb");
            $finish;
        end
        $fatal(1, "%0d FPGA FIFO test(s) failed", failures);
    end
endmodule

`default_nettype wire
