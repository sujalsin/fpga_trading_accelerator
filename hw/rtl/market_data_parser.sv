// Market Data Parser Module
// Handles incoming market data packets with ultra-low latency
`timescale 1ns / 1ps

module market_data_parser #(
    parameter DATA_WIDTH = 64,
    parameter SYMBOL_WIDTH = 32,
    parameter PRICE_WIDTH = 32,
    parameter QUANTITY_WIDTH = 32
)(
    input  logic                    clk,
    input  logic                    rst_n,
    input  logic                    data_valid,
    input  logic [DATA_WIDTH-1:0]   data_in,
    input  logic                    ready_next,
    
    output logic                    data_ready,
    output logic [SYMBOL_WIDTH-1:0] symbol,
    output logic [PRICE_WIDTH-1:0]  price,
    output logic [QUANTITY_WIDTH-1:0] quantity,
    output logic                    parse_valid,
    output logic [1:0]             msg_type    // 00: undefined, 01: trade, 10: quote, 11: order
);

    // State machine states
    typedef enum logic [2:0] {
        IDLE,
        HEADER_PARSE,
        SYMBOL_PARSE,
        PRICE_PARSE,
        QUANTITY_PARSE
    } parse_state_t;

    parse_state_t current_state, next_state;

    // Internal registers
    logic [DATA_WIDTH-1:0] data_buffer;
    logic [2:0] header_type;

    // State machine
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            current_state <= IDLE;
            data_buffer <= '0;
            symbol <= '0;
            price <= '0;
            quantity <= '0;
            parse_valid <= 1'b0;
            msg_type <= 2'b00;
        end else begin
            current_state <= next_state;
            
            case (current_state)
                IDLE: begin
                    if (data_valid) begin
                        data_buffer <= data_in;
                        parse_valid <= 1'b0;
                    end
                end

                HEADER_PARSE: begin
                    header_type <= data_buffer[2:0];
                    msg_type <= data_buffer[1:0];
                end

                SYMBOL_PARSE: begin
                    symbol <= data_buffer[SYMBOL_WIDTH-1:0];
                end

                PRICE_PARSE: begin
                    price <= data_buffer[PRICE_WIDTH-1:0];
                end

                QUANTITY_PARSE: begin
                    quantity <= data_buffer[QUANTITY_WIDTH-1:0];
                    parse_valid <= 1'b1;
                end
            endcase
        end
    end

    // Next state logic
    always_comb begin
        next_state = current_state;
        data_ready = 1'b0;

        case (current_state)
            IDLE: begin
                if (data_valid) begin
                    next_state = HEADER_PARSE;
                end
                data_ready = 1'b1;
            end

            HEADER_PARSE: begin
                next_state = SYMBOL_PARSE;
            end

            SYMBOL_PARSE: begin
                next_state = PRICE_PARSE;
            end

            PRICE_PARSE: begin
                next_state = QUANTITY_PARSE;
            end

            QUANTITY_PARSE: begin
                if (ready_next) begin
                    next_state = IDLE;
                end
            end

            default: next_state = IDLE;
        endcase
    end

endmodule
