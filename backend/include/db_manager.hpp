#ifndef DB_MANAGER_HPP
#define DB_MANAGER_HPP

#include "data_models.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>

class DatabaseManager {
public:
    DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();
    
    // Initialize database tables
    bool initialize();
    
    // Stock operations
    bool saveStock(const Stock& stock);
    std::vector<Stock> getStocks();
    
    // User portfolio operations
    bool addToPortfolio(int userId, const std::string& symbol, int shares, double price);
    std::vector<UserPortfolio> getUserPortfolio(int userId);
    bool removeFromPortfolio(int userId, const std::string& symbol);
    
    // Watchlist operations
    bool addToWatchlist(int userId, const std::string& symbol);
    std::vector<std::string> getWatchlist(int userId);
    
private:
    sqlite3* db;
    std::string dbPath;
    
    bool executeQuery(const std::string& query);
};

#endif // DB_MANAGER_HPP
