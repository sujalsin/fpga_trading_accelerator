#include "trading_interface.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    trading::TradingAccelerator accelerator;

    // Initialize FPGA
    if (!accelerator.initialize("bitstream.bit")) {
        std::cerr << "Failed to initialize FPGA" << std::endl;
        return 1;
    }

    // Example market data
    trading::MarketData market_data{
        "AAPL",
        150.25,
        100,
        true,  // is_bid
        std::chrono::system_clock::now().time_since_epoch()
    };

    // Send market data
    if (!accelerator.send_market_data(market_data)) {
        std::cerr << "Failed to send market data" << std::endl;
        return 1;
    }

    // Get order book
    trading::OrderBook book;
    if (!accelerator.get_order_book("AAPL", book)) {
        std::cerr << "Failed to get order book" << std::endl;
        return 1;
    }

    // Print order book
    std::cout << "Order Book for AAPL:" << std::endl;
    std::cout << "Best Bid: " << book.best_bid_price << " (" 
              << book.best_bid_qty << " shares)" << std::endl;
    std::cout << "Best Ask: " << book.best_ask_price << " (" 
              << book.best_ask_qty << " shares)" << std::endl;

    // Print performance metrics
    std::cout << "\nPerformance Metrics:" << std::endl;
    std::cout << "Latency: " << accelerator.get_latency_ns() << " ns" << std::endl;
    std::cout << "Throughput: " << accelerator.get_throughput_orders_per_sec() 
              << " orders/sec" << std::endl;

    return 0;
}
