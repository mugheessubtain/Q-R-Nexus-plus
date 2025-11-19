// GroqClient.cpp
#include "GroqClient.h"
#include <drogon/drogon.h>
#include <drogon/HttpClient.h>
#include <iostream>

using json = nlohmann::json;
using namespace drogon;

GroqClient::GroqClient() {
    const auto &cfg = drogon::app().getCustomConfig();

    if (cfg.isMember("groq_api_key")) apiKey_ = cfg["groq_api_key"].asString();
    if (cfg.isMember("groq_base_url")) baseUrl_ = cfg["groq_base_url"].asString();
    if (cfg.isMember("groq_api_path")) apiPath_ = cfg["groq_api_path"].asString();
    if (cfg.isMember("groq_model")) modelName_ = cfg["groq_model"].asString();

    // sensible defaults
    if (baseUrl_.empty()) baseUrl_ = "https://api.groq.com";
    if (apiPath_.empty()) apiPath_ = "/openai/v1/chat/completions";
    if (modelName_.empty()) modelName_ = "gpt-4o";

    personaChunk_ =
        "You are a professional crypto & stock market analyst. "
        "You specialize in market structure, technical analysis, execution models, "
        "and interpreting candle data. "
        "Always provide concise, accurate, expert-level responses. "
        "Never hallucinate missing information. "
        "If something is unclear or unknown, say it directly. "
        "Keep your tone analytical and objective.\n\n";
}

std::string GroqClient::chat(const std::string& prompt, double timeout_seconds) {
    if (apiKey_.empty()) {
        return std::string("ERROR: 'groq_api_key' not set in config.");
    }

    std::string finalPrompt = personaChunk_ + prompt;
    auto client = HttpClient::newHttpClient(baseUrl_);
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath(apiPath_);
    req->addHeader("Content-Type", "application/json");
    req->addHeader("Authorization", "Bearer " + apiKey_);

    json body;
    body["model"] = modelName_;
    body["messages"] = json::array();
    body["messages"].push_back({
        {"role", "user"},
        {"content", finalPrompt}
    });
    req->setBody(body.dump());

    try {
        auto respPair = client->sendRequest(req, timeout_seconds);
        if (respPair.first != ReqResult::Ok || !respPair.second) {
            return std::string("ERROR: Network request to Groq failed.");
        }

        auto resp = respPair.second;
        int status = resp->statusCode();
        std::string bodyStr(resp->getBody());  // <-- convert string_view to string

        if (status < 200 || status >= 300) {
            return std::string("ERROR: Groq HTTP ") + std::to_string(status) + " - " + bodyStr;
        }

        try {
            auto parsed = json::parse(bodyStr);
            if (parsed.contains("choices") && parsed["choices"].is_array() && !parsed["choices"].empty()) {
                auto &c0 = parsed["choices"][0];
                if (c0.contains("message") && c0["message"].contains("content")) {
                    return c0["message"]["content"].get<std::string>();
                }
                if (c0.contains("text")) {
                    return c0["text"].get<std::string>();
                }
            }
            if (parsed.contains("output")) return parsed["output"].dump();
            if (parsed.contains("result")) return parsed["result"].dump();

            // fallback: return raw body
            return bodyStr;
        } catch (const std::exception &e) {
            // Not JSON or parsing failed; return raw body
            return bodyStr;
        }
    } catch (const std::exception &e) {
        return std::string("ERROR: Exception contacting Groq: ") + e.what();
    }
}
