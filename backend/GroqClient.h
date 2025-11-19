//GroqClient.h
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class GroqClient {
public:
    GroqClient();
    // Send prompt and return the textual response (raw). On error returns string starting with "ERROR:"
    std::string chat(const std::string& prompt, double timeout_seconds = 15.0);

private:
    std::string apiKey_;
    std::string baseUrl_;
    std::string apiPath_;
    std::string personaChunk_;
    std::string modelName_;
};