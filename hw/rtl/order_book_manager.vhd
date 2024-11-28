library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity order_book_manager is
    generic (
        MAX_ORDERS      : integer := 1024;
        PRICE_WIDTH     : integer := 32;
        QUANTITY_WIDTH  : integer := 32;
        SYMBOL_WIDTH    : integer := 32
    );
    port (
        -- Clock and Reset
        clk             : in  std_logic;
        rst_n           : in  std_logic;
        
        -- Input Interface
        symbol_in       : in  std_logic_vector(SYMBOL_WIDTH-1 downto 0);
        price_in        : in  std_logic_vector(PRICE_WIDTH-1 downto 0);
        quantity_in     : in  std_logic_vector(QUANTITY_WIDTH-1 downto 0);
        is_bid          : in  std_logic;
        update_valid    : in  std_logic;
        
        -- Output Interface
        best_bid_price  : out std_logic_vector(PRICE_WIDTH-1 downto 0);
        best_ask_price  : out std_logic_vector(PRICE_WIDTH-1 downto 0);
        best_bid_qty    : out std_logic_vector(QUANTITY_WIDTH-1 downto 0);
        best_ask_qty    : out std_logic_vector(QUANTITY_WIDTH-1 downto 0);
        book_valid      : out std_logic;
        
        -- Status
        book_full       : out std_logic
    );
end order_book_manager;

architecture rtl of order_book_manager is
    -- Type definitions for order book entries
    type order_entry_t is record
        price    : unsigned(PRICE_WIDTH-1 downto 0);
        quantity : unsigned(QUANTITY_WIDTH-1 downto 0);
        valid    : std_logic;
    end record;
    
    type order_book_t is array (0 to MAX_ORDERS-1) of order_entry_t;
    
    -- Order books for bids and asks
    signal bid_book : order_book_t;
    signal ask_book : order_book_t;
    
    -- Best bid and ask tracking
    signal best_bid_idx : integer range 0 to MAX_ORDERS-1;
    signal best_ask_idx : integer range 0 to MAX_ORDERS-1;
    
    -- Internal signals
    signal order_count : unsigned(31 downto 0);
    signal updating    : std_logic;
    
begin
    -- Main order book update process
    process(clk, rst_n)
        variable new_price : unsigned(PRICE_WIDTH-1 downto 0);
        variable new_qty   : unsigned(QUANTITY_WIDTH-1 downto 0);
    begin
        if rst_n = '0' then
            -- Reset all signals
            order_count <= (others => '0');
            book_valid <= '0';
            book_full <= '0';
            updating <= '0';
            
            -- Reset order books
            for i in 0 to MAX_ORDERS-1 loop
                bid_book(i).valid <= '0';
                ask_book(i).valid <= '0';
            end loop;
            
        elsif rising_edge(clk) then
            if update_valid = '1' and updating = '0' then
                new_price := unsigned(price_in);
                new_qty := unsigned(quantity_in);
                
                if is_bid = '1' then
                    -- Update bid book
                    for i in 0 to MAX_ORDERS-1 loop
                        if bid_book(i).valid = '0' or bid_book(i).price = new_price then
                            bid_book(i).price <= new_price;
                            bid_book(i).quantity <= new_qty;
                            bid_book(i).valid <= '1';
                            exit;
                        end if;
                    end loop;
                else
                    -- Update ask book
                    for i in 0 to MAX_ORDERS-1 loop
                        if ask_book(i).valid = '0' or ask_book(i).price = new_price then
                            ask_book(i).price <= new_price;
                            ask_book(i).quantity <= new_qty;
                            ask_book(i).valid <= '1';
                            exit;
                        end if;
                    end loop;
                end if;
                
                updating <= '1';
            else
                updating <= '0';
            end if;
            
            -- Update best bid/ask
            if updating = '1' then
                -- Find best bid (highest price)
                for i in 0 to MAX_ORDERS-1 loop
                    if bid_book(i).valid = '1' then
                        if i = 0 or bid_book(i).price > bid_book(best_bid_idx).price then
                            best_bid_idx <= i;
                        end if;
                    end if;
                end loop;
                
                -- Find best ask (lowest price)
                for i in 0 to MAX_ORDERS-1 loop
                    if ask_book(i).valid = '1' then
                        if i = 0 or ask_book(i).price < ask_book(best_ask_idx).price then
                            best_ask_idx <= i;
                        end if;
                    end if;
                end loop;
                
                -- Update outputs
                best_bid_price <= std_logic_vector(bid_book(best_bid_idx).price);
                best_bid_qty <= std_logic_vector(bid_book(best_bid_idx).quantity);
                best_ask_price <= std_logic_vector(ask_book(best_ask_idx).price);
                best_ask_qty <= std_logic_vector(ask_book(best_ask_idx).quantity);
                book_valid <= '1';
            end if;
        end if;
    end process;

end rtl;
