// #include "DataController.h"
// #include <string>
// #include <nlohmann/json.hpp>
// // Assuming DataService.h or DataService.cpp has a definition for CryptoModel and StockModel,
// // and that DataService::instance() is thread-safe.

// using json = nlohmann::json;
// using namespace drogon;

// void DataController::getCryptoData(const HttpRequestPtr& req,
//                                        std::function<void(const HttpResponsePtr&)>&& callback,
//                                        const std::string& symbol) {
    
//     // Get interval from query param, default to "1m"
//     std::string interval = req->getParameter("interval");
//     if (interval.empty()) {
//         interval = "1m";
//     }

//     // Call the service - auto model is likely a raw pointer (CryptoModel*)
//     auto model = DataService::instance().getCryptoData(symbol, interval);

//     if (model) {
//         // Use model's toJson() method
//         auto resp = HttpResponse::newHttpResponse();
        
//         // Use try/catch to handle potential serialization errors, though this doesn't stop segfaults.
//         try {
//             // Segfault likely occurs if the pointer 'model' is dangling (pointing to deleted memory).
//             resp->setBody(model->toJson().dump());
//         } catch (const std::exception& e) {
//             LOG_ERROR << "Exception dumping CryptoModel to JSON: " << e.what();
//             // Return 500 if serialization fails
//             auto errorResp = HttpResponse::newHttpResponse();
//             errorResp->setStatusCode(k500InternalServerError);
//             errorResp->setBody("{\"error\":\"Internal error serializing data.\" }");
//             errorResp->setContentTypeCode(CT_APPLICATION_JSON);
//             callback(errorResp);
//             return;
//         }

//         resp->setContentTypeCode(CT_APPLICATION_JSON);
//         callback(resp);
//     } else {
//         // Service failed to return data (e.g., network error or invalid symbol)
//         json error;
//         error["error"] = "Failed to fetch data for symbol " + symbol + " from all sources";
//         auto resp = HttpResponse::newHttpResponse();
//         resp->setStatusCode(k404NotFound);
//         resp->setBody(error.dump());
//         resp->setContentTypeCode(CT_APPLICATION_JSON);
//         callback(resp);
//     }
// }

// void DataController::getStockData(const HttpRequestPtr& req,
//                                      std::function<void(const HttpResponsePtr&)>&& callback,
//                                      const std::string& symbol) {
    
//     std::string interval = req->getParameter("interval");
//     if (interval.empty()) {
//         interval = "1D"; // Default for stocks
//     }

//     // Call the service - auto model is likely a raw pointer (StockModel*)
//     auto model = DataService::instance().getStockData(symbol, interval);

//     if (model) {
//         auto resp = HttpResponse::newHttpResponse();
        
//         try {
//             resp->setBody(model->toJson().dump());
//         } catch (const std::exception& e) {
//             LOG_ERROR << "Exception dumping StockModel to JSON: " << e.what();
//             auto errorResp = HttpResponse::newHttpResponse();
//             errorResp->setStatusCode(k500InternalServerError);
//             errorResp->setBody("{\"error\":\"Internal error serializing data.\" }");
//             errorResp->setContentTypeCode(CT_APPLICATION_JSON);
//             callback(errorResp);
//             return;
//         }

//         resp->setContentTypeCode(CT_APPLICATION_JSON);
//         callback(resp);
//     } else {
//         json error;
//         error["error"] = "Failed to fetch data for symbol " + symbol;
//         auto resp = HttpResponse::newHttpResponse();
//         resp->setStatusCode(k404NotFound);
//         resp->setBody(error.dump());
//         resp->setContentTypeCode(CT_APPLICATION_JSON);
//         callback(resp);
//     }
// }

// void DataController::searchSymbol(const HttpRequestPtr& req,
//                                      std::function<void(const HttpResponsePtr&)>&& callback,
//                                      const std::string& query) {
    
//     // Assuming searchSymbol returns a valid nlohmann::json object that is safe.
//     auto results = DataService::instance().searchSymbol(query);
//     auto resp = HttpResponse::newHttpResponse();
//     resp->setBody(results.dump());
//     resp->setContentTypeCode(CT_APPLICATION_JSON);
//     callback(resp);
// }

// void DataController::healthCheck(const HttpRequestPtr& req,
//                                      std::function<void(const HttpResponsePtr&)>&& callback)
// {
//     // FIX: Use Drogon's built-in Json::Value utility to avoid potential nlohmann::json conflict/crash.
//     Json::Value resp; 
//     resp["status"] = "OK";
//     auto httpResp = HttpResponse::newHttpJsonResponse(resp); // Helper creates JSON response correctly
    
//     callback(httpResp);
// }






















#include "DataController.h"
#include <drogon/HttpController.h>
#include <nlohmann/json.hpp>
#include <deque>

using json = nlohmann::json;
using namespace drogon;

void DataController::getCryptoData(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback,
                                   const std::string& symbol) {
    std::string interval = req->getParameter("interval");
    if (interval.empty()) interval = "1m";

    auto model = DataService::instance().getCryptoData(symbol, interval);

    if (model) {
        // DSA Example: rolling MA for last 5 candles
        std::deque<double> window;
        int maPeriod = 5;
        json maArray = json::array();

        for (auto& candle : model->candles) {
            window.push_back(candle.close);
            if (window.size() > maPeriod) window.pop_front();
            maArray.push_back(calculateRollingMA(window)); // rolling MA using deque
        }

        json resp = model->toJson();
        resp["rollingMA"] = maArray;

        auto httpResp = HttpResponse::newHttpJsonResponse(resp);
        callback(httpResp);
    } else {
        json error;
        error["error"] = "Failed to fetch data for symbol " + symbol;
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}

// Search using hash map
void DataController::searchSymbol(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  const std::string& query) {
    json results = json::array();
    auto& map = DataService::instance().symbolMap;

    for (const auto& [sym, exchange] : map) {
        if (sym.find(query) != std::string::npos) {
            results.push_back({{"symbol", sym}, {"exchange", exchange}});
        }
    }

    auto resp = HttpResponse::newHttpJsonResponse(results);
    callback(resp);
}
