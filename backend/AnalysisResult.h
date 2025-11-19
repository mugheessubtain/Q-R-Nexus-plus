#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Simple struct to store AI analysis result
struct AnalysisResult {
    std::string symbol;
    std::string sentiment;      // "bullish", "neutral", "bearish"
    std::string summary;        // textual summary
    std::string trend;          // "uptrend", "downtrend", "range"
    std::vector<std::string> keyLevels; // support/resistance levels
    double confidence = 0.0;    // 0.0 - 1.0
    AnalysisResult() = default;
};

// nlohmann::json conversions
inline void to_json(nlohmann::json& j, const AnalysisResult& a) {
    j = nlohmann::json{
        {"symbol", a.symbol},
        {"sentiment", a.sentiment},
        {"summary", a.summary},
        {"trend", a.trend},
        {"keyLevels", a.keyLevels},
        {"confidence", a.confidence}
    };
}

inline void from_json(const nlohmann::json& j, AnalysisResult& a) {
    j.at("symbol").get_to(a.symbol);
    j.at("sentiment").get_to(a.sentiment);
    j.at("summary").get_to(a.summary);
    j.at("trend").get_to(a.trend);
    j.at("keyLevels").get_to(a.keyLevels);
    j.at("confidence").get_to(a.confidence);
}
