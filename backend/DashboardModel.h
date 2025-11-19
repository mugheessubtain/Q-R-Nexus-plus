#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

// Use nlohmann::json for convenience
using json = nlohmann::json;

namespace models {

/**
 * @brief Represents a single candlestick.
 */
struct Candle {
    long long time;
    double open;
    double high;
    double low;
    double close;
    double volume;

    // Helper to convert this struct to JSON
    json toJson() const {
        return {
            {"time", time},
            {"open", open},
            {"close", close},
            {"high", high},
            {"low", low},
            {"volume", volume}
        };
    }
};


/**
 * @brief Polymorphic base model for all dashboard assets (Crypto, Stock, etc.)
 * Provides a unified interface for data controllers and services.
 */
struct DashboardModel {
    std::string symbol;
    std::string name; // Full name, e.g., "Bitcoin"
    std::string interval;
    std::string type; // "crypto" or "stock"
    std::vector<Candle> candles;

    virtual ~DashboardModel() = default;

    /**
     * @brief Serializes the complete model to nlohmann::json.
     * This will be overridden by derived classes to add specific fields
     * like "exchange".
     */
    virtual json toJson() const {
        json j;
        j["symbol"] = symbol;
        j["name"] = name;
        j["type"] = type;
        j["interval"] = interval;
        
        // Serialize all candles
        j["candles"] = json::array();
        for(const auto& candle : candles) {
            j["candles"].push_back(candle.toJson());
        }
        return j;
    }
};

} // namespace models