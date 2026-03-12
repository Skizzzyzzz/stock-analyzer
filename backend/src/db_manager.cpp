#include "db_manager.hpp"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& dbPath) : db(nullptr), dbPath(dbPath) {}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // Create tables
    std::string createStocksTable = R"(
        CREATE TABLE IF NOT EXISTS stocks (
            symbol TEXT PRIMARY KEY,
            name TEXT,
            price REAL,
            change_percent REAL,
            volume REAL,
            market_cap REAL,
            analysis_result TEXT,
            last_updated DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    std::string createPortfolioTable = R"(
        CREATE TABLE IF NOT EXISTS portfolio (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            symbol TEXT,
            shares INTEGER,
            purchase_price REAL,
            purchase_date DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    std::string createWatchlistTable = R"(
        CREATE TABLE IF NOT EXISTS watchlist (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            symbol TEXT,
            added_date DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    return executeQuery(createStocksTable) && 
           executeQuery(createPortfolioTable) && 
           executeQuery(createWatchlistTable);
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool DatabaseManager::saveStock(const Stock& stock) {
    std::string query = "INSERT OR REPLACE INTO stocks (symbol, name, price, change_percent, volume, market_cap, analysis_result) VALUES ('"
        + stock.symbol + "', '" + stock.name + "', " + std::to_string(stock.price) + ", " 
        + std::to_string(stock.change_percent) + ", " + std::to_string(stock.volume) + ", " 
        + std::to_string(stock.market_cap) + ", '" + stock.analysis_result + "');";
    
    return executeQuery(query);
}

std::vector<Stock> DatabaseManager::getStocks() {
    std::vector<Stock> stocks;
    std::string query = "SELECT symbol, name, price, change_percent, volume, market_cap, analysis_result FROM stocks;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Stock stock;
            stock.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            stock.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            stock.price = sqlite3_column_double(stmt, 2);
            stock.change_percent = sqlite3_column_double(stmt, 3);
            stock.volume = sqlite3_column_double(stmt, 4);
            stock.market_cap = sqlite3_column_double(stmt, 5);
            stock.analysis_result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            stocks.push_back(stock);
        }
    }
    
    sqlite3_finalize(stmt);
    return stocks;
}

bool DatabaseManager::addToPortfolio(int userId, const std::string& symbol, int shares, double price) {
    std::string query = "INSERT INTO portfolio (user_id, symbol, shares, purchase_price) VALUES (" 
        + std::to_string(userId) + ", '" + symbol + "', " + std::to_string(shares) + ", " 
        + std::to_string(price) + ");";
    
    return executeQuery(query);
}

std::vector<UserPortfolio> DatabaseManager::getUserPortfolio(int userId) {
    std::vector<UserPortfolio> portfolio;
    std::string query = "SELECT user_id, symbol, shares, purchase_price, purchase_date FROM portfolio WHERE user_id = " 
        + std::to_string(userId) + ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            UserPortfolio item;
            item.user_id = sqlite3_column_int(stmt, 0);
            item.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.shares = sqlite3_column_int(stmt, 2);
            item.purchase_price = sqlite3_column_double(stmt, 3);
            item.purchase_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            portfolio.push_back(item);
        }
    }
    
    sqlite3_finalize(stmt);
    return portfolio;
}

bool DatabaseManager::removeFromPortfolio(int userId, const std::string& symbol) {
    std::string query = "DELETE FROM portfolio WHERE user_id = " + std::to_string(userId) 
        + " AND symbol = '" + symbol + "';";
    
    return executeQuery(query);
}

bool DatabaseManager::addToWatchlist(int userId, const std::string& symbol) {
    std::string query = "INSERT INTO watchlist (user_id, symbol) VALUES (" 
        + std::to_string(userId) + ", '" + symbol + "');";
    
    return executeQuery(query);
}

std::vector<std::string> DatabaseManager::getWatchlist(int userId) {
    std::vector<std::string> watchlist;
    std::string query = "SELECT symbol FROM watchlist WHERE user_id = " + std::to_string(userId) + ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            watchlist.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }
    
    sqlite3_finalize(stmt);
    return watchlist;
}
