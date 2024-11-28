# FPGA-Based Low-Latency Trading Accelerator

A high-performance FPGA-based trading system designed for ultra-low latency market data processing and algorithmic trading execution.

## System Architecture

```
Market Data Feed                 Trading Strategy
     │                                │
     ▼                                ▼
┌────────────────────────────────────────────────────┐
│                     FPGA                           │
│  ┌──────────────┐    ┌─────────────┐    ┌──────┐  │
│  │ Market Data  │ => │ Order Book  │ => │Trade │  │
│  │   Parser     │    │  Manager    │    │Engine│  │
│  └──────────────┘    └─────────────┘    └──────┘  │
│           ▲                 │                ▼     │
│           │                 ▼                │     │
│      ┌─────────────────────────────────────┐      │
│      │           PCIe Interface            │      │
└──────┼─────────────────────────────────────┼──────┘
       │                                     │
       ▼                                     ▼
┌─────────────┐                    ┌──────────────┐
│ Market Data │                    │   Trading    │
│   Stream    │                    │ Applications │
└─────────────┘                    └──────────────┘
```

## Components Overview

### 1. Hardware Components (FPGA)

#### Market Data Parser (SystemVerilog)
- Processes incoming market data packets with ultra-low latency
- Parses and normalizes different market data formats
- Implements a finite state machine for packet processing
```
   IDLE → HEADER_PARSE → SYMBOL_PARSE → PRICE_PARSE → QUANTITY_PARSE
    ▲                                                      │
    └──────────────────────────────────────────────────────┘
```

#### Order Book Manager (VHDL)
- Maintains real-time order book state
- Tracks best bid/ask prices and quantities
- Implements priority queue for price levels
```
Price Levels:      Order Queue:
   105.00 Ask └─► Order3 → Order5 → Order8
   104.75 Ask └─► Order1 → Order4
   104.50 Bid └─► Order2 → Order6
   104.25 Bid └─► Order7 → Order9
```

### 2. Software Components (C++)

#### Host Interface Library
```cpp
namespace trading {
    class TradingAccelerator {
        // High-level trading interface
        bool send_market_data(const MarketData& data);
        bool get_order_book(const std::string& symbol, OrderBook& book);
        // ... more methods
    };
}
```

## Performance Metrics

| Metric                  | Target Performance |
|------------------------|-------------------|
| Market Data Latency    | < 100 ns         |
| Order Book Updates     | > 10M updates/s  |
| Trading Decision       | < 250 ns         |
| End-to-End Latency    | < 1 µs           |

## Data Flow

```
1. Market Data Ingestion:
   UDP → FPGA → Parser → Order Book
   
2. Trading Decision:
   Order Book → Trading Logic → Order Generation

3. Order Execution:
   FPGA → Exchange Gateway → Market
```

## Low Latency Features

1. **Zero-Copy Architecture**
   ```
   Market Data → DMA → FPGA Memory
   No CPU intervention required
   ```

2. **Parallel Processing**
   ```
   ┌─────────────┐
   │ Market Data │ 
   └─────────────┘
         ║ 
   ╔════════════╗
   ║ Parse      ║
   ║ Update     ║  Concurrent
   ║ Calculate  ║  Processing
   ║ Execute    ║
   ╚════════════╝
   ```

3. **Fixed-Point Arithmetic**
   - Custom fixed-point implementation
   - 6 decimal places precision
   - Hardware-optimized calculations

## Building and Testing

### Prerequisites
- Xilinx Vivado or Intel Quartus for FPGA synthesis
- Modern C++ compiler (C++17 support required)
- CMake 3.10 or higher

### Build Instructions

1. **FPGA Bitstream**
   ```bash
   # Using Vivado
   vivado -mode batch -source scripts/generate_bitstream.tcl
   ```

2. **Software Components**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Running Tests**
   ```bash
   # In simulation mode
   ./trading_example
   ```

### Simulation Mode
For development and testing without FPGA hardware:
```bash
# Enable simulation mode in CMake
cmake -DSIMULATION_MODE=ON ..
```

## Project Structure
```
fpga_trading_accelerator/
├── hw/                     # Hardware design files
│   ├── rtl/               # RTL design files
│   │   ├── market_data_parser.sv
│   │   └── order_book_manager.vhd
│   ├── constraints/       # Timing and pin constraints
│   └── tb/               # Testbenches
├── sw/                     # Software components
│   ├── driver/           # PCIe driver
│   ├── api/              # Trading API
│   │   ├── trading_interface.hpp
│   │   └── trading_interface.cpp
│   └── apps/             # Applications
│       └── main.cpp
└── doc/                    # Documentation
```

## Performance Optimization Tips

1. **FPGA Optimization**
   - Use parallel processing pipelines
   - Minimize clock domain crossings
   - Optimize critical paths

2. **Software Optimization**
   - Use huge pages for memory allocation
   - Pin threads to CPU cores
   - Disable CPU frequency scaling

3. **System Optimization**
   - Optimize PCIe settings
   - Use direct memory access (DMA)
   - Minimize system calls

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## License

This project is proprietary and confidential.

## Trading Scenarios and Examples

### 1. High-Frequency Market Making
```
Scenario: Two-sided Quote Maintenance

Market Data:        AAPL
                   ┌────────────┐
Best Bid: 180.50   │   Spread   │   Best Ask: 180.55
Quantity: 100      └────────────┘   Quantity: 150

FPGA Response (< 250ns):
┌─────────────────────┐    ┌─────────────────────┐
│    Place Bid @ 180.52│    │Place Ask @ 180.53   │
│    Quantity: 50     │    │Quantity: 50         │
└─────────────────────┘    └─────────────────────┘
        
Profit Potential = (180.53 - 180.52) * 50 = $0.50 per fill
```

### 2. Statistical Arbitrage
```
Time     AAPL      AAPL_FUT    Spread    Action
t0       180.50    180.55      0.05      Wait
t1       180.60    180.55     -0.05      ┌────────────┐
                                         │Buy Future   │
                                         │Sell Stock   │
                                         └────────────┘
t2       180.55    180.55      0.00      Close Position
```

### 3. Order Book Imbalance Trading
```
AAPL Order Book State:
                                        
Price    Bids      Asks     Imbalance
180.60     0       500        -500
180.55     0       800        -800    ┐
180.50   200       700        -500    ├─ Significant Ask-Side
180.45   300       400        -100    │  Pressure Detected
180.40   250       300         -50    ┘
                                        
FPGA Action (< 100ns):
┌─────────────────────────────┐
│ Place Sell Order @ 180.45   │
│ Quantity: 200              │
└─────────────────────────────┘
```

### 4. News-Based Trading
```
Time (ns)  Event
0          ├─ News Event Detected
50         ├─ Parse Keywords
100        ├─ Calculate Impact
150        ├─ Generate Orders
200        └─ Send to Exchange

Example News Impact:
┌───────────────────────────┐
│ AAPL Earnings Beat       │ 
│ Expected: $1.50         │
│ Actual:   $1.65         │
└───────────────────────────┘
                ↓
FPGA Response (< 250ns total):
┌───────────────────────────┐
│ Buy AAPL @ Market        │
│ Quantity: Dynamic based  │
│ on impact calculation    │
└───────────────────────────┘
```

### 5. Multi-Venue Latency Arbitrage
```
Exchange Latencies (ns):
NYSE:    500
NASDAQ:  600
BATS:    550

Price Updates:
Time(ns)  Exchange  AAPL Price
0         NYSE      180.50
50        NASDAQ    180.50
100       NYSE      180.55  ←── Price jump detected
250       BATS      180.50  ←── Arbitrage window
300       NASDAQ    180.50

FPGA Action Timeline:
0ns   : ├─ Detect NYSE price change
100ns : ├─ Calculate arbitrage opportunity
150ns : ├─ Generate orders
200ns : └─ Send buy orders to BATS/NASDAQ
```

### 6. Example Code for Market Making Scenario
```cpp
// Initialize the trading accelerator
trading::TradingAccelerator accelerator;
accelerator.initialize("bitstream.bit");

// Market making parameters
const double SPREAD_THRESHOLD = 0.05;
const double ORDER_SIZE = 50;

// Market making loop
while (true) {
    // Get current order book
    trading::OrderBook book;
    accelerator.get_order_book("AAPL", book);
    
    // Calculate spread
    double spread = book.best_ask_price - book.best_bid_price;
    
    if (spread >= SPREAD_THRESHOLD) {
        // Place two-sided quotes inside the spread
        double mid_price = (book.best_ask_price + book.best_bid_price) / 2;
        
        // Place buy order
        accelerator.place_order("AAPL", 
                              mid_price - SPREAD_THRESHOLD/4,  // Bid price
                              ORDER_SIZE,
                              true);  // is_buy = true
        
        // Place sell order
        accelerator.place_order("AAPL",
                              mid_price + SPREAD_THRESHOLD/4,  // Ask price
                              ORDER_SIZE,
                              false);  // is_buy = false
    }
}
```

### 7. Risk Management Scenarios
```
Position Limits:
┌────────────────────┐
│ Symbol: AAPL      │
│ Max Position: 1000│
│ Current Pos: 800  │
└────────────────────┘

Incoming Buy Order:
┌────────────────┐
│ Quantity: 300  │
│ Price: 180.50  │
└────────────────┘

FPGA Risk Check (< 50ns):
┌────────────────────────┐
│ 800 + 300 > 1000      │
│ Result: Reject Order   │
└────────────────────────┘
```

Each scenario demonstrates the FPGA accelerator's capability to:
1. Process market data in nanoseconds
2. Make complex trading decisions
3. Execute orders with minimal latency
4. Manage risk in real-time
5. Handle multiple trading strategies simultaneously

The accelerator's low latency and high throughput enable these sophisticated trading strategies to be executed effectively in modern markets where speed is crucial.
