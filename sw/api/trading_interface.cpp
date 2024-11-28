#include "trading_interface.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace trading {

// Implementation class
class TradingAccelerator::Impl {
public:
    Impl() : fd_(-1), base_addr_(nullptr) {}
    ~Impl() {
        if (base_addr_) {
            #ifdef SIMULATION_MODE
            free(base_addr_);
            #else
            munmap(base_addr_, MAP_SIZE);
            #endif
        }
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    bool initialize(const std::string& bitstream_path) {
        #ifdef SIMULATION_MODE
        std::cout << "Running in simulation mode" << std::endl;
        base_addr_ = malloc(MAP_SIZE);
        if (!base_addr_) {
            std::cerr << "Failed to allocate simulation memory" << std::endl;
            return false;
        }
        // Initialize simulation memory with some test data
        volatile uint32_t* regs = static_cast<volatile uint32_t*>(base_addr_);
        regs[REG_STATUS] = 1;  // Ready
        regs[REG_LATENCY] = 100;  // 100ns latency
        regs[REG_THROUGHPUT] = 1000000;  // 1M orders/sec
        return true;
        #else
        // Open PCIe device
        fd_ = open("/dev/xdma0", O_RDWR);
        if (fd_ < 0) {
            std::cerr << "Failed to open PCIe device" << std::endl;
            return false;
        }

        // Map BAR0 memory region
        base_addr_ = mmap(nullptr, MAP_SIZE, PROT_READ | PROT_WRITE, 
                         MAP_SHARED, fd_, 0);
        if (base_addr_ == MAP_FAILED) {
            std::cerr << "Failed to map BAR0 memory" << std::endl;
            close(fd_);
            fd_ = -1;
            return false;
        }

        return true;
        #endif
    }

    bool send_market_data(const MarketData& data) {
        // Convert price to fixed-point representation
        uint64_t fixed_price = double_to_fixed(data.price);
        
        // Write to FPGA registers
        volatile uint32_t* regs = static_cast<volatile uint32_t*>(base_addr_);
        
        // Write symbol (assumed to be 4 characters max)
        regs[REG_SYMBOL] = *reinterpret_cast<const uint32_t*>(data.symbol.c_str());
        
        // Write price (high and low 32-bits)
        regs[REG_PRICE_H] = static_cast<uint32_t>(fixed_price >> 32);
        regs[REG_PRICE_L] = static_cast<uint32_t>(fixed_price);
        
        // Write quantity
        regs[REG_QUANTITY] = data.quantity;
        
        // Write control (is_bid and valid)
        uint32_t control = (data.is_bid ? 2 : 0) | 1;
        regs[REG_CONTROL] = control;
        
        // Wait for acknowledgment
        while ((regs[REG_STATUS] & 1) == 0) {
            // Add timeout if needed
        }
        
        return true;
    }

    bool get_order_book(const std::string& symbol, OrderBook& book) {
        volatile uint32_t* regs = static_cast<volatile uint32_t*>(base_addr_);
        
        // Request order book for symbol
        regs[REG_SYMBOL] = *reinterpret_cast<const uint32_t*>(symbol.c_str());
        regs[REG_CONTROL] = 4; // Request order book command
        
        // Wait for valid data
        while ((regs[REG_STATUS] & 2) == 0) {
            // Add timeout if needed
        }
        
        // Read best bid/ask prices and quantities
        uint64_t best_bid = (static_cast<uint64_t>(regs[REG_BEST_BID_H]) << 32) |
                            regs[REG_BEST_BID_L];
        uint64_t best_ask = (static_cast<uint64_t>(regs[REG_BEST_ASK_H]) << 32) |
                            regs[REG_BEST_ASK_L];
        
        book.best_bid_price = fixed_to_double(best_bid);
        book.best_ask_price = fixed_to_double(best_ask);
        book.best_bid_qty = regs[REG_BEST_BID_QTY];
        book.best_ask_qty = regs[REG_BEST_ASK_QTY];
        
        return true;
    }

    double get_latency_ns() {
        volatile uint32_t* regs = static_cast<volatile uint32_t*>(base_addr_);
        return static_cast<double>(regs[REG_LATENCY]);
    }

    uint64_t get_throughput_orders_per_sec() {
        volatile uint32_t* regs = static_cast<volatile uint32_t*>(base_addr_);
        return static_cast<uint64_t>(regs[REG_THROUGHPUT]);
    }

private:
    static constexpr size_t MAP_SIZE = 4096;
    static constexpr uint32_t REG_SYMBOL = 0;
    static constexpr uint32_t REG_PRICE_H = 1;
    static constexpr uint32_t REG_PRICE_L = 2;
    static constexpr uint32_t REG_QUANTITY = 3;
    static constexpr uint32_t REG_CONTROL = 4;
    static constexpr uint32_t REG_STATUS = 5;
    static constexpr uint32_t REG_BEST_BID_H = 6;
    static constexpr uint32_t REG_BEST_BID_L = 7;
    static constexpr uint32_t REG_BEST_ASK_H = 8;
    static constexpr uint32_t REG_BEST_ASK_L = 9;
    static constexpr uint32_t REG_BEST_BID_QTY = 10;
    static constexpr uint32_t REG_BEST_ASK_QTY = 11;
    static constexpr uint32_t REG_LATENCY = 12;
    static constexpr uint32_t REG_THROUGHPUT = 13;

    int fd_;
    void* base_addr_;

    // Convert between double and fixed-point representation
    static uint64_t double_to_fixed(double value) {
        return static_cast<uint64_t>(value * 1000000.0); // 6 decimal places
    }

    static double fixed_to_double(uint64_t value) {
        return static_cast<double>(value) / 1000000.0;
    }
};

// Public interface implementation
TradingAccelerator::TradingAccelerator() : impl_(new Impl()) {}
TradingAccelerator::~TradingAccelerator() = default;

bool TradingAccelerator::initialize(const std::string& bitstream_path) {
    return impl_->initialize(bitstream_path);
}

bool TradingAccelerator::send_market_data(const MarketData& data) {
    return impl_->send_market_data(data);
}

bool TradingAccelerator::get_order_book(const std::string& symbol, OrderBook& book) {
    return impl_->get_order_book(symbol, book);
}

bool TradingAccelerator::place_order(const std::string& symbol, double price,
                                   uint32_t quantity, bool is_buy) {
    MarketData data{symbol, price, quantity, is_buy,
                   std::chrono::nanoseconds(0)};
    return send_market_data(data);
}

bool TradingAccelerator::cancel_order(uint64_t order_id) {
    // Not implemented in this version
    return false;
}

double TradingAccelerator::get_latency_ns() {
    return impl_->get_latency_ns();
}

uint64_t TradingAccelerator::get_throughput_orders_per_sec() {
    return impl_->get_throughput_orders_per_sec();
}

} // namespace trading
