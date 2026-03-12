#include "analyzer.hpp"
#include "api_client.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

void StockAnalyzer::process(std::vector<Stock>& stocks) {
    for (auto& stock : stocks) {
        // Fetch historical data for each stock
        std::vector<HistoricalData> history = ApiClient::fetchHistoricalData(stock.symbol, 30);
        
        if (!history.empty()) {
            stock.analysis_result = analyzeSingleStock(stock, history);
        } else {
            stock.analysis_result = "HOLD";
        }
    }
}

std::string StockAnalyzer::analyzeSingleStock(const Stock& stock, const std::vector<HistoricalData>& history) {
    if (history.size() < 14) {
        return "HOLD"; // Not enough data for proper analysis
    }
    
    // Calculate technical indicators
    double rsi = calculateRSI(history, 14);
    double ma20 = calculateMovingAverage(history, std::min(20, (int)history.size()));
    
    return determineSignal(rsi, stock.price, ma20);
}

double StockAnalyzer::calculateMovingAverage(const std::vector<HistoricalData>& data, int period) {
    if (data.empty() || period <= 0 || period > (int)data.size()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (int i = 0; i < period; i++) {
        sum += data[i].close;
    }
    
    return sum / period;
}

double StockAnalyzer::calculateRSI(const std::vector<HistoricalData>& data, int period) {
    if (data.size() < (size_t)(period + 1)) {
        return 50.0; // Neutral RSI if not enough data
    }
    
    double gains = 0.0;
    double losses = 0.0;
    
    // Calculate initial average gain and loss
    for (int i = 1; i <= period; i++) {
        double change = data[i-1].close - data[i].close;
        if (change > 0) {
            gains += change;
        } else {
            losses += std::abs(change);
        }
    }
    
    double avgGain = gains / period;
    double avgLoss = losses / period;
    
    if (avgLoss == 0) {
        return 100.0;
    }
    
    double rs = avgGain / avgLoss;
    double rsi = 100.0 - (100.0 / (1.0 + rs));
    
    return rsi;
}

std::string StockAnalyzer::determineSignal(double rsi, double price, double ma) {
    // RSI-based signals:
    // RSI < 30: Oversold (potential BUY)
    // RSI > 70: Overbought (potential SELL)
    // Also consider price relative to moving average
    
    if (rsi < 30 && price < ma) {
        return "STRONG BUY";
    } else if (rsi < 40 && price < ma) {
        return "BUY";
    } else if (rsi > 70 && price > ma) {
        return "STRONG SELL";
    } else if (rsi > 60 && price > ma) {
        return "SELL";
    } else {
        return "HOLD";
    }
}
