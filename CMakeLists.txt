cmake_minimum_required(VERSION 3.10)
project(fpga_trading_accelerator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# Enable simulation mode for testing without FPGA
add_definitions(-DSIMULATION_MODE)

# Create trading interface library
add_library(trading_interface
    sw/api/trading_interface.cpp
)

target_include_directories(trading_interface
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sw/api
)

# Create example application
add_executable(trading_example
    sw/apps/main.cpp
)

target_link_libraries(trading_example
    PRIVATE
        trading_interface
)
