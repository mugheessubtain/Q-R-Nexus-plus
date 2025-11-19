#pragma once
#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "DataService.h"
#include "AnalysisResult.h"
#include "GroqClient.h"

class MarketService {
public:
    // Singleton accessor
    static MarketService& instance();

    // Optional init to set dependencies
    void init();

    // Fetch candle data for a symbol/interval and approximate range in days
    std::shared_ptr<models::CryptoModel> getCryptoRange(
        const std::string& symbol,
        const std::string& interval,
        int rangeDays);

    // Ask AI (Groq) to analyze model
    AnalysisResult analyzeModelWithAI(
        const std::string& symbol,
        const std::string& interval,
        const std::shared_ptr<models::CryptoModel>& model,
        const std::string& userQuestion = "");

private:
    MarketService() = default;   // singleton private constructor
    GroqClient groq_;
    DataService* dataService_ = nullptr;

    // Build compact textual/JSON representation of N recent candles
    std::string buildCandlesPayload(
        const std::shared_ptr<models::CryptoModel>& model,
        size_t maxPoints = 200);
};
