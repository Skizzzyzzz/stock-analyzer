#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "data_models.hpp"
#include <vector>

class StockAnalyzer {
public:
    // Process and analyze stock data
    static void process(std::vector<Stock>& stocks);
    
    // Analyze a single stock
    static std::string analyzeSingleStock(const Stock& stock, const std::vector<HistoricalData>& history);
    
    // Calculate moving average
    static double calculateMovingAverage(const std::vector<HistoricalData>& data, int period);
    
    // Calculate RSI (Relative Strength Index)
    static double calculateRSI(const std::vector<HistoricalData>& data, int period = 14);
    
    // Determine buy/sell/hold signal
    static std::string determineSignal(double rsi, double price, double ma);
};

#endif // ANALYZER_HPP
