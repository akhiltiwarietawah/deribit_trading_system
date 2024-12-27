#include "token_manager.h"
#include "CurlConnectionManager.h"

using namespace std;

std::string TokenManager::getAccessToken() {
    std::lock_guard<std::mutex> lock(token_mutex);

    if (std::chrono::steady_clock::now() > expiry_time || access_token.empty()) {
        access_token = fetchAccessToken();
        expiry_time = std::chrono::steady_clock::now() + std::chrono::minutes(59);
    }

    return access_token;
}

std::string TokenManager::fetchAccessToken() {
    CURL* curl = curl_easy_init(); // Initialize CURL
    if (!curl) {
        cerr << "Failed to initialize CURL" << endl;
        return "";
    }

    string readBuffer;
    string url = "https://test.deribit.com/api/v2/public/auth?client_id=" + client_id + "&client_secret=" + client_secret + "&grant_type=client_credentials";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "CURL request failed: " << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl); // Cleanup CURL handle
        return "";
    }

    curl_easy_cleanup(curl); // Cleanup CURL handle
    return parseAccessToken(readBuffer);
}

std::string TokenManager::parseAccessToken(const std::string& jsonResponse) {
    auto json = nlohmann::json::parse(jsonResponse);
    if (json.contains("result")) {
        return json["result"]["access_token"];
    }
    return "";
}

size_t TokenManager::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
