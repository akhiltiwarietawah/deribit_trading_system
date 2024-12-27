//websocket_server.cpp
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <curl/curl.h>
#include <zlib.h>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sys/resource.h>
#include <memory> // For smart pointers

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

        // Open log file
        logFile.open("server_metrics.log", ios::out | ios::app);
        if (!logFile.is_open()) {
            cerr << "Failed to open log file." << endl;
        }
    }

    ~WebSocketServer() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }

private:
    using server = websocketpp::server<websocketpp::config::asio>;
    server m_server;
    unordered_map<connection_hdl, string, connection_hdl_hash, connection_hdl_equal> m_subscriptions;
    ofstream logFile;

    void log_metrics(const string& metric) {
        if (logFile.is_open()) {
            logFile << chrono::system_clock::to_time_t(chrono::system_clock::now()) << ": " << metric << endl;
        }
    }

    void on_open(connection_hdl hdl) {
        cout << "Client connected!" << endl;
        m_subscriptions[hdl] = "";
        log_metrics("Client connected");
    }

    void on_close(connection_hdl hdl) {
        cout << "Client disconnected!" << endl;
        m_subscriptions.erase(hdl);
        log_metrics("Client disconnected");
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        auto receivedTime = chrono::system_clock::now().time_since_epoch();
        auto receivedMillis = chrono::duration_cast<chrono::milliseconds>(receivedTime).count();

        std::thread([this, hdl, msg, receivedMillis]() {
            string payload = msg->get_payload();
            cout << "Received message: " << payload << endl;
            log_metrics("Received message: " + payload);

            json request = json::parse(payload, nullptr, false);
            if (request.contains("client_send_time")) {
                auto clientSendTime = request["client_send_time"].get<int64_t>();
                auto propagationDelay = receivedMillis - clientSendTime;
                log_metrics("WebSocket Message Propagation Delay: " + to_string(propagationDelay) + " ms");
            }

            if (request.contains("symbol")) {
                string symbol = request["symbol"];
                m_subscriptions[hdl] = symbol;

                json orderBookData = getOrderBook(symbol);
                if (!orderBookData.empty()) {
                    m_server.send(hdl, compressString(orderBookData.dump()), websocketpp::frame::opcode::binary);
                }

                send_order_book_updates(hdl, symbol);
            }
        }).detach();
    }

    void send_order_book_updates(connection_hdl hdl, const string& symbol) {
        while (m_subscriptions.count(hdl)) {
            this_thread::sleep_for(chrono::seconds(1));

            json orderBookData = getOrderBook(symbol);
            if (!orderBookData.empty()) {
                auto now = chrono::system_clock::now().time_since_epoch();
                orderBookData["server_send_time"] = chrono::duration_cast<chrono::milliseconds>(now).count();

                m_server.send(hdl, compressString(orderBookData.dump()), websocketpp::frame::opcode::binary);

                // Log send time
                log_metrics("Order book update sent for " + symbol);
            }
        }
    }

    json getOrderBook(const string& symbol) {
        static const string apiUrlBase = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=";
        string apiUrl = apiUrlBase + symbol;
        CURL* curl;
        CURLcode res;
        string readBuffer;

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
            cerr << "Failed to fetch order book data: " << orderBook << endl;
            return {};
        }

        auto end_time = chrono::high_resolution_clock::now();
        auto processing_latency = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

        orderBook["result"]["processing_latency_ms"] = processing_latency;

        return orderBook["result"];
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* out) {
        size_t totalSize = size * nmemb;
        out->append(static_cast<const char*>(contents), totalSize);
        return totalSize;
    }

    string compressString(const string& str) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
            throw(runtime_error("deflateInit failed while compressing."));
        }

        zs.next_in = (Bytef*)str.data();
        zs.avail_in = str.size();

        int ret;
        char outbuffer[32768];
        string outstring;

        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);

            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            throw(runtime_error("deflate failed while compressing."));
        }

        return outstring;
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
