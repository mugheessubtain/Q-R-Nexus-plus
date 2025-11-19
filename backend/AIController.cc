// //AIController.cpp
// #include "AIController.h"
// #include <drogon/drogon.h>
// #include <iostream>
// #include <regex>

// using namespace drogon;
// using json = nlohmann::json;

// // ----------------------
// // Helper: call Groq API
// // ----------------------
// std::string AIController::callGroq(const std::string& userPrompt) {
//     // finalPrompt = persona + user content
//     std::string finalPrompt = personaChunk + userPrompt;

//     // Read API key from config
//     const auto &config = drogon::app().getCustomConfig();
//     if (!config.isMember("groq_api_key") || config["groq_api_key"].asString().empty()) {
//         return std::string("ERROR: 'groq_api_key' missing in config.");
//     }
//     std::string apiKey = config["groq_api_key"].asString();

//     // Prepare HTTP client
//     // Note: change base URL/path if your Groq endpoint differs
//     auto client = drogon::HttpClient::newHttpClient("https://api.groq.com");

//     auto req = drogon::HttpRequest::newHttpRequest();
//     req->setMethod(drogon::Post);
//     req->setPath("/openai/v1/chat/completions"); // compatible path used earlier
//     req->addHeader("Content-Type", "application/json");
//     req->addHeader("Authorization", "Bearer " + apiKey);

//     // Build request body (messages style)
//     json body;
//     body["model"] = "gpt-4o"; // replace with desired Groq model if needed
//     body["messages"] = json::array();
//     json msg;
//     msg["role"] = "user";
//     msg["content"] = finalPrompt;
//     body["messages"].push_back(msg);

//     req->setBody(body.dump());

//     try {
//         auto respPair = client->sendRequest(req, 15.0); // 15s timeout
//         if (respPair.first != ReqResult::Ok || !respPair.second) {
//             std::string err = "ERROR: Groq HTTP request failed (network).";
//             return err;
//         }
//         auto resp = respPair.second;
//         if (resp->statusCode() < 200 || resp->statusCode() >= 300) {
//             return std::string("ERROR: Groq returned HTTP ") + std::to_string(resp->statusCode());
//         }

//         std::string bodyStr = resp->getBody();
//         // Try parse JSON and extract common response layout
//         try {
//             auto parsed = json::parse(bodyStr);
//             // try new "choices"[0]["message"]["content"]
//             if (parsed.contains("choices") && parsed["choices"].is_array() && !parsed["choices"].empty()) {
//                 auto &c0 = parsed["choices"][0];
//                 if (c0.contains("message") && c0["message"].contains("content")) {
//                     return c0["message"]["content"].get<std::string>();
//                 }
//                 if (c0.contains("text")) { // fallback
//                     return c0["text"].get<std::string>();
//                 }
//             }
//             // fallback: if top-level "output" or "result"
//             if (parsed.contains("output")) return parsed["output"].dump();
//             if (parsed.contains("result")) return parsed["result"].dump();

//             // If we couldn't find structured field, return raw body
//             return bodyStr;
//         } catch (const std::exception &e) {
//             // If JSON parse fails, return raw body (it might be plain text)
//             return bodyStr;
//         }
//     } catch (const std::exception &e) {
//         return std::string("ERROR: Exception contacting Groq: ") + e.what();
//     }
// }

// // ----------------------
// // Endpoint: /api/ai/chat
// // ----------------------
// void AIController::chat(const HttpRequestPtr& req,
//                         std::function<void (const HttpResponsePtr &) > &&callback) {
//     auto j = req->getJsonObject();
//     if (!j || !(*j).isMember("prompt")) {
//         json err = { {"error", "Missing 'prompt' in JSON body"} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     std::string prompt = (*j)["prompt"].get<std::string>();
//     std::string aiResp = callGroq(prompt);

//     if (aiResp.rfind("ERROR:", 0) == 0) {
//         json err = { {"error", aiResp} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     json out = { {"response", aiResp} };
//     callback(HttpResponse::newHttpJsonResponse(out));
// }

// // -------------------------
// // Endpoint: /api/ai/augmented
// // -------------------------
// void AIController::augmented(const HttpRequestPtr& req,
//                              std::function<void (const HttpResponsePtr &) > &&callback) {
//     auto j = req->getJsonObject();
//     if (!j || !(*j).isMember("data")) {
//         json err = { {"error", "Missing 'data' in JSON body"} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     std::string data = (*j)["data"].get<std::string>();

//     // Tell the model this is augmented content and must be treated as text
//     std::string prompt =
//         "This is an augmented request. The user may include charts, tables, CSV-like blocks, or structured text. "
//         "Treat the entirety strictly as text and analyze or summarize the structure. "
//         "Return clear, expert analysis and label sections you identify where helpful.\n\n"
//         "USER DATA:\n" + data;

//     std::string aiResp = callGroq(prompt);

//     if (aiResp.rfind("ERROR:", 0) == 0) {
//         json err = { {"error", aiResp} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     json out = { {"response", aiResp} };
//     callback(HttpResponse::newHttpJsonResponse(out));
// }

// // -------------------------
// // Endpoint: /api/ai/analyze
// // -------------------------
// // This endpoint asks the AI to extract a structured data request (symbol, interval, range)
// // and then uses DataService to fetch the requested candles. The AI MUST return JSON and
// // we attempt to parse it. If parsing fails fallbacks are tried.
// void AIController::analyzeData(const HttpRequestPtr& req,
//                                std::function<void (const HttpResponsePtr &) > &&callback) {
//     auto j = req->getJsonObject();
//     if (!j || !(*j).isMember("question")) {
//         json err = { {"error", "Missing 'question' in JSON body"} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     std::string question = (*j)["question"].get<std::string>();

//     // Instruct the model to return only JSON in expected schema
//     std::string instruct =
//         "You are an expert data-parsing assistant. Extract trading data request parameters from the user question. "
//         "Return ONLY valid JSON (no explanation) in the exact form:\n"
//         "{ \"symbol\": \"BTCUSDT\", \"interval\": \"1d\", \"range\": \"60d\" }\n"
//         "Where 'range' must be one of: 7d, 14d, 30d, 60d. Interval should be standard (1m,5m,15m,1h,4h,1d). "
//         "If the user doesn't specify interval, choose 1d for multi-week requests and 1h for shorter requests.\n\n"
//         "User question:\n" + question + "\n\nOnly output the JSON.";

//     std::string aiParsed = callGroq(instruct);
//     if (aiParsed.rfind("ERROR:", 0) == 0) {
//         json err = { {"error", aiParsed} };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     // Try parse JSON returned by model
//     json parsedObj;
//     bool parsedOk = false;
//     try {
//         parsedObj = json::parse(aiParsed);
//         parsedOk = true;
//     } catch (...) {
//         // Try to extract JSON-like substring using regex as fallback
//         std::smatch m;
//         std::regex re(R"(\{[\s\S]*\})");
//         std::string s = aiParsed;
//         if (std::regex_search(s, m, re)) {
//             try {
//                 parsedObj = json::parse(m.str(0));
//                 parsedOk = true;
//             } catch (...) {
//                 parsedOk = false;
//             }
//         }
//     }

//     if (!parsedOk) {
//         json err = {
//             {"error", "Failed to parse JSON from AI"},
//             {"ai_raw_output", aiParsed}
//         };
//         callback(HttpResponse::newHttpJsonResponse(err));
//         return;
//     }

//     // Extract fields with defaults & safety checks
//     std::string symbol = "BTCUSDT";
//     std::string interval = "1d";
//     std::string range = "30d";

//     if (parsedObj.contains("symbol") && parsedObj["symbol"].is_string()) symbol = parsedObj["symbol"].get<std::string>();
//     if (parsedObj.contains("interval") && parsedObj["interval"].is_string()) interval = parsedObj["interval"].get<std::string>();
//     if (parsedObj.contains("range") && parsedObj["range"].is_string()) range = parsedObj["range"].get<std::string>();

//     // Map range to an approximate limit or internal decision
//     // (DataService currently fetches latest 100 by default; if you implement pagination adapt accordingly)
//     // We'll fetch from DataService using the interval the AI provided.
//     auto &service = DataService::instance();

//     // If it's a stock request (symbol likely all letters or contains no USDT), you may want to use getStockData.
//     // We'll detect simple heuristics: if symbol contains "USDT" or digits -> crypto, else stock.
//     bool isCrypto = (symbol.find("USDT") != std::string::npos || symbol.find('/') != std::string::npos ||
//                      std::any_of(symbol.begin(), symbol.end(), ::isdigit));

//     json output;
//     output["ai_parsed"] = parsedObj;
//     output["requested"] = { {"symbol", symbol}, {"interval", interval}, {"range", range}, {"isCrypto", isCrypto} };

//     if (isCrypto) {
//         try {
//             auto cryptoModel = service.getCryptoData(symbol, interval);
//             if (!cryptoModel) {
//                 output["error"] = "DataService returned no crypto data for the requested symbol/interval.";
//                 callback(HttpResponse::newHttpJsonResponse(output));
//                 return;
//             }

//             json arr = json::array();
//             for (const auto &c : cryptoModel->candles) {
//                 json cand;
//                 cand["time"] = c.time;
//                 cand["open"] = c.open;
//                 cand["high"] = c.high;
//                 cand["low"] = c.low;
//                 cand["close"] = c.close;
//                 cand["volume"] = c.volume;
//                 arr.push_back(cand);
//             }
//             output["candles"] = arr;
//             callback(HttpResponse::newHttpJsonResponse(output));
//             return;
//         } catch (const std::exception &e) {
//             output["error"] = std::string("Exception while fetching crypto data: ") + e.what();
//             callback(HttpResponse::newHttpJsonResponse(output));
//             return;
//         }
//     } else {
//         // Stock path
//         try {
//             auto stockModel = service.getStockData(symbol, interval);
//             if (!stockModel) {
//                 output["error"] = "DataService returned no stock data for the requested symbol/interval.";
//                 callback(HttpResponse::newHttpJsonResponse(output));
//                 return;
//             }

//             json arr = json::array();
//             for (const auto &c : stockModel->candles) {
//                 json cand;
//                 cand["time"] = c.time;
//                 cand["open"] = c.open;
//                 cand["high"] = c.high;
//                 cand["low"] = c.low;
//                 cand["close"] = c.close;
//                 cand["volume"] = c.volume;
//                 arr.push_back(cand);
//             }
//             output["candles"] = arr;
//             callback(HttpResponse::newHttpJsonResponse(output));
//             return;
//         } catch (const std::exception &e) {
//             output["error"] = std::string("Exception while fetching stock data: ") + e.what();
//             callback(HttpResponse::newHttpJsonResponse(output));
//             return;
//         }
//     }
// }











#include "AIController.h"
#include <drogon/drogon.h>
#include <drogon/HttpResponse.h>
#include <json/json.h> // JsonCpp
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
    body["model"] = "gpt-4o";
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

        auto parsed = json::parse(resp->getBody());
        if (parsed.contains("choices") && parsed["choices"].is_array() && !parsed["choices"].empty()) {
            auto &c0 = parsed["choices"][0];
            if (c0.contains("message") && c0["message"].contains("content"))
                return c0["message"]["content"].get<std::string>();
            if (c0.contains("text")) return c0["text"].get<std::string>();
        }
        if (parsed.contains("output")) return parsed["output"].dump();
        if (parsed.contains("result")) return parsed["result"].dump();
       return std::string(resp->getBody());

    } catch (const std::exception &e) {
        return "ERROR: Exception contacting Groq: " + std::string(e.what());
    }
}

// ----------------------
// Endpoint: /api/ai/chat
// ----------------------
void AIController::chat(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback) {
    auto j = req->getJsonObject();
    if (!j || !(*j).isMember("prompt")) {
        Json::Value err;
        err["error"] = "Missing 'prompt' in JSON body";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    std::string prompt = (*j)["prompt"].asString();
    std::string aiResp = callGroq(prompt);

    Json::Value out;
    if (aiResp.rfind("ERROR:", 0) == 0)
        out["error"] = aiResp;
    else
        out["response"] = aiResp;

    callback(HttpResponse::newHttpJsonResponse(out));
}

// -------------------------
// Endpoint: /api/ai/augmented
// -------------------------
void AIController::augmented(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
    auto j = req->getJsonObject();
    if (!j || !(*j).isMember("data")) {
        Json::Value err;
        err["error"] = "Missing 'data' in JSON body";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    std::string data = (*j)["data"].asString();
    std::string prompt =
        "This is an augmented request. Treat as text. USER DATA:\n" + data;
    std::string aiResp = callGroq(prompt);

    Json::Value out;
    if (aiResp.rfind("ERROR:", 0) == 0)
        out["error"] = aiResp;
    else
        out["response"] = aiResp;

    callback(HttpResponse::newHttpJsonResponse(out));
}

// -------------------------
// Endpoint: /api/ai/analyze
// -------------------------
void AIController::analyzeData(const HttpRequestPtr &req,
                               std::function<void(const HttpResponsePtr &)> &&callback) {
    auto j = req->getJsonObject();
    if (!j || !(*j).isMember("question")) {
        Json::Value err;
        err["error"] = "Missing 'question' in JSON body";
        callback(HttpResponse::newHttpJsonResponse(err));
        return;
    }

    std::string question = (*j)["question"].asString();

    std::string instruct =
        "Extract trading data request parameters from the question. "
        "Return ONLY JSON {\"symbol\":\"BTCUSDT\",\"interval\":\"1d\",\"range\":\"60d\"}. "
        "User question: " + question;

    std::string aiParsed = callGroq(instruct);

    Json::Value parsedObj;
    try {
        parsedObj = nlohmannToJsonCpp(json::parse(aiParsed));
    } catch (...) {
        Json::Value fallback;
        fallback["error"] = "Failed to parse JSON from AI";
        fallback["ai_raw_output"] = aiParsed;
        callback(HttpResponse::newHttpJsonResponse(fallback));
        return;
    }

    callback(HttpResponse::newHttpJsonResponse(parsedObj));
}
