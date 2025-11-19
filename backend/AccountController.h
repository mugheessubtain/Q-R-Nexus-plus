#pragma once

#include <drogon/HttpController.h>
#include <drogon/Session.h>
#include <mutex> // Required for thread safety with file access
#include <string>

using namespace drogon;

class AccountController : public drogon::HttpController<AccountController>
{
public:
  static constexpr const char* path() { return "/api/v1/Account"; }

    METHOD_LIST_BEGIN
    // CORS is now handled globally in main.cpp, only AuthFilter remains
    ADD_METHOD_TO(AccountController::login, "/auth/login", Post);
    ADD_METHOD_TO(AccountController::signup, "/auth/signup", Post);
    ADD_METHOD_TO(AccountController::logout, "/auth/logout", Post);
    ADD_METHOD_TO(AccountController::checkSession, "/auth/check", Get);

    METHOD_LIST_END

    void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void signup(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void logout(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void checkSession(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);

private:
    // Helper function to load all users from users.json
    Json::Value loadUsersFromFile();

    // Helper function to save all users to users.json
    bool saveUsersToFile(const Json::Value& users);

    // Mutex to protect concurrent access to the users.json file 
    std::mutex file_mutex_; 

    // The name of the file used for user storage
    const std::string USERS_FILE_NAME = "users.json";
};