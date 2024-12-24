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
    string label = "market0000234";
    string symbol = "BTC-PERPETUAL";  



    // start measuring end-to-end latency
    auto overallStart = chrono::high_resolution_clock::now();


    // Place a market order

    auto start = chrono::high_resolution_clock::now();
    if (placeMarketOrder(accessToken, instrument, quantity, label, orderId)) {
        cout << "Market order placed successfully." << endl;
    } else {
        cerr << "Failed to place market order." << endl;
        return 1;
    }

    auto end = chrono::high_resolution_clock::now();
    cout << "Market Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


    // // Place a limit order

    // start = chrono::high_resolution_clock::now();
    // if (placeOrder(accessToken, instrument, quantity, price, orderId)) {
    //     cout << "Limit order placed successfully. Order ID: " << orderId << endl;    
    // } else {    
    //     cerr << "Failed to place order." << endl;    
    // }
    // end = chrono::high_resolution_clock::now();
    // cout << "Limit Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


    //get order status 
    json orderStatus = getOrderStatus(accessToken, orderId);
    if (!orderStatus.empty()) {
        // cout << "Order status: " << orderStatus.dump(4) << endl;
    } else {
        cerr << "Failed to get order status." << endl;
        return 1;
    }


    // Get order book data

    start = chrono::high_resolution_clock::now();

    json orderBookData = getOrderBook(symbol);
    if (!orderBookData.empty()) {
        cout << "Order Book for " << symbol << ":\n";
        cout << orderBookData.dump(4) << endl;  
    }

    end = chrono::high_resolution_clock::now();
    cout << "Market Data processing Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


    

    // Modify the order
    double newPrice = 66200.0;  
    double newAmount = 80;  

    start = chrono::high_resolution_clock::now();
    if (modifyOrder(accessToken, orderId, newPrice, newAmount)) {
        cout << "Order modified successfully." << endl;
    } else {
        cerr << "Failed to modify order." << endl;
    }
    end = chrono::high_resolution_clock::now();
    cout << "Order Modification Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;



    // Cancel the order

    start = chrono::high_resolution_clock::now();

    if (cancelOrder(accessToken, orderId)) {
        cout << "Order canceled successfully." << endl;
    } else {
        cerr << "Failed to cancel order." << endl;
    }

    end = chrono::high_resolution_clock::now();
    cout << "Cancel order Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


    // Get current positions
    json positions = getCurrentPositions(accessToken);
    cout << "Current Positions: " << positions.dump(4) << endl; 


    // Measure overall latency
    auto overallEnd = chrono::high_resolution_clock::now();
    cout << "End-to-End Trading Loop Latency: " << chrono::duration_cast<chrono::milliseconds>(overallEnd - overallStart).count() << " ms" << endl;
 

    return 0;
}