//AIController.h

#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpClient.h>
#include <nlohmann/json.hpp>
#include "DataService.h"

using namespace drogon;
using json = nlohmann::json;

class AIController : public drogon::HttpController<AIController> {
public:
    METHOD_LIST_BEGIN
        // POST /api/ai/chat       -> { "prompt": "..." }
        ADD_METHOD_TO(AIController::chat, "/chat", Post);

        // POST /api/ai/augmented  -> { "data": "long text / chart / structure ..." }
        ADD_METHOD_TO(AIController::augmented, "/augmented", Post);

        // POST /api/ai/analyze    -> { "question": "Give me last 2 months BTCUSDT candle data" }
        ADD_METHOD_TO(AIController::analyzeData, "/analyze", Post);
    METHOD_LIST_END

    void chat(const HttpRequestPtr& req,
              std::function<void (const HttpResponsePtr &) > &&callback);

    void augmented(const HttpRequestPtr& req,
                   std::function<void (const HttpResponsePtr &) > &&callback);

    void analyzeData(const HttpRequestPtr& req,
                     std::function<void (const HttpResponsePtr &) > &&callback);

private:
    // Calls Groq (or any LLM endpoint) and returns text result.
    // Throws on fatal error or returns error text starting with "ERROR:".
    std::string callGroq(const std::string& userPrompt);

    // Persona chunk injected into every prompt
    const std::string personaChunk =
        "You are a professional crypto & stock market analyst. "
        "You specialize in market structure, technical analysis, execution models, "
        "and interpreting candle data. "
        "Always provide concise, accurate, expert-level responses. "
        "Never hallucinate missing information. "
        "If something is unclear or unknown, say it directly. "
        "Keep your tone analytical and objective.\n\n";
};