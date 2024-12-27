#ifndef ORDER_MANAGEMENT_H
#define ORDER_MANAGEMENT_H

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "CurlConnectionManager.h"
#include "token_manager.h"
#include "utils.h"

class OrderAPI {
public:
    OrderAPI(CurlConnectionManager& connectionManager, size_t numThreads, TokenManager& tokenManager);

    bool placeMarketOrder(const std::string& instrument, double quantity, const std::string& label, std::string& orderId);
    bool placeLimitOrder(const std::string& instrument, double quantity, double price, std::string& orderId);
    nlohmann::json getOrderBook(const std::string& symbol);
    bool cancelOrder(const std::string& orderId);
    bool modifyOrder(const std::string& orderId, double newPrice, double amount);

private:
    CurlConnectionManager& curlManager;
    ThreadPool threadPool;
    TokenManager& tokenManager;
    MemoryPool orderPool;
    std::unordered_map<std::string, nlohmann::json> orders;

    bool performCurlRequest(const std::string& url, const std::string& accessToken, std::string& response);
    bool batchNetworkRequests(const std::vector<std::string>& urls, const std::string& accessToken, std::vector<std::string>& responses);
    bool performAsyncCurlRequest(const std::string& url, const std::string& accessToken, std::string& response);
    bool performBatchAsyncCurlRequest(const std::vector<std::string>& urls, const std::string& accessToken, std::vector<std::string>& responses);

    std::string decompressGzip(const std::string& str);
    nlohmann::json parseMarketData(const std::string& jsonData);

    nlohmann::json getOrderBookData(const std::string& symbol);
    bool cancelOrderRequest(const std::string& orderId);

    bool placeOrder(const std::string& instrument, double quantity, double price, const std::string& label, std::string& orderId, const std::string& orderType);
};

#endif // ORDER_MANAGEMENT_H
