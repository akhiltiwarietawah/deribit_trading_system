#include "order_management.h"
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "utils.h"

using namespace std;
using json = nlohmann::json;

bool modifyOrder(const string& accessToken, const string& orderId, double newPrice, double amount) {
    CURL* curl = curl_easy_init();  
    string readBuffer;

    if (curl) {
        if (amount <= 0) {
            cerr << "Invalid amount value." << endl;
            return false;    
        }

        string url = "https://test.deribit.com/api/v2/private/edit?order_id=" + orderId +
                      "&price=" + to_string(newPrice) + "&amount=" + to_string(amount);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return false;
        }

        cout << "Response: " << readBuffer << endl; // Log the full response

        auto jsonResponse = json::parse(readBuffer);
        if (jsonResponse.contains("error")) {
            cerr << "Failed to modify order. Error: " << jsonResponse["error"]["message"] << endl;
            return false;
        } else {
            cout << "Order modified successfully." << endl;
            return true;
        }
    }
    return false;
}

bool cancelOrder(const string& accessToken, const string& orderId) {
    CURL* curl = curl_easy_init(); 
    string readBuffer;

    if (curl) {
        string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + orderId;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return false;
        }

        auto jsonResponse = json::parse(readBuffer);
        if (jsonResponse.contains("error")) {
            cerr << "Failed to cancel order. Error: " << jsonResponse["error"]["message"] << endl;
            return false;
        } else {
            cout << "Order canceled successfully." << endl;
            return true;
        }
    }
    return false;
}

bool placeMarketOrder(const string& accessToken, const string& instrument, double quantity, string label, string& orderId) {
    CURL* curl = curl_easy_init();  
    string readBuffer;

    if (curl) {
        string url = "https://test.deribit.com/api/v2/private/buy?amount=" + to_string(quantity) +
                      "&instrument_name=" + instrument +
                      "&label=" + label +
                      "&type=market"; 

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            return false;
        }

        auto jsonResponse = json::parse(readBuffer);
        cout << "API Response: " << jsonResponse.dump(4) << endl;

        if (jsonResponse.contains("result")) {
            auto result = jsonResponse["result"];
            orderId = result["order"]["order_id"];
            if (result.contains("order") && result["order"]["order_state"] == "filled") {
                cout << "Market order placed and filled successfully. Order ID: " << orderId << endl;
                return true;
            } else {
                cout << "Order not filled: " << result.dump(4) << endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return false;
}

bool placeOrder(const string& accessToken, const string& instrument, double quantity, double price, string& orderId) {
    CURL* curl = curl_easy_init(); // Initialize cURL
    string readBuffer;

    if (curl) {
        string url = "https://test.deribit.com/api/v2/private/buy?amount=" + to_string(quantity) +
                      "&instrument_name=" + instrument +
                      "&label=limit_order_123&type=limit&price=" + to_string(price);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return false;
        }

        auto jsonResponse = json::parse(readBuffer);
        if (jsonResponse.contains("result")) {
            string orderState = jsonResponse["result"]["order"]["order_state"];
            orderId = jsonResponse["result"]["order"]["order_id"];

            if (orderState == "open" || orderState == "filled") {
                cout << "Limit order placed successfully. Order ID: " << orderId
                     << " | Order State: " << orderState << endl;
                return true;
            }
        }
        cerr << "Failed to place limit order. Response: " << readBuffer << endl;
        return false;
    }
    return false;
}

json getOrderBook(const string& symbol) {
    string apiUrl = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
    CURL* curl = curl_easy_init(); // Initialize cURL
    string readBuffer;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Timeout for low latency

        CURLcode res = curl_easy_perform(curl); // Perform the request
        if (res != CURLE_OK) {
            cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            return {};
        }
        curl_easy_cleanup(curl);
    }

    json orderBook = json::parse(readBuffer, nullptr, false); // Parse the JSON response
    if (orderBook.is_discarded() || orderBook["result"].is_null()) {
        cerr << "Failed to fetch order book data." << endl;
        return {};
    }

    return orderBook["result"];
}

// Function to view current positions
json getCurrentPositions(const string& accessToken) {
    CURL* curl = curl_easy_init(); // Initialize cURL
    string readBuffer;

    if (curl) {
        string url = "https://test.deribit.com/api/v2/private/get_positions?currency=BTC&kind=future";
        
        cout << "URL: " << url << endl;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return {};
        }

        auto jsonResponse = json::parse(readBuffer);
        if (jsonResponse.contains("error")) {
            cerr << "Error fetching positions: " << jsonResponse["error"]["message"] << endl;
            return {};
        }
        return jsonResponse["result"];
    }
    return {};
}

//function to view order status

json getOrderStatus(const string& accessToken, const string& orderId) {
    CURL* curl = curl_easy_init(); // Initialize cURL
    string readBuffer;

    if (curl) {
        string url = "https://test.deribit.com/api/v2/private/get_order_state?order_id=" + orderId;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return {};
        }

        auto jsonResponse = json::parse(readBuffer);
        if (jsonResponse.contains("result")) {
            string orderState = jsonResponse["result"]["order_state"];
            cout << "Order State: " << orderState << endl;

            return jsonResponse["result"];
        } else if (jsonResponse.contains("error")) {
            cerr << "Error fetching order status: " << jsonResponse["error"]["message"] << endl;
        } else {
            cerr << "Unexpected response structure." << endl;
        }
    } else {
        cerr << "Failed to initialize cURL." << endl;
    }

    return {};
}
