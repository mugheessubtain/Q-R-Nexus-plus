#pragma once
#include <drogon/HttpController.h>

namespace api {
class CryptoController : public drogon::HttpController<CryptoController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CryptoController::getCryptoList, "/api/crypto/list", drogon::Get);
    ADD_METHOD_TO(CryptoController::getCryptoPrice, "/api/crypto/price/{1}", drogon::Get);
    ADD_METHOD_TO(CryptoController::getHistoricalData, "/api/crypto/history/{1}", drogon::Get);
    METHOD_LIST_END

    void getCryptoList(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void getCryptoPrice(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        const std::string& symbol);
    void getHistoricalData(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& symbol);
};
}