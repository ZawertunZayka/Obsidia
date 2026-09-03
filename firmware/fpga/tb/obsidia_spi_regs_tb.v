`timescale 1ns/1ps
`default_nettype none

module obsidia_spi_regs_tb;
    reg reset_n = 1'b1;
    reg spi_sclk = 1'b0;
    reg spi_cs_n = 1'b1;
    reg spi_mosi = 1'b0;
    wire spi_miso;

    reg [7:0] received;
    integer failures = 0;

    obsidia_spi_regs dut (
        .reset_n(reset_n),
        .spi_sclk(spi_sclk),
        .spi_cs_n(spi_cs_n),
        .spi_mosi(spi_mosi),
        .spi_miso(spi_miso)
    );

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
        // Model the physical CS-high asynchronous reset level after FPGA reset.
        #5 spi_cs_n = 1'b0;
        #5 spi_cs_n = 1'b1;
        #5;

        begin_transaction();
        spi_byte(8'h80, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h4f, "device id 0");
        spi_byte(8'h00, received); expect_byte(received, 8'h42, "device id 1");
        spi_byte(8'h00, received); expect_byte(received, 8'h53, "device id 2");
        spi_byte(8'h00, received); expect_byte(received, 8'h44, "device id 3");
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "version major");
        spi_byte(8'h00, received); expect_byte(received, 8'h00, "version minor");
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "ready status");
        end_transaction();

        begin_transaction();
        spi_byte(8'h07, received);
        spi_byte(8'h02, received);
        end_transaction();
        begin_transaction();
        spi_byte(8'h87, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h02, "control readback");
        end_transaction();

        begin_transaction();
        spi_byte(8'hff, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h00, "invalid read");
        end_transaction();
        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h11, "latched error");
        end_transaction();

        begin_transaction();
        spi_byte(8'h07, received);
        spi_byte(8'h80, received);
        end_transaction();
        begin_transaction();
        spi_byte(8'h86, received);
        spi_byte(8'h00, received); expect_byte(received, 8'h01, "soft reset status");
        end_transaction();

        if (failures == 0) begin
            $display("PASS obsidia_spi_regs_tb");
            $finish;
        end
        $fatal(1, "%0d FPGA SPI test(s) failed", failures);
    end
endmodule

`default_nettype wire
