#include "order_management.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <future>
#include "token_manager.h"
#include "utils.h"
#include "CurlConnectionManager.h"
#include <immintrin.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <zlib.h>

using namespace std;
using json = nlohmann::json;

OrderAPI::OrderAPI(CurlConnectionManager& connectionManager, size_t numThreads, TokenManager& tokenManager)
    : curlManager(connectionManager), threadPool(numThreads), tokenManager(tokenManager), orderPool(sizeof(nlohmann::json), 100) {
    orders.reserve(100);
}

bool OrderAPI::performCurlRequest(const std::string& url, const std::string& accessToken, std::string& response) {
    CURL* curl = curlManager.getCurlHandle();
    if (!curl) return false;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    const int maxRetries = 3;
    int retries = 0;
    CURLcode res;
    do {
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) break;
        retries++;
    } while (retries < maxRetries);

    curl_slist_free_all(headers);
    curlManager.releaseCurlHandle(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL request failed after retries: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    return true;
}

bool OrderAPI::batchNetworkRequests(const std::vector<std::string>& urls, const std::string& accessToken, std::vector<std::string>& responses) {
    CURLM* multi_handle = curl_multi_init();
    std::vector<CURL*> handles(urls.size());
    std::vector<curl_slist*> headers(urls.size());

    for (size_t i = 0; i < urls.size(); ++i) {
        handles[i] = curl_easy_init();
        headers[i] = curl_slist_append(nullptr, ("Authorization: Bearer " + accessToken).c_str());
        headers[i] = curl_slist_append(headers[i], "Content-Type: application/json");

        curl_easy_setopt(handles[i], CURLOPT_URL, urls[i].c_str());
        curl_easy_setopt(handles[i], CURLOPT_HTTPHEADER, headers[i]);
        curl_easy_setopt(handles[i], CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(handles[i], CURLOPT_WRITEDATA, &responses[i]);

        curl_multi_add_handle(multi_handle, handles[i]);
    }

    int still_running;
    do {
        CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK) {
            std::cerr << "curl_multi_perform() failed: " << curl_multi_strerror(mc) << std::endl;
            break;
        }
    } while (still_running);

    for (CURL* handle : handles) {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }
    curl_multi_cleanup(multi_handle);

    for (curl_slist* header : headers) {
        curl_slist_free_all(header);
    }

    return true;
}

bool OrderAPI::performAsyncCurlRequest(const std::string& url, const std::string& accessToken, std::string& response) {
    auto task = std::async(std::launch::async, [&]() {
        return performCurlRequest(url, accessToken, response);
    });
    return task.get();
}

bool OrderAPI::performBatchAsyncCurlRequest(const std::vector<std::string>& urls, const std::string& accessToken, std::vector<std::string>& responses) {
    std::vector<std::future<bool>> futures;
    for (const auto& url : urls) {
        futures.push_back(std::async(std::launch::async, [&]() {
            std::string response;
            bool result = performCurlRequest(url, accessToken, response);
            responses.push_back(std::move(response));
            return result;
        }));
    }
    bool allSuccessful = true;
    for (auto& future : futures) {
        allSuccessful &= future.get();
    }
    return allSuccessful;
}

std::string OrderAPI::decompressGzip(const std::string& str) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK)
        throw std::runtime_error("inflateInit2 failed");

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("decompression failed");
    }

    return outstring;
}

nlohmann::json OrderAPI::parseMarketData(const std::string& jsonData) {
    rapidjson::Document document;
    if (document.Parse(jsonData.c_str()).HasParseError()) {
        std::cerr << "Error parsing JSON data." << std::endl;
        return {};
    }
    if (!document.IsObject() || !document.HasMember("result")) {
        std::cerr << "Invalid JSON structure." << std::endl;
        return {};
    }
    return rapidjsonToNlohmannJson(document["result"]);
}

bool OrderAPI::placeOrder(const std::string& instrument, double quantity, double price, const std::string& label, std::string& orderId,     const std::string& orderType) {
    std::string readBuffer;
    std::string url = "https://test.deribit.com/api/v2/private/buy?amount=" + to_string(quantity)
                    + "&instrument_name=" + instrument
                    + "&label=" + label
                    + "&type=" + orderType;

    if (orderType == "limit") {
        url += "&price=" + to_string(price);
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool success = performAsyncCurlRequest(url, tokenManager.getAccessToken(), readBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Order Placement Request Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    if (!success) {
        return false;
    }

    // Extract body from the response
    std::string body = readBuffer.substr(readBuffer.find("\r\n\r\n") + 4);

    // Decompress response if it's gzipped
    if (body.size() > 2 && body[0] == '\x1f' && body[1] == '\x8b') {
        try {
            body = decompressGzip(body);
        } catch (const std::runtime_error& e) {
            std::cerr << "Failed to decompress response: " << e.what() << std::endl;
            return false;
        }
    }

    auto parseStart = std::chrono::high_resolution_clock::now();
    rapidjson::Document document;
    success = parseJSONResponse(body, document);
    auto parseEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Order Placement Parsing Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseStart).count() << " ms" << std::endl;

    if (success && document.IsObject() && document.HasMember("result") && document["result"].HasMember("order")) {
        if (document["result"]["order"].HasMember("order_id")) {
            orderId = document["result"]["order"]["order_id"].GetString();
        } else {
            std::cerr << "Missing 'order_id' field in order object" << std::endl;
            return false;
        }
        void* orderPtr = orderPool.allocate();
        if (orderPtr) {
            nlohmann::json& order = *new (orderPtr) nlohmann::json(rapidjsonToNlohmannJson(document["result"]["order"]));
            orders[orderId] = order;
        }

        return true;
    } else if (document.HasMember("error")) {
        std::cerr << "Error placing order: " << document["error"]["message"].GetString() << std::endl;
        return false;
    }

    std::cerr << "Failed to place order. Full Response Body: " << body << std::endl;
    return false;
}

bool OrderAPI::placeMarketOrder(const std::string& instrument, double quantity, const std::string& label, std::string& orderId) {
    return placeOrder(instrument, quantity, 0, label, orderId, "market");
}

bool OrderAPI::placeLimitOrder(const std::string& instrument, double quantity, double price, std::string& orderId) {
    return placeOrder(instrument, quantity, price, "limit_order", orderId, "limit");
}

nlohmann::json OrderAPI::getOrderBookData(const std::string& symbol) {
    std::string readBuffer;
    std::string apiUrl = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;

    auto start = std::chrono::high_resolution_clock::now();
    bool success = performAsyncCurlRequest(apiUrl, "", readBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Market Data Fetch Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    if (!success) {
        return {};
    }

    // Strip headers if present
    size_t headerEndPos = readBuffer.find("\r\n\r\n");
    if (headerEndPos != std::string::npos) {
        readBuffer = readBuffer.substr(headerEndPos + 4);
    }

    // Decompress response if it's gzipped
    if (readBuffer.size() > 2 && readBuffer[0] == '\x1f' && readBuffer[1] == '\x8b') {
        try {
            readBuffer = decompressGzip(readBuffer);
        } catch (const std::runtime_error& e) {
            std::cerr << "Failed to decompress response: " << e.what() << std::endl;
            return {};
        }
    }

    auto parseStart = std::chrono::high_resolution_clock::now();
    rapidjson::Document document;
    success = parseJSONResponse(readBuffer, document);
    auto parseEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Market Data Parsing Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseStart).count() << " ms" << std::endl;

    if (!success || !document.IsObject() || !document.HasMember("result")) {
        std::cerr << "Failed to parse market data response. Full Response: " << readBuffer << std::endl;
        return {};
    }

    return rapidjsonToNlohmannJson(document);
}

nlohmann::json OrderAPI::getOrderBook(const std::string& symbol) {
    return executeAsyncTask<nlohmann::json>([&]() {
        return getOrderBookData(symbol);
    });
}

bool OrderAPI::cancelOrderRequest(const std::string& orderId) {
    std::string readBuffer;
    std::string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + orderId;

    auto start = std::chrono::high_resolution_clock::now();
    bool success = performAsyncCurlRequest(url, tokenManager.getAccessToken(), readBuffer);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Order Cancellation Request Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

    if (!success) {
        return false;
    }

    // Extract body from the response
    std::string body = readBuffer.substr(readBuffer.find("\r\n\r\n") + 4);

    // Decompress response if it's gzipped
    if (body.size() > 2 && body[0] == '\x1f' && body[1] == '\x8b') {
        try {
            body = decompressGzip(body);
        } catch (const std::runtime_error& e) {
            std::cerr << "Failed to decompress response: " << e.what() << std::endl;
            return false;
        }
    }

    auto parseStart = std::chrono::high_resolution_clock::now();
    rapidjson::Document document;
    success = parseJSONResponse(body, document);
    auto parseEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Order Cancellation Parsing Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseStart).count() << " ms" << std::endl;

    if (!success || !document.IsObject() || document.HasMember("error")) {
        std::cerr << "Failed to cancel order. Error: " << (document.HasMember("error") ? document["error"]["message"].GetString() : "Unknown") << std::endl;
        return false;
    }

    if (orders.find(orderId) != orders.end()) {
        orderPool.deallocate(static_cast<void*>(&orders[orderId]));
        orders.erase(orderId);
    }

    return true;
}

bool OrderAPI::cancelOrder(const std::string& orderId) {
    return executeAsyncTask<bool>([&]() {
        return cancelOrderRequest(orderId);
    });
}

bool OrderAPI::modifyOrder(const std::string& orderId, double newPrice, double amount) {
    auto task = [&]() -> bool {
        std::string readBuffer;
        std::string url = "https://test.deribit.com/api/v2/private/edit?order_id=" + orderId
                    + "&price=" + to_string(newPrice)
                    + "&amount=" + to_string(amount);

        auto start = std::chrono::high_resolution_clock::now();
        bool success = performCurlRequest(url, tokenManager.getAccessToken(), readBuffer);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Modify Order Request Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

        if (!success) {
            return false;
        }

        // Extract body from the response
        std::string body = readBuffer.substr(readBuffer.find("\r\n\r\n") + 4);

        // Decompress response if it's gzipped
        if (body.size() > 2 && body[0] == '\x1f' && body[1] == '\x8b') {
            try {
                body = decompressGzip(body);
            } catch (const std::runtime_error& e) {
        std::cerr << "Failed to decompress response: " << e.what() << std::endl;
                return false;
            }
        }

        auto parseStart = std::chrono::high_resolution_clock::now();
        rapidjson::Document document;
        success = parseJSONResponse(body, document);
        auto parseEnd = std::chrono::high_resolution_clock::now();
        std::cout << "Modify Order Parsing Latency: " << std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseStart).count() << " ms" << std::endl;

        if (!success || !document.IsObject() || document.HasMember("error")) {
            std::cerr << "Failed to modify order. Error: " << (document.HasMember("error") ? document["error"]["message"].GetString() : "Unknown") << std::endl;
            return false;
        }

        if (!document["result"].HasMember("order")) {
            std::cerr << "Failed to get modified order data. Response: " << body << std::endl;
            return false;
        }

        void* orderPtr = orderPool.allocate();
        if (orderPtr) {
            nlohmann::json& order = *new (orderPtr) nlohmann::json(rapidjsonToNlohmannJson(document["result"]["order"]));
            orders[orderId] = order;
        }

        return true;
    };
    return executeAsyncTask<bool>(task);
}

