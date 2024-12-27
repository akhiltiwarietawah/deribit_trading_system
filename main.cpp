#include <iostream>
#include <chrono>
#include "order_management.h"
#include "token_manager.h"
#include "utils.h"
#include "CurlConnectionManager.h"

using namespace std;

void benchmark(OrderAPI& orderAPI, const string& instrument, double quantity, double price, const string& label) {
    std::string orderId;
    auto overallStart = std::chrono::high_resolution_clock::now();

    auto start = chrono::high_resolution_clock::now();
    if (orderAPI.placeMarketOrder(instrument, quantity, label, orderId)) {
        cout << "Market order placed successfully." << endl;
    } else {
        cerr << "Failed to place market order." << endl;
    }
    auto end = chrono::high_resolution_clock::now();
    cout << "Market Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

    start = chrono::high_resolution_clock::now();
    if (orderAPI.placeLimitOrder(instrument, quantity, price, orderId)) {
        cout << "Limit order placed successfully." << endl;
    } else {
        cerr << "Failed to place limit order." << endl;
    }
    end = chrono::high_resolution_clock::now();
    cout << "Limit Order Placement Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

    start = chrono::high_resolution_clock::now();
    
    if (orderAPI.modifyOrder(orderId, price * 2, quantity * 2)) {
        cout << "Order modified successfully." << endl;
    } else {
        cerr << "Failed to modify order." << endl;
    }
    end = chrono::high_resolution_clock::now();
    cout << "Modify Order Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

    nlohmann::json orderBookData = orderAPI.getOrderBook(instrument);
    if (!orderBookData.empty()) {
        cout << "Order Book for " << instrument << " fetched successfully." << endl;
    } else {
        cerr << "Failed to get order book." << endl;
    }

    start = chrono::high_resolution_clock::now();
    if (orderAPI.cancelOrder(orderId)) {
        cout << "Order canceled successfully." << endl;
    } else {
        cerr << "Failed to cancel order." << endl;
    }
    end = chrono::high_resolution_clock::now();
    cout << "Cancel Order Latency: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

    auto overallEnd = std::chrono::high_resolution_clock::now();
    cout << "Total Execution Latency: " << chrono::duration_cast<chrono::milliseconds>(overallEnd - overallStart).count() << " ms" << endl;
}

int main() {
    const char* client_id = getenv("DERIBIT_CLIENT_ID");
    const char* client_secret = getenv("DERIBIT_CLIENT_SECRET");

    if (!client_id || !client_secret) {
        cerr << "Client ID or secret is not set in environment variables." << endl;
        return 1;
    }

    CURLcode globalInit = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (globalInit != CURLE_OK) {
        cerr << "CURL global initialization failed!" << endl;
        return 1;
    }

    TokenManager tokenManager(client_id, client_secret);
    string accessToken = tokenManager.getAccessToken();
    if (accessToken.empty()) {
        cerr << "Failed to retrieve access token." << endl;
        curl_global_cleanup(); 
        return 1;
    }

    CurlConnectionManager curlManager(10);
    OrderAPI orderAPI(curlManager, thread::hardware_concurrency(), tokenManager);

    // Call the benchmark function to measure latencies
    benchmark(orderAPI, "BTC-PERPETUAL", 40, 6000.0, "market0000234");
    // Final log message to confirm completion 
    std::cout << "All operations completed successfully." <<std::endl;
    curl_global_cleanup();
    return 0;
}
