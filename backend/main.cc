#include <drogon/drogon.h>


int main()
{
    auto &app = drogon::app();

    // Listen on all interfaces, port 8080
    app.addListener("0.0.0.0", 8080);

    // ------------------- GET /api/hello -------------------
    app.registerHandler("/api/hello",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            Json::Value ret;
            ret["message"] = "Hello from Drogon backend!";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");

            callback(resp);
        },
        {drogon::Get, drogon::Options});
        

    // ------------------- POST /api/data -------------------
    app.registerHandler("/api/data",
        [](const drogon::HttpRequestPtr &req,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            if (req->method() == drogon::Options)
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
                callback(resp);
                return;
            }

            auto json = req->getJsonObject();
            std::string name = (*json)["name"].asString();
            std::cout << "Received name: " << name << std::endl;

            Json::Value ret;
            ret["message"] = "Hello " + name + "! Data received!";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");

            callback(resp);
        },
        {drogon::Post, drogon::Options});

    // ------------------- Initialize singletons -------------------
    // ------------------- Run the server -------------------
    app.run();
}
