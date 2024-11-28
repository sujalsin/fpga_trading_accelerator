#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace trading {

struct MarketData {
    std::string symbol;
    double price;
    uint32_t quantity;
    bool is_bid;
    std::chrono::nanoseconds timestamp;
};

struct OrderBook {
    double best_bid_price;
    double best_ask_price;
    uint32_t best_bid_qty;
    uint32_t best_ask_qty;
    std::chrono::nanoseconds timestamp;
};

class TradingAccelerator {
public:
    TradingAccelerator();
    ~TradingAccelerator();

    // Initialize the FPGA and PCIe connection
    bool initialize(const std::string& bitstream_path);

    // Market data interface
    bool send_market_data(const MarketData& data);
    bool get_order_book(const std::string& symbol, OrderBook& book);

    // Trading interface
    bool place_order(const std::string& symbol, double price, 
                    uint32_t quantity, bool is_buy);
    bool cancel_order(uint64_t order_id);

    // Performance monitoring
    double get_latency_ns();
    uint64_t get_throughput_orders_per_sec();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace trading
