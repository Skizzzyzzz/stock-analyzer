#!/usr/bin/env python3
import sys
import json

def get_market_cap(symbol):
    try:
        import yfinance as yf
        stock = yf.Ticker(symbol)
        info = stock.info
        market_cap = info.get('marketCap', 0)
        return market_cap
    except Exception as e:
        print(f"Error getting market cap for {symbol}: {e}", file=sys.stderr)
        return 0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(json.dumps({"error": "Usage: get_marketcap.py SYMBOL"}))
        sys.exit(1)
    
    symbol = sys.argv[1].upper()
    market_cap = get_market_cap(symbol)
    print(json.dumps({"symbol": symbol, "marketCap": market_cap}))
