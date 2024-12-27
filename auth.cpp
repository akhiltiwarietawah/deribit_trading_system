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



// bool modifyOrder(const string& accessToken, const string& orderId, double newPrice, double amount, MemoryPool& pool) {
//     CURL* curl = pool.allocate();  // Allocate CURL handle from the memory pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle from pool." << endl;
//         return false;
//     }

//     string readBuffer;  // Buffer to store the response

//     if (amount <= 0) {
//         cerr << "Invalid amount value." << endl;
//         pool.deallocate(curl);  // Return CURL handle to the pool
//         return false;
//     }

//     string url = "https://test.deribit.com/api/v2/private/edit?order_id=" + orderId +
//                  "&price=" + to_string(newPrice) + "&amount=" + to_string(amount);

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         curl_slist_free_all(headers);
//         pool.deallocate(curl);  // Return CURL handle to the pool
//         return false;
//     }

//     cout << "Response: " << readBuffer << endl;

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("error")) {
//         cerr << "Failed to modify order. Error: " << jsonResponse["error"]["message"] << endl;
//         curl_slist_free_all(headers);
//         pool.deallocate(curl);  // Return CURL handle to the pool
//         return false;
//     } else {
//         cout << "Order modified successfully." << endl;
//         curl_slist_free_all(headers);
//         pool.deallocate(curl);  // Return CURL handle to the pool
//         return true;
//     }
// }

// bool cancelOrder(const string& accessToken, const string& orderId, MemoryPool& pool) {
//     CURL* curl = pool.allocate();  // Allocate CURL handle from the memory pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle from pool." << endl;
//         return false;
//     }
//     string readBuffer;

//     string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + orderId;
//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     pool.deallocate(curl);  // Return CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         return false;
//     }

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("error")) {
//         cerr << "Failed to cancel order. Error: " << jsonResponse["error"]["message"] << endl;
//         return false;
//     } else {
//         cout << "Order canceled successfully." << endl;
//         return true;
//     }
// }

// bool placeMarketOrder(const string& accessToken, const string& instrument, double quantity, const string& label, string& orderId, MemoryPool& curlPool) {
//     CURL* curl = curlPool.allocate();  // Allocate CURL handle from pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle" << endl;
//         return false;
//     }

//     string readBuffer;
//     string url = "https://test.deribit.com/api/v2/private/buy?amount=" + to_string(quantity)
//               + "&instrument_name=" + instrument
//               + "&label=" + label
//               + "&type=market";

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     curlPool.deallocate(curl);  // Return the CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         curlPool.deallocate(curl); // Deallocate after curl perform
//         curl_slist_free_all(headers);
//         return false;
//     }

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("result") && jsonResponse["result"].contains("order")) {
//         orderId = jsonResponse["result"]["order"]["order_id"];
//         if (jsonResponse["result"]["order"]["order_state"] == "filled") {
//             cout << "Market order placed and filled successfully. Order ID: " << orderId << endl;
//             return true;
//         }
//     }
//     cerr << "Failed to place market order. Response: " << readBuffer << endl;
//     return false;
// }

// bool placeOrder(const string& accessToken, const string& instrument, double quantity, double price, string& orderId, MemoryPool& curlPool) {
//     CURL* curl = curlPool.allocate();  // Allocate CURL handle from pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle" << endl;
//         return false;
//     }

//     string readBuffer;
//     string url = "https://test.deribit.com/api/v2/private/buy?amount=" + to_string(quantity)
//               + "&instrument_name=" + instrument
//               + "&label=limit_order_123&type=limit&price=" + to_string(price);

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     curlPool.deallocate(curl);  // Return the CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         curlPool.deallocate(curl); // Deallocate after curl perform
//         curl_slist_free_all(headers);
//         return false;
//     }

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("result") && jsonResponse["result"].contains("order")) {
//         string orderState = jsonResponse["result"]["order"]["order_state"];
//         orderId = jsonResponse["result"]["order"]["order_id"];
//         if (orderState == "open" || orderState == "filled") {
//             cout << "Limit order placed successfully. Order ID: " << orderId
//                  << " | Order State: " << orderState << endl;
//             return true;
//         }
//     }
//     cerr << "Failed to place limit order. Response: " << readBuffer << endl;
//     return false;
// }

// json getOrderBook(const string& symbol, MemoryPool& curlPool) {
//     CURL* curl = curlPool.allocate();  // Get a CURL handle from the pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle" << endl;
//         return {};
//     }

//     string readBuffer;
//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     string apiUrl = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
    

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     curlPool.deallocate(curl);  // Return the CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
//         return {};
//     }

//     auto orderBook = json::parse(readBuffer, nullptr, false);
//     if (orderBook.is_discarded() || orderBook["result"].is_null()) {
//         cerr << "Failed to fetch order book data." << endl;
//         return {};
//     }

//     return orderBook["result"];
// }

// json getCurrentPositions(const string& accessToken, MemoryPool& pool) {
//     CURL* curl = pool.allocate();  // Allocate CURL handle from the memory pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle from pool." << endl;
//         return {};
//     }
//     string readBuffer;

//     string url = "https://test.deribit.com/api/v2/private/get_positions?currency=BTC&kind=future";
//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     pool.deallocate(curl);  // Return CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         return {};
//     }

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("error")) {
//         cerr << "Error fetching positions: " << jsonResponse["error"]["message"] << endl;
//         return {};
//     }
//     return jsonResponse["result"];
// }
// json getOrderStatus(const string& accessToken, const string& orderId, MemoryPool& curlPool) {
//     CURL* curl = curlPool.allocate();  // Get a CURL handle from the pool
//     if (!curl) {
//         cerr << "Failed to allocate CURL handle" << endl;
//         return {};
//     }

//     string readBuffer;
//     string url = "https://test.deribit.com/api/v2/private/get_order_state?order_id=" + orderId;

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

//     CURLcode res = curl_easy_perform(curl);
//     curlPool.deallocate(curl);  // Return CURL handle to the pool
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK) {
//         cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         return {};
//     }

//     auto jsonResponse = json::parse(readBuffer);
//     if (jsonResponse.contains("error")) {
//         cerr << "Error getting order status: " << jsonResponse["error"]["message"] << endl;
//         return {};
//     }
//     return jsonResponse["result"];
// }











// MemoryPool implementation
// MemoryPool::MemoryPool(size_t blockCount) : blockCount(blockCount) {
//     pool = new CURL*[blockCount];
//     freeBlocks.reserve(blockCount);

//     for (size_t i = 0; i < blockCount; ++i) {
//         pool[i] = curl_easy_init();
//         if (!pool[i]) {
//             cerr << "Failed to initialize CURL handle!" << endl;
//         }
//         freeBlocks.push_back(pool[i]);
//     }
// }

// MemoryPool::~MemoryPool() {
//     for (size_t i = 0; i < blockCount; ++i) {
//         if (pool[i]) {
//             curl_easy_cleanup(pool[i]);
//         }
//     }
//     delete[] pool;
// }

// CURL* MemoryPool::allocate() {
//     if (freeBlocks.empty()) {
//         cerr << "Memory pool exhausted" << endl;
//         return nullptr;
//     }
//     CURL* handle = freeBlocks.back();
//     freeBlocks.pop_back();
//     return handle;
// }

// void MemoryPool::deallocate(CURL* handle) {
//     if (handle) {
//         auto it = find(freeBlocks.begin(), freeBlocks.end(), handle);
//         if (it != freeBlocks.end()) {
//             cerr << "Error: Attempting to deallocate the same CURL handle twice!" << endl;
//             return;
//         }
//         freeBlocks.push_back(handle);
//     } else {
//         cerr << "Error: Attempted to deallocate a null CURL handle!" << endl;
//     }
// }
