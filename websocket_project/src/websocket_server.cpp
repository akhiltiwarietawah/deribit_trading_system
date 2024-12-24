// #include <iostream>
// #include <cstdlib>
// #include <string>
// #include <thread>
// #include <nlohmann/json.hpp>
// #include <boost/asio.hpp>
// #include <websocketpp/config/asio_no_tls.hpp>
// #include <websocketpp/server.hpp>
// #include <curl/curl.h>
// #include <functional>
// #include <set>
// #include <unordered_map>

// using namespace std;
// using json = nlohmann::json;
// using websocketpp::connection_hdl;

// struct connection_hdl_hash {
//     size_t operator()(connection_hdl hdl) const {
//         return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(hdl.lock().get()));
//     }
// };

// struct connection_hdl_equal {
//     bool operator()(connection_hdl a, connection_hdl b) const {
//         return a.lock() == b.lock();
//     }
// };

// class WebSocketServer {
// public:
//     WebSocketServer() {
//         m_server.init_asio();
//         m_server.set_open_handler(bind(&WebSocketServer::on_open, this, websocketpp::lib::placeholders::_1));
//         m_server.set_close_handler(bind(&WebSocketServer::on_close, this, websocketpp::lib::placeholders::_1));
//         m_server.set_message_handler(bind(&WebSocketServer::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
//     }

//     void run(uint16_t port) {
//         m_server.listen(port);
//         m_server.start_accept();
//         m_server.run();
//     }

// private:
//     using server = websocketpp::server<websocketpp::config::asio>;
//     server m_server;
//     // storing connectn
//     set<connection_hdl, std::owner_less<connection_hdl>> m_connections;  
//     // store subscrptn
//     unordered_map<connection_hdl, string, connection_hdl_hash, connection_hdl_equal> m_subscriptions; // Store subscriptions

//     void on_open(connection_hdl hdl) {
//         cout << "Client connected!" << endl;
//         // add new connectn
//         m_connections.insert(hdl); 
//     }

//     void on_close(connection_hdl hdl) {
//         cout << "Client disconnected!" << endl;
//         //remove
//         m_connections.erase(hdl); 
//         m_subscriptions.erase(hdl);  
//     }

//     void on_message(connection_hdl hdl, server::message_ptr msg) {
//         string payload = msg->get_payload();
//         cout << "Received message: " << payload << endl;

//         // process subscriptn msg
//         json request = json::parse(payload);
//         if (request.contains("symbol")) {
//             string symbol = request["symbol"];
//             //update subcptn
//             m_subscriptions[hdl] = symbol;  

//             // send init order book
//             json orderBookData = getOrderBook(symbol);
//             if (!orderBookData.empty()) {
//                 m_server.send(hdl, orderBookData.dump(), websocketpp::frame::opcode::text);
//             }

//             // thread to send subcrubd data
//             thread(&WebSocketServer::send_order_book_updates, this, hdl, symbol).detach();
//         }
//     }

//     void send_order_book_updates(connection_hdl hdl, const string& symbol) {
//         while (m_connections.count(hdl)) {  
//             this_thread::sleep_for(chrono::seconds(1));  

//             json orderBookData = getOrderBook(symbol);  
//             if (!orderBookData.empty()) {
//                 m_server.send(hdl, orderBookData.dump(), websocketpp::frame::opcode::text);
//             }
//         }
//     }

//     json getOrderBook(const string& symbol) {
//         string apiUrl = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
//         CURL* curl;
//         CURLcode res;
//         string readBuffer;

//         curl = curl_easy_init();
//         if (curl) {
//             curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
//             curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//             curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
//             curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

//             res = curl_easy_perform(curl);
//             if (res != CURLE_OK) {
//                 cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
//                 curl_easy_cleanup(curl);
//                 return {};
//             }
//             curl_easy_cleanup(curl);
//         }

//         json orderBook = json::parse(readBuffer, nullptr, false);
//         if (orderBook.is_discarded() || orderBook["result"].is_null()) {
//             cerr << "Failed to fetch order book data." << endl;
//             return {};
//         }

//         return orderBook["result"];
//     }
//     //fnctn to handle data after rqst
//     static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* out) {
//         size_t totalSize = size * nmemb;
//         out->append(static_cast<const char*>(contents), totalSize);
//         return totalSize;
//     }
// };

// int main() {
//     const char* client_id = getenv("DERIBIT_CLIENT_ID");
//     const char* client_secret = getenv("DERIBIT_CLIENT_SECRET");

//     if (!client_id || !client_secret) {
//         cerr << "Client ID or secret is not set in environment variables." << endl;
//         return 1;
//     }

//     // Create the server instance
//     WebSocketServer server;
//     server.run(9002); // Run server on port 9002

//     return 0;
// }




#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <curl/curl.h>
#include <functional>
#include <set>
#include <unordered_map>
#include <chrono>

using namespace std;
using json = nlohmann::json;
using websocketpp::connection_hdl;

struct connection_hdl_hash {
    size_t operator()(connection_hdl hdl) const {
        return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(hdl.lock().get()));
    }
};

struct connection_hdl_equal {
    bool operator()(connection_hdl a, connection_hdl b) const {
        return a.lock() == b.lock();
    }
};

class WebSocketServer {
public:
    WebSocketServer() {
        m_server.init_asio();
        m_server.set_open_handler(bind(&WebSocketServer::on_open, this, websocketpp::lib::placeholders::_1));
        m_server.set_close_handler(bind(&WebSocketServer::on_close, this, websocketpp::lib::placeholders::_1));
        m_server.set_message_handler(bind(&WebSocketServer::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }

private:
    using server = websocketpp::server<websocketpp::config::asio>;
    server m_server;
    set<connection_hdl, std::owner_less<connection_hdl>> m_connections;
    unordered_map<connection_hdl, string, connection_hdl_hash, connection_hdl_equal> m_subscriptions;

    void on_open(connection_hdl hdl) {
        cout << "Client connected!" << endl;
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        cout << "Client disconnected!" << endl;
        m_connections.erase(hdl);
        m_subscriptions.erase(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        string payload = msg->get_payload();
        cout << "Received message: " << payload << endl;

        json request = json::parse(payload);
        if (request.contains("symbol")) {
            string symbol = request["symbol"];
            m_subscriptions[hdl] = symbol;

            json orderBookData = getOrderBook(symbol);
            if (!orderBookData.empty()) {
                m_server.send(hdl, orderBookData.dump(), websocketpp::frame::opcode::text);
            }

            thread(&WebSocketServer::send_order_book_updates, this, hdl, symbol).detach();
        }
    }

    void send_order_book_updates(connection_hdl hdl, const string& symbol) {
        while (m_connections.count(hdl)) {
            this_thread::sleep_for(chrono::seconds(1));

            json orderBookData = getOrderBook(symbol);
            if (!orderBookData.empty()) {
                // Add server send timestamp
                auto now = chrono::system_clock::now();
                auto millis = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
                orderBookData["server_send_time"] = millis;

                m_server.send(hdl, orderBookData.dump(), websocketpp::frame::opcode::text);
            }
        }
    }

    json getOrderBook(const string& symbol) {
        string apiUrl = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
        CURL* curl;
        CURLcode res;
        string readBuffer;

        // Start the latency measurement for market data processing
        auto start_time = chrono::high_resolution_clock::now();

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
                curl_easy_cleanup(curl);
                return {};
            }
            curl_easy_cleanup(curl);
        }

        json orderBook = json::parse(readBuffer, nullptr, false);
        if (orderBook.is_discarded() || orderBook["result"].is_null()) {
            cerr << "Failed to fetch order book data." << endl;
            return {};
        }

        // End the latency measurement for market data processing
        auto end_time = chrono::high_resolution_clock::now();
        auto processing_latency = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

        // Add processing latency to the order book data
        orderBook["result"]["processing_latency_ms"] = processing_latency;

        return orderBook["result"];
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* out) {
        size_t totalSize = size * nmemb;
        out->append(static_cast<const char*>(contents), totalSize);
        return totalSize;
    }
};

int main() {
    const char* client_id = getenv("DERIBIT_CLIENT_ID");
    const char* client_secret = getenv("DERIBIT_CLIENT_SECRET");

    if (!client_id || !client_secret) {
        cerr << "Client ID or secret is not set in environment variables." << endl;
        return 1;
    }

    WebSocketServer server;
    server.run(9002);

    return 0;
}
