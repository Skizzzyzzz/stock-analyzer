#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <string>
#include <vector>

struct Stock {
    std::string symbol;
    std::string name;
    double price;
    double change_percent;
    double volume;
    double market_cap;
    std::string analysis_result; 
};

struct HistoricalData {
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct UserPortfolio {
    int user_id;
    std::string symbol;
    int shares;
    double purchase_price;
    std::string purchase_date;
};

#endif