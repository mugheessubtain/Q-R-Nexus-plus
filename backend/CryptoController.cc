




#include "CryptoController.h"
#include <drogon/HttpClient.h>
#include <json/json.h>

using namespace drogon;

void CryptoController::getCryptoList(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

    Json::Value cryptoList(Json::arrayValue);

    // Vector example (DSA) for storing crypto data
    std::vector<Json::Value> cryptos;

    Json::Value btc;
    btc["symbol"] = "BTCUSDT"; btc["price"] = "28000";
    Json::Value eth;
    eth["symbol"] = "ETHUSDT"; eth["price"] = "1800";
    cryptos.push_back(btc);
    cryptos.push_back(eth);

    for (auto& item : cryptos) cryptoList.append(item);

    auto resp = HttpResponse::newHttpJsonResponse(cryptoList);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}
