# Stock Analyzer

Local stock dashboard with:
- C++ backend API on port 8080
- Node.js CORS proxy on port 8081
- Frontend served by Python HTTP server on port 3000

## 1) Prerequisites

Install:
- CMake (3.10+)
- C++ compiler (C++17)
- OpenSSL
- SQLite3
- libcurl
- Node.js + npm
- Python 3

On macOS (Homebrew):

```bash
brew install cmake openssl sqlite curl node python
```

## 2) First-time setup

From the project root:

```bash
npm install
python3 -m venv .venv
source .venv/bin/activate
pip install yfinance
```

`yfinance` is required for market cap retrieval in `backend/get_marketcap.py`.

## 3) Build backend

Run when cloning for the first time or after C++ changes:

```bash
cd backend
mkdir -p build
cd build
cmake ..
cmake --build .
cd ../..
```

## 4) Start everything

```bash
chmod +x start.sh
./start.sh
```

Open:
- Frontend: http://localhost:3000
- Proxy API: http://localhost:8081
- Backend API: http://localhost:8080

## 5) Stop services

```bash
pkill -9 stock_analyzer
pkill -f "python.*http.server"
pkill -f "node proxy-server.js"
```

## 6) Logs

```bash
tail -f /tmp/stock_analyzer.log
tail -f /tmp/proxy.log
tail -f /tmp/frontend.log
```

## Quick restart

```bash
source .venv/bin/activate
./start.sh
```
