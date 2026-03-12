#!/bin/bash

# Stock Analyzer Startup Script
# This script starts all required servers for the application

echo "🚀 Starting Stock Analyzer Application..."
echo ""

# Kill any existing processes
echo "Stopping existing processes..."
pkill -9 stock_analyzer 2>/dev/null
pkill -f "python.*-m http.server" 2>/dev/null
pkill -f "node proxy-server.js" 2>/dev/null
sleep 1

# Start C++ backend on port 8080
echo "Starting C++ backend server on port 8080..."
cd /Users/vern/StockAnalyzer/backend/build
./stock_analyzer > /tmp/stock_analyzer.log 2>&1 &
BACKEND_PID=$!
echo "✓ Backend started (PID: $BACKEND_PID)"
echo ""

# Start CORS proxy on port 8081
echo "Starting CORS proxy on port 8081..."
cd /Users/vern/StockAnalyzer
node proxy-server.js > /tmp/proxy.log 2>&1 &
PROXY_PID=$!
echo "✓ Proxy started (PID: $PROXY_PID)"
echo ""

# Start frontend HTTP server on port 3000
echo "Starting frontend server on port 3000..."
cd /Users/vern/StockAnalyzer/frontend
python3 -m http.server 3000 > /tmp/frontend.log 2>&1 &
FRONTEND_PID=$!
echo "✓ Frontend started (PID: $FRONTEND_PID)"
echo ""

sleep 2

echo "✅ All servers running!"
echo ""
echo "📊 Access the application:"
echo "   Frontend:  http://localhost:3000"
echo "   API Proxy: http://localhost:8081"
echo "   Backend:   http://localhost:8080"
echo ""
echo "📝 Logs:"
echo "   Backend:  tail -f /tmp/stock_analyzer.log"
echo "   Proxy:    tail -f /tmp/proxy.log"
echo "   Frontend: tail -f /tmp/frontend.log"
echo ""
echo "🛑 To stop all servers: pkill -9 stock_analyzer && pkill -f \"python.*http.server\" && pkill -f \"node proxy-server\""
