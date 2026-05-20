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

# macOS: Homebrew puts headers in /opt/homebrew/include (Apple Silicon) or /usr/local/include (Intel)
if(APPLE)
    if(EXISTS /opt/homebrew/include)
        include_directories(/opt/homebrew/include)
    elseif(EXISTS /usr/local/include)
        include_directories(/usr/local/include)
    endif()
endif()

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
)

# pthreads on Unix/macOS; Winsock on Windows
if(WIN32)
    target_link_libraries(stock_analyzer ws2_32 mswsock)
else()
    target_link_libraries(stock_analyzer pthread)
endif()

# Create database directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/database)
