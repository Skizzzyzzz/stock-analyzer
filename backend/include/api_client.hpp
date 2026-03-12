#ifndef API_CLIENT_HPP
#define API_CLIENT_HPP

#include "data_models.hpp"
#include <string>
#include <vector>

class ApiClient {
public:
    // Fetch top stocks from Yahoo Finance
    static std::vector<Stock> fetchTopTen();
    
    // Fetch detailed quote for a specific symbol
    static Stock fetchStockQuote(const std::string& symbol);
    
    // Fetch historical data for a symbol
    static std::vector<HistoricalData> fetchHistoricalData(const std::string& symbol, int days = 30);
    
private:
    static std::string makeHttpRequest(const std::string& url);
    static std::string buildYahooFinanceUrl(const std::string& symbol);
    static std::string buildYahooQuoteUrl(const std::string& symbol);
};

#endif // API_CLIENT_HPP
