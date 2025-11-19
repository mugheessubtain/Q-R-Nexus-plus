#pragma once

#include <drogon/drogon.h>

#include <drogon/HttpClient.h>
#include "CryptoModel.h"
#include "StockModel.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>

using json = nlohmann::json;

/**
 * @brief Singleton service class to handle all data fetching.
 * Manages API clients and business logic.
 */
class DataService {
public:
    // Singleton access
    static DataService& instance();

    // Initialize clients and read API keys from config
    void init();

    /**
     * @brief Gets crypto data. Tries Binance first, then Alpaca as fallback.
     */
    std::shared_ptr<models::CryptoModel> getCryptoData(const std::string& symbol, const std::string& interval);

    /**
     * @brief Gets stock data. Only uses Alpaca.
     */
    std::shared_ptr<models::StockModel> getStockData(const std::string& symbol, const std::string& interval);

    /**
     * @brief (Mocked) Searches for a symbol.
     */
    json searchSymbol(const std::string& query);

private:
    // Private constructor for singleton
    DataService() = default;
    
    // Private helpers for specific API calls
    std::shared_ptr<models::CryptoModel> getCryptoDataFromBinance(const std::string& symbol, const std::string& interval);
    std::shared_ptr<models::CryptoModel> getCryptoDataFromAlpaca(const std::string& symbol, const std::string& interval);


    // API clients
    drogon::HttpClientPtr binanceClient;
    drogon::HttpClientPtr alpacaClient;

    // API keys
    std::string alpacaApiKey;
    std::string alpacaApiSecret;
};