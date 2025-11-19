#include "CryptoController.h"
#include <drogon/HttpClient.h>
#include <json/json.h>

using namespace api;
using namespace drogon;

const std::string BINANCE_API = "https://api.binance.com";

// void CryptoController::getCryptoList(
//     const HttpRequestPtr& req,
//     std::function<void(const HttpResponsePtr&)>&& callback) {
    
//     auto client = HttpClient::newHttpClient(BINANCE_API);
//     auto request = HttpRequest::newHttpRequest();
//     request->setPath("/api/v3/ticker/24hr");
//     request->setMethod(Get);

//     client->sendRequest(request, [callback](ReqResult result, 
//                                            const HttpResponsePtr& response) {
//         if (result == ReqResult::Ok) {
//             auto resp = HttpResponse::newHttpResponse();
//             resp->setBody(std::string(response->getBody()));
//             resp->setContentTypeCode(CT_APPLICATION_JSON);
//             resp->addHeader("Access-Control-Allow-Origin", "*");
//             callback(resp);
//         } else {
//             auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
//             resp->setStatusCode(k500InternalServerError);
//             callback(resp);
//         }
//     });
// }


void CryptoController::getCryptoList(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

    // Hardcoded crypto data (similar to Binance API structure)
    Json::Value cryptoList(Json::arrayValue);

    Json::Value btc;
    btc["symbol"] = "BTCUSDT";
    btc["priceChange"] = "200.50";
    btc["priceChangePercent"] = "0.72";
    btc["weightedAvgPrice"] = "28050.10";
    btc["lastPrice"] = "28000.50";
    btc["lastQty"] = "0.123";
    btc["bidPrice"] = "27995.00";
    btc["askPrice"] = "28005.00";
    cryptoList.append(btc);

    Json::Value eth;
    eth["symbol"] = "ETHUSDT";
    eth["priceChange"] = "15.75";
    eth["priceChangePercent"] = "0.88";
    eth["weightedAvgPrice"] = "1795.30";
    eth["lastPrice"] = "1800.75";
    eth["lastQty"] = "1.5";
    eth["bidPrice"] = "1799.50";
    eth["askPrice"] = "1801.00";
    cryptoList.append(eth);

    Json::Value bnb;
    bnb["symbol"] = "BNBUSDT";
    bnb["priceChange"] = "5.25";
    bnb["priceChangePercent"] = "1.61";
    bnb["weightedAvgPrice"] = "325.50";
    bnb["lastPrice"] = "330.25";
    bnb["lastQty"] = "2.0";
    bnb["bidPrice"] = "329.80";
    bnb["askPrice"] = "330.50";
    cryptoList.append(bnb);

    // Create JSON response
    auto resp = HttpResponse::newHttpJsonResponse(cryptoList);
    resp->addHeader("Access-Control-Allow-Origin", "*"); // Allow CORS for frontend
    callback(resp);
}

void CryptoController::getCryptoPrice(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& symbol) {
    
    // Hardcoded example response
    Json::Value json;
    json["symbol"] = symbol;
    json["price"] = "34000.50";  // example price

    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}

void CryptoController::getHistoricalData(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& symbol) {

    Json::Value json(Json::arrayValue);

    // Hardcoded historical data example
    for (int i = 0; i < 5; i++) {
        Json::Value entry;
        entry["time"] = 1699152000 + i * 3600;  // example UNIX timestamp
        entry["open"] = 34000.0 + i * 10;
        entry["high"] = 34100.0 + i * 10;
        entry["low"] = 33950.0 + i * 10;
        entry["close"] = 34050.0 + i * 10;
        entry["volume"] = 12.5 + i;
        json.append(entry);
    }

    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}


// void CryptoController::getCryptoPrice(
//     const HttpRequestPtr& req,
//     std::function<void(const HttpResponsePtr&)>&& callback,
//     const std::string& symbol) {
    
//     auto client = HttpClient::newHttpClient(BINANCE_API);
//     auto request = HttpRequest::newHttpRequest();
//     request->setPath("/api/v3/ticker/price?symbol=" + symbol);
//     request->setMethod(Get);

//     client->sendRequest(request, [callback](ReqResult result, 
//                                            const HttpResponsePtr& response) {
//         if (result == ReqResult::Ok) {
//             auto resp = HttpResponse::newHttpResponse();
//             resp->setBody(std::string(response->getBody()));
//             resp->setContentTypeCode(CT_APPLICATION_JSON);
//             resp->addHeader("Access-Control-Allow-Origin", "*");
//             callback(resp);
//         } else {
//             auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
//             resp->setStatusCode(k500InternalServerError);
//             callback(resp);
//         }
//     });
// }

// void CryptoController::getHistoricalData(
//     const HttpRequestPtr& req,
//     std::function<void(const HttpResponsePtr&)>&& callback,
//     const std::string& symbol) {
    
//     auto params = req->getParameters();
//     std::string interval = params.find("interval") != params.end() ? 
//                           params.at("interval") : "1h";
//     std::string limit = params.find("limit") != params.end() ? 
//                        params.at("limit") : "100";

//     auto client = HttpClient::newHttpClient(BINANCE_API);
//     auto request = HttpRequest::newHttpRequest();
//     request->setPath("/api/v3/klines?symbol=" + symbol + 
//                     "&interval=" + interval + 
//                     "&limit=" + limit);
//     request->setMethod(Get);

//     client->sendRequest(request, [callback](ReqResult result, 
//                                            const HttpResponsePtr& response) {
//         if (result == ReqResult::Ok) {
//             auto resp = HttpResponse::newHttpResponse();
//             resp->setBody(std::string(response->getBody()));
//             resp->setContentTypeCode(CT_APPLICATION_JSON);
//             resp->addHeader("Access-Control-Allow-Origin", "*");
//             callback(resp);
//         } else {
//             auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
//             resp->setStatusCode(k500InternalServerError);
//             callback(resp);
//         }
//     });
// }