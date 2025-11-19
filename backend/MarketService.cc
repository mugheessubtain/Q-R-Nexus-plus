// //MarketService.cpp
// #include "MarketService.h"
// #include <sstream>
// #include <iomanip>
// #include <drogon/drogon.h>

// using json = nlohmann::json;

// // Singleton
// MarketService& MarketService::instance() {
//     static MarketService s;
//     return s;
// }

// void MarketService::init() {
//     dataService_ = &DataService::instance();
// }

// // Attempt to fetch data using DataService. DataService currently returns up to 100 points normally.
// // rangeDays is advisory: depending on interval, your actual returned candles count may vary.
// std::shared_ptr<models::CryptoModel> MarketService::getCryptoRange(const std::string& symbol,
//                                                                    const std::string& interval,
//                                                                    int rangeDays) {
//     if (!dataService_) dataService_ = &DataService::instance();

//     // Basic heuristic: if user asks for many days, use daily interval if not provided.
//     // But caller should pass the desired interval.
//     try {
//         auto model = dataService_->getCryptoData(symbol, interval);
//         return model;
//     } catch (const std::exception &e) {
//         drogon::logger::warn("MarketService::getCryptoRange exception: {}", e.what());
//         return nullptr;
//     }
// }

// // Prepare a compact CSV/JSON string of recent candles for the prompt (time,open,high,low,close,volume)
// std::string MarketService::buildCandlesPayload(const std::shared_ptr<models::CryptoModel>& model, size_t maxPoints) {
//     if (!model) return "";

//     std::ostringstream ss;
//     ss << "CANDLES_JSON:\n";
//     json arr = json::array();

//     size_t start = 0;
//     if (model->candles.size() > maxPoints) start = model->candles.size() - maxPoints;

//     for (size_t i = start; i < model->candles.size(); ++i) {
//         const auto &c = model->candles[i];
//         json cand;
//         cand["t"] = c.time;
//         cand["o"] = c.open;
//         cand["h"] = c.high;
//         cand["l"] = c.low;
//         cand["c"] = c.close;
//         cand["v"] = c.volume;
//         arr.push_back(cand);
//     }
//     ss << arr.dump();
//     return ss.str();
// }

// // Ask Groq to analyze the model + user question and return structured AnalysisResult if possible.
// // If Groq doesn't return structured JSON, we stash the raw text into summary.
// AnalysisResult MarketService::analyzeModelWithAI(const std::string& symbol,
//                                                  const std::string& interval,
//                                                  const std::shared_ptr<models::CryptoModel>& model,
//                                                  const std::string& userQuestion) {
//     AnalysisResult result;
//     result.symbol = symbol;

//     // Build prompt
//     std::ostringstream prompt;
//     prompt << "You are an expert market analyst. The user asked: \"" << userQuestion << "\"\n\n";
//     prompt << "Provide a concise JSON analysis with fields: symbol, sentiment, summary, trend, keyLevels (array), confidence (0-1).\n";
//     prompt << "Also include a short human-readable summary after the JSON separated by '---'.\n\n";
//     // Include candles
//     const auto candlesPayload = buildCandlesPayload(model, 400);
//     if (!candlesPayload.empty()) {
//         prompt << "Here are recent candles (JSON array) for analysis:\n" << candlesPayload << "\n\n";
//     } else {
//         prompt << "No candle data available for " << symbol << ".\n\n";
//     }

//     prompt << "Return ONLY the JSON object first. Then after the JSON you may optionally add short notes separated by ---.\n";
//     // Ask for conservative answers and no hallucinations
//     prompt << "Be conservative: do not invent precise numeric levels; round levels to 2 decimal places and set confidence appropriately.\n";

//     auto aiReply = groq_.chat(prompt.str(), 25.0);

//     if (aiReply.rfind("ERROR:", 0) == 0) {
//         result.summary = aiReply;
//         result.sentiment = "unknown";
//         result.trend = "unknown";
//         result.confidence = 0.0;
//         return result;
//     }

//     // Try to extract first JSON object from AI reply
//     std::smatch m;
//     std::regex re(R"(\{[\s\S]*?\})");
//     std::string s = aiReply;
//     bool parsed = false;
//     try {
//         if (std::regex_search(s, m, re)) {
//             std::string jsonText = m.str(0);
//             auto j = json::parse(jsonText);

//             // Map to AnalysisResult (we use from_json helper)
//             try {
//                 result = j.get<AnalysisResult>();
//                 parsed = true;
//             } catch (...) {
//                 // Could not map directly; try manual extraction
//                 if (j.contains("summary")) result.summary = j.value("summary", "");
//                 if (j.contains("sentiment")) result.sentiment = j.value("sentiment", "");
//                 if (j.contains("trend")) result.trend = j.value("trend", "");
//                 if (j.contains("keyLevels") && j["keyLevels"].is_array()) {
//                     for (auto &k : j["keyLevels"]) result.keyLevels.push_back(k.get<std::string>());
//                 }
//                 if (j.contains("confidence")) result.confidence = j.value("confidence", 0.0);
//                 parsed = true;
//             }
//         }
//     } catch (const std::exception &e) {
//         // parse fail - fall through to raw text
//         drogon::logger::warn("MarketService::analyzeModelWithAI parse exception: {}", e.what());
//     }

//     if (!parsed) {
//         // fallback: put full AI text into summary
//         result.summary = aiReply;
//         result.sentiment = "unknown";
//         result.trend = "unknown";
//         result.confidence = 0.0;
//     }

//     return result;
// }





#include "MarketService.h"
#include <drogon/drogon.h>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Singleton
MarketService& MarketService::instance() {
    static MarketService s;
    return s;
}

// Init dependencies
void MarketService::init() {
    dataService_ = &DataService::instance();
}

// Fetch crypto range
std::shared_ptr<models::CryptoModel> MarketService::getCryptoRange(
    const std::string& symbol,
    const std::string& interval,
    int rangeDays)
{
    if (!dataService_) dataService_ = &DataService::instance();
    try {
        return dataService_->getCryptoData(symbol, interval);
    } catch (const std::exception& e) {
        LOG_WARN << "MarketService::getCryptoRange exception: " << e.what();
        return nullptr;
    }
}

// Prepare candle payload
std::string MarketService::buildCandlesPayload(
    const std::shared_ptr<models::CryptoModel>& model,
    size_t maxPoints)
{
    if (!model) return "";
    json arr = json::array();
    size_t start = model->candles.size() > maxPoints ? model->candles.size() - maxPoints : 0;

    for (size_t i = start; i < model->candles.size(); ++i) {
        const auto& c = model->candles[i];
        arr.push_back({
            {"t", c.time},
            {"o", c.open},
            {"h", c.high},
            {"l", c.low},
            {"c", c.close},
            {"v", c.volume}
        });
    }

    return arr.dump();
}

// Analyze model with AI
AnalysisResult MarketService::analyzeModelWithAI(
    const std::string& symbol,
    const std::string& interval,
    const std::shared_ptr<models::CryptoModel>& model,
    const std::string& userQuestion)
{
    AnalysisResult result;
    result.symbol = symbol;

    std::ostringstream prompt;
    prompt << "User asked: \"" << userQuestion << "\"\nJSON analysis only.\n";
    prompt << buildCandlesPayload(model, 400);

    std::string aiReply = groq_.chat(prompt.str(), 25.0);

    if (aiReply.rfind("ERROR:", 0) == 0) {
        result.summary = aiReply;
        result.sentiment = "unknown";
        result.trend = "unknown";
        result.confidence = 0.0;
        return result;
    }

    try {
        std::regex re(R"(\{[\s\S]*?\})");
        std::smatch m;
        if (std::regex_search(aiReply, m, re)) {
            json j = json::parse(m.str(0));
            result = j.get<AnalysisResult>();
        } else {
            result.summary = aiReply;
        }
    } catch (...) {
        result.summary = aiReply;
    }

    return result;
}
