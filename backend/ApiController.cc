#include "ApiController.h"

void ApiController::hello(const drogon::HttpRequestPtr& req,
                          std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value ret;
    ret["message"] = "Hello from Drogon C++ backend!";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}

void ApiController::data(const drogon::HttpRequestPtr& req,
                         std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();
    std::string name = (*json)["name"].asString();

    Json::Value ret;
    ret["message"] = "Hello " + name + "! Data received!";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}