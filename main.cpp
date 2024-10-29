#include <iostream>
#include <cstdlib>  
#include <thread>  
#include <chrono>  
#include <nlohmann/json.hpp>
#include "order_management.h"
#include "auth.h"
#include <boost/asio.hpp>

using namespace std;
using json = nlohmann::json;

int main() {
     
    const char* client_id = getenv("DERIBIT_CLIENT_ID");
    const char* client_secret = getenv("DERIBIT_CLIENT_SECRET");

    // Check for valid credentials
    if (!client_id || !client_secret) {
        cerr << "Client ID or secret is not set in environment variables." << endl;
        return 1;
    }

    // Get access token
    string accessToken = getAccessToken(client_id, client_secret);
    if (accessToken.empty()) {
        cerr << "Failed to retrieve access token." << endl;
        return 1;
    }

    string instrument = "BTC-PERPETUAL";
    double quantity = 40;
    double price = 6000.0;
    string orderId;

    // Place a market order
    // string label = "market0000234";
    // if (placeMarketOrder(accessToken, instrument, quantity, label, orderId)) {
    //     cout << "Market order placed successfully." << endl;
    // } else {
    //     cerr << "Failed to place market order." << endl;
    //     return 1;
    // }

    // Place a limit order
    // if (placeOrder(accessToken, instrument, quantity, price, orderId)) {
    //     cout << "Limit order placed successfully. Order ID: " << orderId << endl;    
    // } else {    
    //     cerr << "Failed to place order." << endl;    
    // }

    // Modify the order
    // double newPrice = 66200.0;  
    // double newAmount = 80;  
    // if (modifyOrder(accessToken, orderId, newPrice, newAmount)) {
    //     cout << "Order modified successfully." << endl;
    // } else {
    //     cerr << "Failed to modify order." << endl;
    // }

    // Cancel the order
    // if (cancelOrder(accessToken, orderId)) {
    //     cout << "Order canceled successfully." << endl;
    // } else {
    //     cerr << "Failed to cancel order." << endl;
    // }

    // Get order book data
    // string symbol = "BTC-PERPETUAL";  
    // json orderBookData = getOrderBook(symbol);
    // if (!orderBookData.empty()) {
    //     cout << "Order Book for " << symbol << ":\n";
    //     cout << orderBookData.dump(4) << endl;  
    // 

    // // Get current positions
    // json positions = getCurrentPositions(accessToken);
    // cout << "Current Positions: " << positions.dump(4) << endl;  

    return 0;
}
