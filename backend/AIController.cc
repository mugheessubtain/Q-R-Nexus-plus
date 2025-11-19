#include "AIController.h"
#include <drogon/drogon.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <iostream>
#include <regex>

using namespace drogon;
using json = nlohmann::json;

// Helper: convert nlohmann::json -> Json::Value
Json::Value nlohmannToJsonCpp(const json &j) {
    Json::Value val;
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            val[it.key()] = nlohmannToJsonCpp(it.value());
        }
    } else if (j.is_array()) {
        for (const auto &e : j) val.append(nlohmannToJsonCpp(e));
    } else if (j.is_string()) val = j.get<std::string>();
    else if (j.is_boolean()) val = j.get<bool>();
    else if (j.is_number()) val = j.get<double>();
    else val = Json::nullValue;
    return val;
}

// ----------------------
// Helper: call Groq API
// ----------------------
std::string AIController::callGroq(const std::string &userPrompt) {
    std::string finalPrompt = personaChunk + userPrompt;
    const auto &config = drogon::app().getCustomConfig();
    if (!config.isMember("groq_api_key") || config["groq_api_key"].asString().empty())
        return "ERROR: 'groq_api_key' missing in config.";
    std::string apiKey = config["groq_api_key"].asString();

    auto client = drogon::HttpClient::newHttpClient("https://api.groq.com");
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/openai/v1/chat/completions");
    req->addHeader("Content-Type", "application/json");
    req->addHeader("Authorization", "Bearer " + apiKey);

    json body;
    body["model"] = "llama3-8b-8192"; // Use available Groq model
    body["messages"] = json::array();
    body["messages"].push_back({{"role", "user"}, {"content", finalPrompt}});
    req->setBody(body.dump());

    try {
        auto respPair = client->sendRequest(req, 15.0);
        if (respPair.first != ReqResult::Ok || !respPair.second)
            return "ERROR: Groq HTTP request failed (network).";

        auto resp = respPair.second;
        if (resp->statusCode() < 200 || resp->statusCode() >= 300)
            return "ERROR: Groq returned HTTP " + std::to_string(resp->statusCode());

        std::string bodyStr(resp->getBody()); // Convert string_view to string
        auto parsed = json::parse(bodyStr);
        
        if (parsed.contains("choices") && parsed["choices"].is_array() && !parsed["choices"].empty()) {
            auto &c0 = parsed["choices"][0];
            if (c0.contains("message") && c0["message"].contains("content"))
                return c0["message"]["content"].get<std::string>();
            if (c0.contains("text")) return c0["text"].get<std::string>();
        }
        if (parsed.contains("output")) return parsed["output"].dump();
        if (parsed.contains("result")) return parsed["result"].dump();
        return bodyStr;

    } catch (const std::exception &e) {
        return "ERROR: Exception contacting Groq: " + std::string(e.what());
    }
}

// ----------------------
// Endpoint: /api/ai/chat
// ----------------------
void AIController::chat(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback) {
    // Handle CORS preflight
    if (req->method() == Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        resp->setStatusCode(k200OK);
        callback(resp);
        return;
    }

    auto j = req->getJsonObject();
    if (!j) {
        Json::Value err;
        err["error"] = "Invalid JSON body";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    // Support both "prompt" and "message" fields for flexibility
    std::string prompt;
    if ((*j).isMember("prompt")) {
        prompt = (*j)["prompt"].asString();
    } else if ((*j).isMember("message")) {
        prompt = (*j)["message"].asString();
    } else {
        Json::Value err;
        err["error"] = "Missing 'prompt' or 'message' in JSON body";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    std::string aiResp = callGroq(prompt);

    Json::Value out;
    if (aiResp.rfind("ERROR:", 0) == 0) {
        out["error"] = aiResp;
    } else {
        // Format response to match Gemini-style structure for frontend compatibility
        Json::Value candidate;
        Json::Value content;
        Json::Value part;
        part["text"] = aiResp;
        content["parts"].append(part);
        candidate["content"] = content;
        out["candidates"].append(candidate);
    }

    auto resp = HttpResponse::newHttpJsonResponse(out);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
    callback(resp);
}

// -------------------------
// Endpoint: /api/ai/augmented
// -------------------------
void AIController::augmented(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
    // Handle CORS preflight
    if (req->method() == Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        resp->setStatusCode(k200OK);
        callback(resp);
        return;
    }

    auto j = req->getJsonObject();
    if (!j || !(*j).isMember("data")) {
        Json::Value err;
        err["error"] = "Missing 'data' in JSON body";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    std::string data = (*j)["data"].asString();
    std::string prompt =
        "This is an augmented request. Analyze this trading data and provide insights. USER DATA:\n" + data;
    std::string aiResp = callGroq(prompt);

    Json::Value out;
    if (aiResp.rfind("ERROR:", 0) == 0) {
        out["error"] = aiResp;
    } else {
        // Format response to match Gemini-style structure
        Json::Value candidate;
        Json::Value content;
        Json::Value part;
        part["text"] = aiResp;
        content["parts"].append(part);
        candidate["content"] = content;
        out["candidates"].append(candidate);
    }

    auto resp = HttpResponse::newHttpJsonResponse(out);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}

// -------------------------
// Endpoint: /api/ai/analyze
// -------------------------
void AIController::analyzeData(const HttpRequestPtr &req,
                               std::function<void(const HttpResponsePtr &)> &&callback) {
    // Handle CORS preflight
    if (req->method() == Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        resp->setStatusCode(k200OK);
        callback(resp);
        return;
    }

    auto j = req->getJsonObject();
    if (!j || !(*j).isMember("question")) {
        Json::Value err;
        err["error"] = "Missing 'question' in JSON body";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    std::string question = (*j)["question"].asString();

    std::string instruct =
        "Analyze this trading question and provide a detailed prediction with technical analysis. "
        "Question: " + question;

    std::string aiResp = callGroq(instruct);

    Json::Value out;
    if (aiResp.rfind("ERROR:", 0) == 0) {
        out["error"] = aiResp;
    } else {
        // Format response to match Gemini-style structure
        Json::Value candidate;
        Json::Value content;
        Json::Value part;
        part["text"] = aiResp;
        content["parts"].append(part);
        candidate["content"] = content;
        out["candidates"].append(candidate);
    }

    auto resp = HttpResponse::newHttpJsonResponse(out);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}