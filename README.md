# Stock Analyzer

Stock Analyzer is a local full-stack app with:
- C++ backend API (Crow + libcurl + SQLite + OpenSSL) on port `8080`
- Node.js CORS proxy on port `8081`
- Static frontend served with Python on port `3000`

## Prerequisites

Install these before running the project:

- Git
- CMake (3.10+)
- C++ compiler with C++17 support (Apple Clang on macOS is fine)
- OpenSSL
- SQLite3
- libcurl
- Node.js + npm
- Python 3

### macOS (Homebrew)

```bash
brew install cmake openssl sqlite curl node python
```

## Project setup

From the project root:

```bash
npm install
```

## Share with a friend (fresh machine)

Send your friend the repository URL and ask them to run:

```bash
git clone <your-repo-url>
cd StockAnalyzer
npm install
```

Then build backend:

```bash
cd backend
mkdir -p build
cd build
cmake ..
cmake --build .
cd ../..
```

Then start app:

```bash
chmod +x start.sh
./start.sh
```

Open: `http://localhost:3000`

## Build backend

If backend is not built yet (or after C++ changes):

```bash
cd backend
mkdir -p build
cd build
cmake ..
cmake --build .
```

This produces the backend executable at `backend/build/stock_analyzer`.

## Start the app

Quick start (recommended):

```bash
chmod +x start.sh
./start.sh
```

App URLs:
- Frontend: http://localhost:3000
- Proxy API: http://localhost:8081
- Backend API (direct): http://localhost:8080

## Stop all services

```bash
pkill -9 stock_analyzer
pkill -f "python.*http.server"
pkill -f "node proxy-server.js"
```

## Logs

```bash
tail -f /tmp/stock_analyzer.log
tail -f /tmp/proxy.log
tail -f /tmp/frontend.log
```

## What to add to `.gitignore`

For this project, keep these ignored:

- Build output (`backend/build/`, object files)
- Runtime databases (`*.db`, `database/`)
- Runtime logs (`*.log`)
- Python virtual envs (`.venv/`, `venv/`)
- Node dependencies (`node_modules/`)
- Local secrets (`.env`, `.env.*`)
- Editor/OS files (`.vscode/`, `.DS_Store`)

This repository already includes a `.gitignore`; see below for an updated version with recommended entries.
