#ifndef ORDER_MANAGEMENT_H
#define ORDER_MANAGEMENT_H

#include <string>
#include <nlohmann/json.hpp>  // Include this for nlohmann::json

bool cancelOrder(const std::string& accessToken, const std::string& orderId);
bool modifyOrder(const std::string& accessToken, const std::string& orderId, double newPrice, double amount);
bool placeOrder(const std::string& accessToken, const std::string& instrument, double quantity, double price, std::string& orderId);
bool placeMarketOrder(const std::string& accessToken, const std::string& instrument, double quantity, std::string label, std::string& orderId);
nlohmann::json getOrderBook(const std::string& symbol); 
nlohmann::json getCurrentPositions(const std::string& accessToken);
nlohmann::json getOrderStatus(const std::string& accessToken, const std::string& orderId);

#endif // ORDER_MANAGEMENT_H

























// #ifndef ORDER_MANAGEMENT_H
// #define ORDER_MANAGEMENT_H

// #include <string>
// #include <nlohmann/json.hpp>  // Include this for nlohmann::json


// bool cancelOrder(const std::string& accessToken, const std::string& orderId);
// bool modifyOrder(const std::string& accessToken, const std::string& orderId, double newPrice, double amount);
// bool placeOrder(const std::string& accessToken, const std::string& instrument, double quantity, double price, std::string& orderId);
// bool placeMarketOrder(const std::string& accessToken, const std::string& instrument, double quantity, std::string label, std::string& orderId);
// nlohmann::json getOrderBook(const std::string& symbol); 
// nlohmann::json getCurrentPositions(const std::string& accessToken);

// #endif // ORDER_MANAGEMENT_H
