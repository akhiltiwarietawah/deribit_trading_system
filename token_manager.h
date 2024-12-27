#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include <string>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <mutex>

class TokenManager {
public:
    TokenManager(const std::string& client_id, const std::string& client_secret)
        : client_id(client_id), client_secret(client_secret) {}

    std::string getAccessToken();

private:
    std::string client_id;
    std::string client_secret;
    std::string access_token;
    std::chrono::steady_clock::time_point expiry_time;
    std::mutex token_mutex;

    std::string fetchAccessToken();
    std::string parseAccessToken(const std::string& jsonResponse);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
};

#endif // TOKEN_MANAGER_H
