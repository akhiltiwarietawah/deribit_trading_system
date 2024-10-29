#include "auth.h"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "utils.h"

using namespace std;

string getAccessToken(const string& client_id, const string& client_secret) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        string url = "https://test.deribit.com/api/v2/public/auth?client_id=" +
                     client_id + "&client_secret=" + client_secret +
                     "&grant_type=client_credentials";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return parseAccessToken(readBuffer);
}

string parseAccessToken(const string& jsonResponse) {
    auto json = nlohmann::json::parse(jsonResponse);
    if (json.contains("result")) {
        return json["result"]["access_token"];
    }
    return "";
}
