`timescale 1ns/1ps
`default_nettype none

module obsidia_async_fifo #(
    parameter ADDR_WIDTH = 4,
    parameter DATA_WIDTH = 8
) (
    input  wire                  reset_n,
    input  wire                  clear_toggle_async,
    input  wire                  wr_clk,
    input  wire                  wr_enable,
    input  wire [DATA_WIDTH-1:0] wr_data,
    output reg                   wr_full,
    input  wire                  rd_clk,
    input  wire                  rd_enable,
    output wire [DATA_WIDTH-1:0] rd_data,
    output wire                  rd_empty,
    output reg                   rd_full,
    output wire [ADDR_WIDTH:0]   rd_level
);
    localparam PTR_WIDTH = ADDR_WIDTH + 1;

    reg [DATA_WIDTH-1:0] memory [0:(1 << ADDR_WIDTH)-1];
    reg [PTR_WIDTH-1:0] wr_binary;
    reg [PTR_WIDTH-1:0] wr_gray;
    reg [PTR_WIDTH-1:0] rd_binary;
    reg [PTR_WIDTH-1:0] rd_gray;
    reg [PTR_WIDTH-1:0] rd_gray_wr_sync1;
    reg [PTR_WIDTH-1:0] rd_gray_wr_sync2;
    reg [PTR_WIDTH-1:0] wr_gray_rd_sync1;
    reg [PTR_WIDTH-1:0] wr_gray_rd_sync2;
    reg [1:0] wr_clear_sync;
    reg [1:0] rd_clear_sync;
    reg wr_clear_seen;
    reg rd_clear_seen;
    reg wr_full_rd_sync1;

    wire wr_take = wr_enable && !wr_full;
    wire rd_take = rd_enable && !rd_empty;
    wire [PTR_WIDTH-1:0] wr_binary_next = wr_binary + wr_take;
    wire [PTR_WIDTH-1:0] rd_binary_next = rd_binary + rd_take;
    wire [PTR_WIDTH-1:0] wr_gray_next = (wr_binary_next >> 1) ^ wr_binary_next;
    wire [PTR_WIDTH-1:0] rd_gray_next = (rd_binary_next >> 1) ^ rd_binary_next;
    wire [PTR_WIDTH-1:0] synchronized_wr_binary;
    wire wr_full_next;

    function [PTR_WIDTH-1:0] gray_to_binary;
        input [PTR_WIDTH-1:0] gray;
        integer index;
        begin
            gray_to_binary[PTR_WIDTH-1] = gray[PTR_WIDTH-1];
            for (index = PTR_WIDTH-2; index >= 0; index = index - 1)
                gray_to_binary[index] = gray_to_binary[index+1] ^ gray[index];
        end
    endfunction

    assign wr_full_next = (wr_gray_next == {
        ~rd_gray_wr_sync2[PTR_WIDTH-1:PTR_WIDTH-2],
        rd_gray_wr_sync2[PTR_WIDTH-3:0]
    });
    assign rd_empty = (rd_gray == wr_gray_rd_sync2);
    assign rd_data = memory[rd_binary[ADDR_WIDTH-1:0]];
    assign synchronized_wr_binary = gray_to_binary(wr_gray_rd_sync2);
    assign rd_level = synchronized_wr_binary - rd_binary;

    always @(posedge wr_clk or negedge reset_n) begin
        if (!reset_n) begin
            wr_binary         <= {PTR_WIDTH{1'b0}};
            wr_gray           <= {PTR_WIDTH{1'b0}};
            rd_gray_wr_sync1  <= {PTR_WIDTH{1'b0}};
            rd_gray_wr_sync2  <= {PTR_WIDTH{1'b0}};
            wr_clear_sync     <= 2'b00;
            wr_clear_seen     <= 1'b0;
            wr_full           <= 1'b0;
        end else begin
            rd_gray_wr_sync1 <= rd_gray;
            rd_gray_wr_sync2 <= rd_gray_wr_sync1;
            wr_clear_sync <= {wr_clear_sync[0], clear_toggle_async};

            if (wr_clear_sync[1] != wr_clear_seen) begin
                wr_binary     <= {PTR_WIDTH{1'b0}};
                wr_gray       <= {PTR_WIDTH{1'b0}};
                wr_clear_seen <= wr_clear_sync[1];
                wr_full       <= 1'b0;
            end else begin
                wr_full <= wr_full_next;
                if (wr_take) begin
                    memory[wr_binary[ADDR_WIDTH-1:0]] <= wr_data;
                    wr_binary <= wr_binary_next;
                    wr_gray   <= wr_gray_next;
                end
            end
        end
    end

    always @(posedge rd_clk or negedge reset_n) begin
        if (!reset_n) begin
            rd_binary         <= {PTR_WIDTH{1'b0}};
            rd_gray           <= {PTR_WIDTH{1'b0}};
            wr_gray_rd_sync1  <= {PTR_WIDTH{1'b0}};
            wr_gray_rd_sync2  <= {PTR_WIDTH{1'b0}};
            rd_clear_sync     <= 2'b00;
            rd_clear_seen     <= 1'b0;
            wr_full_rd_sync1  <= 1'b0;
            rd_full           <= 1'b0;
        end else begin
            wr_gray_rd_sync1 <= wr_gray;
            wr_gray_rd_sync2 <= wr_gray_rd_sync1;
            rd_clear_sync <= {rd_clear_sync[0], clear_toggle_async};
            wr_full_rd_sync1 <= wr_full;
            rd_full <= wr_full_rd_sync1;

            if (rd_clear_sync[1] != rd_clear_seen) begin
                rd_binary     <= {PTR_WIDTH{1'b0}};
                rd_gray       <= {PTR_WIDTH{1'b0}};
                rd_clear_seen <= rd_clear_sync[1];
            end else if (rd_take) begin
                rd_binary <= rd_binary_next;
                rd_gray   <= rd_gray_next;
            end
        end
    end
endmodule

`default_nettype wire
