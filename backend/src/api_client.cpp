#include "api_client.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#define POPEN  _popen
#define PCLOSE _pclose
#else
#define POPEN  popen
#define PCLOSE pclose
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

using json = nlohmann::json;

// Callback function for libcurl to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Returns the directory containing the running executable
static std::filesystem::path getExecutableDir() {
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    return std::filesystem::canonical(path).parent_path();
#else
    return std::filesystem::read_symlink("/proc/self/exe").parent_path();
#endif
}

// Helper function to execute Python script and get output
static std::string executePythonScript(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(command.c_str(), "r"), PCLOSE);
    if (!pipe) {
        std::cerr << "Failed to execute command: " << command << std::endl;
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

// Get market cap using Python yfinance library
static double getMarketCapFromPython(const std::string& symbol) {
#ifdef _WIN32
    const std::string pythonCmd = "python";
#else
    const std::string pythonCmd = "python3";
#endif
    // MSVC places the exe in a Release/ or Debug/ subdirectory; account for both layouts
    auto exeDir = getExecutableDir();
    auto dirName = exeDir.filename().string();
    std::filesystem::path scriptPath;
    if (dirName == "Release" || dirName == "Debug" ||
        dirName == "RelWithDebInfo" || dirName == "MinSizeRel") {
        scriptPath = exeDir.parent_path().parent_path() / "get_marketcap.py";
    } else {
        scriptPath = exeDir.parent_path() / "get_marketcap.py";
    }
    std::string command = pythonCmd + " \"" + scriptPath.string() + "\" " + symbol;
    std::string output = executePythonScript(command);
    
    if (output.empty()) {
        return 0.0;
    }
    
    try {
        auto jsonResult = json::parse(output);
        if (jsonResult.contains("marketCap")) {
            return jsonResult["marketCap"].get<double>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Python market cap result: " << e.what() << std::endl;
    }
    
    return 0.0;
}

std::string ApiClient::makeHttpRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "StockAnalyzer/1.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

std::string ApiClient::buildYahooFinanceUrl(const std::string& symbol) {
    // Yahoo Finance API v8 endpoint
    return "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol + "?interval=1d&range=1mo";
}

std::string ApiClient::buildYahooQuoteUrl(const std::string& symbol) {
    // Yahoo Finance Quote API v11 - includes market cap
    return "https://query2.finance.yahoo.com/v11/finance/quoteSummary/" + symbol + "?modules=price,summaryDetail";
}

Stock ApiClient::fetchStockQuote(const std::string& symbol) {
    Stock stock;
    stock.symbol = symbol;
    
    try {
        std::string url = buildYahooFinanceUrl(symbol);
        std::string response = makeHttpRequest(url);
        
        if (response.empty()) {
            std::cerr << "Empty response for symbol: " << symbol << std::endl;
            return stock;
        }
        
        auto jsonData = json::parse(response);
        
        // Navigate the Yahoo Finance JSON structure
        auto chart = jsonData["chart"]["result"][0];
        auto meta = chart["meta"];
        
        stock.name = meta.value("longName", symbol);
        stock.price = meta["regularMarketPrice"].get<double>();
        stock.change_percent = ((stock.price - meta["chartPreviousClose"].get<double>()) / 
                                meta["chartPreviousClose"].get<double>() * 100.0);
        stock.volume = meta.value("regularMarketVolume", 0.0);
        
        // Get market cap using Python yfinance library
        stock.market_cap = getMarketCapFromPython(symbol);
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing stock data for " << symbol << ": " << e.what() << std::endl;
    }
    
    return stock;
}

std::vector<HistoricalData> ApiClient::fetchHistoricalData(const std::string& symbol, int days) {
    std::vector<HistoricalData> history;
    
    try {
        std::string url = buildYahooFinanceUrl(symbol);
        std::string response = makeHttpRequest(url);
        
        if (response.empty()) {
            return history;
        }
        
        auto jsonData = json::parse(response);
        auto chart = jsonData["chart"]["result"][0];
        
        auto timestamps = chart["timestamp"];
        auto quotes = chart["indicators"]["quote"][0];
        
        for (size_t i = 0; i < timestamps.size() && i < (size_t)days; i++) {
            HistoricalData data;
            data.date = std::to_string(timestamps[i].get<long>());
            data.open = quotes["open"][i].is_null() ? 0.0 : quotes["open"][i].get<double>();
            data.high = quotes["high"][i].is_null() ? 0.0 : quotes["high"][i].get<double>();
            data.low = quotes["low"][i].is_null() ? 0.0 : quotes["low"][i].get<double>();
            data.close = quotes["close"][i].is_null() ? 0.0 : quotes["close"][i].get<double>();
            data.volume = quotes["volume"][i].is_null() ? 0.0 : quotes["volume"][i].get<double>();
            
            history.push_back(data);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error fetching historical data for " << symbol << ": " << e.what() << std::endl;
    }
    
    return history;
}

std::vector<Stock> ApiClient::fetchTopTen() {
    // Popular stocks to track (you can modify this list)
    std::vector<std::string> symbols = {
        "AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", 
        "META", "NVDA", "JPM", "V", "WMT"
    };
    
    std::vector<Stock> stocks;
    
    for (const auto& symbol : symbols) {
        Stock stock = fetchStockQuote(symbol);
        if (stock.price > 0) { // Only add valid stocks
            stocks.push_back(stock);
        }
    }
    
    return stocks;
}
