#include "crow_all.h"
#include "api_client.hpp"
#include "analyzer.hpp"
#include "db_manager.hpp"
#include "auth_handler.hpp"
#include <iostream>

int main() {
    crow::SimpleApp app;
    
    // Initialize database
    DatabaseManager db("./database/stocks.db");
    if (!db.initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    // Initialize auth handler
    AuthHandler auth;
    
    std::cout << "Stock Analyzer Server starting on port 8080..." << std::endl;

    // Global OPTIONS handler for CORS - must be first
    CROW_CATCHALL_ROUTE(app)
    ([](const crow::request& req, crow::response& res){
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.add_header("Access-Control-Max-Age", "86400");
            res.end();
        } else {
            res.code = 404;
            res.end();
        }
    });

    // Auth routes - handle both OPTIONS and POST in one route
    CROW_ROUTE(app, "/api/auth/register")
    .methods("OPTIONS"_method, "POST"_method)
    ([&auth](const crow::request& req){
        // Handle OPTIONS preflight
        if (req.method == crow::HTTPMethod::Options) {
            auto res = crow::response(204);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        }
        
        // Handle POST request
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            auto response = crow::response(400, "Invalid JSON");
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        }
        
        std::string username = json_data["username"].s();
        std::string email = json_data["email"].s();
        std::string password = json_data["password"].s();
        
        if (auth.registerUser(username, email, password)) {
            std::string token = auth.generateToken(username);
            crow::json::wvalue result({
                {"success", true},
                {"token", token},
                {"username", username}
            });
            auto response = crow::response(result);
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        } else {
            crow::json::wvalue result({
                {"success", false},
                {"message", "Username already exists"}
            });
            auto response = crow::response(400, result);
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        }
    });
    
    CROW_ROUTE(app, "/api/auth/login")
    .methods("OPTIONS"_method, "POST"_method)
    ([&auth](const crow::request& req){
        // Handle OPTIONS preflight
        if (req.method == crow::HTTPMethod::Options) {
            auto res = crow::response(204);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.add_header("Access-Control-Max-Age", "86400");
            return res;
        }
        
        // Handle POST request
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            auto response = crow::response(400, "Invalid JSON");
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        }
        
        std::string username = json_data["username"].s();
        std::string password = json_data["password"].s();
        
        if (auth.loginUser(username, password)) {
            std::string token = auth.generateToken(username);
            crow::json::wvalue result({
                {"success", true},
                {"token", token},
                {"username", username}
            });
            auto response = crow::response(result);
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        } else {
            crow::json::wvalue result({
                {"success", false},
                {"message", "Invalid username or password"}
            });
            auto response = crow::response(401, result);
            response.add_header("Access-Control-Allow-Origin", "*");
            response.add_header("Access-Control-Allow-Headers", "Content-Type");
            return response;
        }
    });

    // Route to get the top 10 analyzed stocks
    CROW_ROUTE(app, "/api/stocks")
    ([&db]{
        std::cout << "Fetching top stocks..." << std::endl;
        
        // 1. Fetch data from Yahoo Finance
        std::vector<Stock> topStocks = ApiClient::fetchTopTen();
        
        if (topStocks.empty()) {
            return crow::response(500, "Failed to fetch stock data");
        }

        // 2. Analyze data
        StockAnalyzer::process(topStocks);
        
        // 3. Save to database
        for (const auto& stock : topStocks) {
            db.saveStock(stock);
        }

        // 4. Convert to JSON to send to frontend
        std::vector<crow::json::wvalue> json_list;
        for (const auto& s : topStocks) {
            json_list.push_back({
                {"symbol", s.symbol},
                {"name", s.name},
                {"price", s.price},
                {"change", s.change_percent},
                {"volume", s.volume},
                {"marketCap", s.market_cap},
                {"signal", s.analysis_result}
            });
        }

        auto response = crow::response(crow::json::wvalue(json_list));
        response.add_header("Access-Control-Allow-Origin", "*");
        return response;
    });
    
    // Route to get a specific stock quote
    CROW_ROUTE(app, "/api/stock/<string>")
    ([&db](const std::string& symbol){
        std::cout << "Fetching stock: " << symbol << std::endl;
        
        Stock stock = ApiClient::fetchStockQuote(symbol);
        
        if (stock.price == 0) {
            return crow::response(404, "Stock not found");
        }
        
        // Analyze single stock
        std::vector<HistoricalData> history = ApiClient::fetchHistoricalData(symbol, 30);
        if (!history.empty()) {
            stock.analysis_result = StockAnalyzer::analyzeSingleStock(stock, history);
        }
        
        // Save to database
        db.saveStock(stock);
        
        crow::json::wvalue json_stock({
            {"symbol", stock.symbol},
            {"name", stock.name},
            {"price", stock.price},
            {"change", stock.change_percent},
            {"volume", stock.volume},
            {"marketCap", stock.market_cap},
            {"signal", stock.analysis_result}
        });
        
        auto response = crow::response(json_stock);
        response.add_header("Access-Control-Allow-Origin", "*");
        return response;
    });
    
    // Route to search for stocks
    CROW_ROUTE(app, "/api/search/<string>")
    ([](const std::string& query){
        // For simplicity, return a JSON response indicating search capability
        // You could expand this to search through a list of all available symbols
        crow::json::wvalue result({
            {"message", "Search functionality - querying: " + query}
        });
        auto response = crow::response(result);
        response.add_header("Access-Control-Allow-Origin", "*");
        return response;
    });
    
    // Route to get cached stocks from database
    CROW_ROUTE(app, "/api/stocks/cached")
    ([&db]{
        std::vector<Stock> stocks = db.getStocks();
        
        std::vector<crow::json::wvalue> json_list;
        for (const auto& s : stocks) {
            json_list.push_back({
                {"symbol", s.symbol},
                {"name", s.name},
                {"price", s.price},
                {"change", s.change_percent},
                {"volume", s.volume},
                {"marketCap", s.market_cap},
                {"signal", s.analysis_result}
            });
        }
        
        auto response = crow::response(crow::json::wvalue(json_list));
        response.add_header("Access-Control-Allow-Origin", "*");
        return response;
    });

    app.port(8080).multithreaded().run();
    
    return 0;
}
