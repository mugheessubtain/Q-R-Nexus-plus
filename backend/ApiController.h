#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class ApiController : public drogon::HttpController<ApiController>
{
  public:
    METHOD_LIST_BEGIN
    //use METHOD_ADD to add your custom processing function here;
    METHOD_ADD(ApiController::hello, "/api/hello", Get);
    METHOD_ADD(ApiController::data, "/api/data", Post);
    METHOD_LIST_END

    void hello(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void data(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
};
