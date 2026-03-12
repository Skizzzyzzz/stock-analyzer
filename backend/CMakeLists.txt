cmake_minimum_required(VERSION 3.10)
project(StockAnalyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Find required packages
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Add Crow (header-only library) and ASIO
# Download crow_all.h from: https://github.com/CrowCpp/Crow/releases
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(/opt/homebrew/include)

# Source files
set(SOURCES
    src/main.cpp
    src/api_client.cpp
    src/analyzer.cpp
    src/db_manager.cpp
    src/auth_handler.cpp
)

# Create executable
add_executable(stock_analyzer ${SOURCES})

# Link libraries
target_link_libraries(stock_analyzer 
    CURL::libcurl
    SQLite::SQLite3
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# Create database directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/database)
