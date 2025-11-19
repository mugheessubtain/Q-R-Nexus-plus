#pragma once

#include <drogon/HttpController.h>
#include "DataService.h"

using namespace drogon;

class DataController : public drogon::HttpController<DataController> {
public:
    // Base path for all routes in this controller
    static constexpr const char* path() { return "/api/v1/data"; }

    METHOD_LIST_BEGIN
    METHOD_ADD(DataController::getCryptoData, "/crypto/{1}", Get);
    METHOD_ADD(DataController::getStockData, "/stock/{1}", Get);
    METHOD_ADD(DataController::searchSymbol, "/search/{1}", Get);
    METHOD_ADD(DataController::healthCheck, "/health", Get);
    METHOD_LIST_END

    void getCryptoData(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& symbol);

    void getStockData(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      const std::string& symbol);

    void searchSymbol(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      const std::string& query);

    void healthCheck(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback);
};
