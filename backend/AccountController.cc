#include "AccountController.h"
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/Session.h>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace drogon;

// ===== FILE I/O HELPERS =====

Json::Value AccountController::loadUsersFromFile()
{
    std::lock_guard<std::mutex> lock(file_mutex_);

    std::ifstream file(USERS_FILE_NAME);
    Json::Value users(Json::arrayValue);

    if (file.is_open())
    {
        try
        {
            Json::Reader reader;
            std::string content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

            if (!content.empty() && reader.parse(content, users) && users.isArray())
            {
                LOG_DEBUG << "Loaded " << users.size() << " users";
            }
            else
            {
                LOG_WARN << "Invalid or empty file, resetting.";
                users = Json::Value(Json::arrayValue);
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << "Parse error: " << e.what();
        }
    }
    else
    {
        LOG_INFO << USERS_FILE_NAME << " not found";
    }

    return users;
}

bool AccountController::saveUsersToFile(const Json::Value &users)
{
    std::lock_guard<std::mutex> lock(file_mutex_);

    std::ofstream file(USERS_FILE_NAME);
    if (!file.is_open()) return false;

    try
    {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "    ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(users, &file);
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Write error: " << e.what();
        return false;
    }
}

// ===== CONTROLLER METHODS =====

void AccountController::login(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_APPLICATION_JSON);

    Json::Value reqJson;
    Json::Reader reader;
    std::string body = std::string(req->body());

    if (body.empty() || !reader.parse(body, reqJson))
    {
        resp->setStatusCode(k400BadRequest);
        resp->setBody("{\"message\":\"Invalid JSON or empty body\"}");
        callback(resp);
        return;
    }

    if (!reqJson.isMember("username") || !reqJson.isMember("password"))
    {
        resp->setStatusCode(k400BadRequest);
        resp->setBody("{\"message\":\"Missing username or password\"}");
        callback(resp);
        return;
    }

    std::string username = reqJson["username"].asString();
    std::string password = reqJson["password"].asString();

    Json::Value users = loadUsersFromFile();
    bool authenticated = false;

    for (const auto &user : users)
    {
        if (user.isObject() &&
            user["username"].asString() == username &&
            user["password"].asString() == password)
        {
            authenticated = true;
            break;
        }
    }

    if (!authenticated)
    {
        resp->setStatusCode(k401Unauthorized);
        resp->setBody("{\"message\":\"Invalid credentials\"}");
        callback(resp);
        return;
    }

    auto session = req->session();
    if (!session)
    {
        resp->setStatusCode(k500InternalServerError);
        resp->setBody("{\"message\":\"Session unavailable\"}");
        callback(resp);
        return;
    }

    session->insert("user_id", username);
    session->insert("is_logged_in", true);

    Json::Value respJson;
    respJson["message"] = "Login successful";
    respJson["username"] = username;

    resp->setBody(respJson.toStyledString());
    callback(resp);
}


void AccountController::signup(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_APPLICATION_JSON);

    Json::Value reqJson;
    Json::Reader reader;

    std::string body = std::string(req->body());   // ✔ FIX
    if (body.empty() || !reader.parse(body, reqJson))
    {
        resp->setStatusCode(k400BadRequest);
        resp->setBody("{\"message\":\"Invalid JSON or empty body\"}");
        callback(resp);
        return;
    }

    if (reqJson["username"].empty() || reqJson["password"].empty() || reqJson["email"].empty())
    {
        resp->setStatusCode(k400BadRequest);
        resp->setBody("{\"message\":\"Missing username, password, or email\"}");
        callback(resp);
        return;
    }

    std::string username = reqJson["username"].asString();
    std::string password = reqJson["password"].asString();
    std::string email = reqJson["email"].asString();

    Json::Value users = loadUsersFromFile();
    for (const auto &user : users)
    {
        if (user["username"].asString() == username)
        {
            resp->setStatusCode(k409Conflict);
            resp->setBody("{\"message\":\"User already exists\"}");
            callback(resp);
            return;
        }
    }

    Json::Value newUser;
    newUser["username"] = username;
    newUser["password"] = password;
    newUser["email"] = email;
    users.append(newUser);

    if (!saveUsersToFile(users))
    {
        resp->setStatusCode(k500InternalServerError);
        resp->setBody("{\"message\":\"Unable to save user data\"}");
        callback(resp);
        return;
    }

    resp->setBody("{\"message\":\"Signup successful. Please log in.\"}");
    callback(resp);
}

void AccountController::logout(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    req->session()->clear();

    Json::Value respJson;
    respJson["message"] = "Logout successful";

    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}

void AccountController::checkSession(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto session = req->session();

    Json::Value respJson;
    respJson["isLoggedIn"] = true;
    respJson["username"] = session->get<std::string>("user_id");
    respJson["message"] = "Session active";

    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}
