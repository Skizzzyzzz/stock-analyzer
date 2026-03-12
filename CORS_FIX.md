# Stock Analyzer - CORS Fix Documentation

## Problem
The Crow C++ web framework's SimpleApp automatically handles OPTIONS requests but doesn't allow adding custom CORS headers to these automatic responses. This caused browser CORS preflight requests to fail, blocking authentication endpoints.

## Solution
Implemented a Node.js CORS proxy server that sits between the frontend and backend:
- **Frontend** (port 3000) → **CORS Proxy** (port 8081) → **C++ Backend** (port 8080)

The proxy intercepts all requests, adds proper CORS headers, and handles OPTIONS preflight requests before forwarding to the backend.

## Files Changed

### New Files
1. **proxy-server.js** - CORS proxy using http-proxy npm package
2. **start.sh** - Startup script to launch all three servers
3. **package.json** & **package-lock.json** - Node.js dependencies

### Modified Files
1. **frontend/js/auth.js** - Updated API_URL from port 8080 to 8081
2. **frontend/js/app.js** - Updated API_URL from port 8080 to 8081

## Running the Application

### Quick Start
```bash
./start.sh
```

### Manual Start
```bash
# 1. Start C++ backend (port 8080)
cd /Users/vern/StockAnalyzer/backend/build
./stock_analyzer &

# 2. Start CORS proxy (port 8081)
cd /Users/vern/StockAnalyzer
node proxy-server.js &

# 3. Start frontend (port 3000)
cd /Users/vern/StockAnalyzer/frontend
python3 -m http.server 3000 &
```

### Stop All Servers
```bash
pkill -9 stock_analyzer && pkill -f "python.*http.server" && pkill -f "node proxy-server"
```

## URLs
- **Frontend**: http://localhost:3000
- **API (via proxy)**: http://localhost:8081/api
- **Backend (direct)**: http://localhost:8080/api

## Testing

### Test CORS Headers
```bash
# Test OPTIONS preflight
curl -i -X OPTIONS http://localhost:8081/api/auth/register \
  -H "Origin: http://localhost:3000" \
  -H "Access-Control-Request-Method: POST" \
  -H "Access-Control-Request-Headers: Content-Type"

# Should return:
# HTTP/1.1 204 No Content
# Access-Control-Allow-Origin: *
# Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
# Access-Control-Allow-Headers: Content-Type, Authorization
```

### Test Registration
```bash
curl -i -X POST http://localhost:8081/api/auth/register \
  -H "Content-Type: application/json" \
  -H "Origin: http://localhost:3000" \
  -d '{"username":"testuser","email":"test@test.com","password":"password123"}'

# Should return:
# HTTP/1.1 200 OK
# Access-Control-Allow-Origin: *
# {"success":true,"token":"...","username":"testuser"}
```

## Dependencies
- **C++ Backend**: Crow, libcurl, SQLite3, OpenSSL, ASIO
- **CORS Proxy**: Node.js, http-proxy npm package
- **Frontend**: Python 3 (http.server module)

## Why This Approach?
1. **Crow Limitation**: SimpleApp doesn't support middleware or custom OPTIONS handling
2. **Minimal Changes**: No need to migrate to a different C++ framework
3. **Flexibility**: Proxy can handle other concerns (rate limiting, logging, etc.)
4. **Standard Solution**: Many production apps use reverse proxies for CORS, SSL, etc.

## Alternative Approaches Attempted
1. ❌ Crow middleware - Not supported in SimpleApp
2. ❌ Manual OPTIONS routes - Overridden by Crow's auto-handling
3. ❌ Combined OPTIONS/POST methods - Still auto-handled by Crow
4. ❌ CATCHALL_ROUTE - Signature issues and still overridden
5. ✅ **Reverse proxy** - Clean, working solution

## Logs
View logs in real-time:
```bash
tail -f /tmp/stock_analyzer.log  # Backend
tail -f /tmp/proxy.log           # Proxy
tail -f /tmp/frontend.log        # Frontend
```
