



#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <nlohmann/json.hpp>   


std::string getAccessToken(const std::string& client_id, const std::string& client_secret);
std::string parseAccessToken(const std::string& jsonResponse);

#endif // AUTH_H







// #include <iostream>
// #include <cstdlib>  
// #include <thread>  
// #include <chrono>  
// #include <nlohmann/json.hpp>
// #include "order_management.h"
// #include "auth.h"
// #include "utils.h"
// #include <boost/asio.hpp>
// #include <memory>
// #include <unordered_map>
// #include <future>

// using namespace std;
// using json = nlohmann::json;

// // Asynchronous function to place a market order
// bool placeMarketOrderAsync(const string& accessToken, const string& instrument, double quantity, const string& label, string& orderId, MemoryPool& curlPool) {
//     return async(launch::async, [&]() {
//         return placeMarketOrder(accessToken, instrument, quantity, label, orderId, curlPool);
//     }).get();
// }

// // Asynchronous function to get order book
// json getOrderBookAsync(const string& symbol, MemoryPool& curlPool) {
//     return async(launch::async, [&]() {
//         return getOrderBook(symbol, curlPool);
//     }).get();
// }

// int main() {
//     const char* client_id = getenv("DERIBIT_CLIENT_ID");
//     const char* client_secret = getenv("DERIBIT_CLIENT_SECRET");

//     // Check for valid credentials
//     if (!client_id || !client_secret) {
//         cerr << "Client ID or secret is not set in environment variables." << endl;
//         return 1;
//     }

//     // Get access token
//     string accessToken = getAccessToken(client_id, client_secret);
//     if (accessToken.empty()) {
//         cerr << "Failed to retrieve access token." << endl;
//         return 1;
//     }

//     string instrument = "BTC-PERPETUAL";
//     double quantity = 40;
//     double price = 6000.0;
//     string orderId;
//     string label = "market0000234";
//     string symbol = "BTC-PERPETUAL";  
//     size_t poolSize = 10; // Number of CURL handles in the memory pool
//     MemoryPool curlPool(poolSize); // Create a MemoryPool instance

    

//     // Start measuring overall latency
//     auto overallStart = chrono::high_resolution_clock::now();

//     // Place a market order asynchronously
//     auto start = chrono::high_resolution_clock::now();
//     if (placeMarketOrderAsync(accessToken, instrument, quantity, label, orderId, curlPool)) {
//         cout << "Market order placed successfully." << endl;
//     } else {
//         cerr << "Failed to place market order." << endl;
//         return 1;
//     }
//     auto end = chrono::high_resolution_clock::now();
//     cout << "Market Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


//     // Place a limit order
//     start = chrono::high_resolution_clock::now();
//     if (placeOrder(accessToken, instrument, quantity, price, orderId, curlPool)) {
//         cout << "Limit order placed successfully. Order ID: " << orderId << endl;    
//     } else {    
//         cerr << "Failed to place order." << endl;    
//     }
//     end = chrono::high_resolution_clock::now();
//     cout << "Limit Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

//     //get order status 
//     json orderStatus = getOrderStatus(accessToken, orderId, curlPool);
//     if (!orderStatus.empty()) {
//         cout << "Order status: " << orderStatus.dump(4) << endl;
//     } else {
//         cerr << "Failed to get order status." << endl;
//         return 1;
//     }

//     // Modify the order
//     double newPrice = 66200.0;  
//     double newAmount = 80;  

//     start = chrono::high_resolution_clock::now();
//     if (modifyOrder(accessToken, orderId, newPrice, newAmount, curlPool)) {
//         cout << "Order modified successfully." << endl;
//     } else {
//         cerr << "Failed to modify order." << endl;
//     }
//     end = chrono::high_resolution_clock::now();
//     cout << "Order Modification Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

//     // Get order book data asynchronously
//     start = chrono::high_resolution_clock::now();
//     json orderBookData = getOrderBookAsync(symbol, curlPool);
//     if (!orderBookData.empty()) {
//         cout << "Order Book for " << symbol << ":\n";
//         cout << orderBookData.dump(4) << endl;  
//     }
//     end = chrono::high_resolution_clock::now();
//     cout << "Market Data Processing Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

//     // Cancel the order
//     start = chrono::high_resolution_clock::now();

//     if (cancelOrder(accessToken, orderId, curlPool)) {
//         cout << "Order canceled successfully." << endl;
//     } else {
//         cerr << "Failed to cancel order." << endl;
//     }

//     end = chrono::high_resolution_clock::now();
//     cout << "Cancel order Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

//     // Get current positions
//     json positions = getCurrentPositions(accessToken, curlPool);
//     cout << "Current Positions: " << positions.dump(4) << endl; 

//     // Overall latency
//     auto overallEnd = chrono::high_resolution_clock::now();
//     cout << "End-to-End Trading Loop Latency: " << chrono::duration_cast<chrono::milliseconds>(overallEnd - overallStart).count() << " ms" << endl;

//     return 0;
// }


// /* Calls: Market orders and order book fetching are now asynchronous, allowing these operations to run concurrently, thus reducing the total latency for multiple tasks.

// Thread Pool: By using std::async, you can manage network calls in parallel, avoiding waiting for each individual operation to finish before starting the next one.

// Latency Measurement: The latency for each operation is still being measured, but now you can compare the differences between synchronous and asynchronous execution.*/