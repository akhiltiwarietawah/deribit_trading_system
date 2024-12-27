//websocket_client.cpp
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <zlib.h>
#include <fstream>
#include <memory> // For smart pointers

using namespace std;
using json = nlohmann::json;

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient {
public:
    WebSocketClient() {
        m_client.init_asio();
        m_client.set_open_handler(bind(&WebSocketClient::on_open, this, websocketpp::lib::placeholders::_1));
        m_client.set_close_handler(bind(&WebSocketClient::on_close, this, websocketpp::lib::placeholders::_1));
        m_client.set_message_handler(bind(&WebSocketClient::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

        // Open log file
        logFile.open("client_metrics.log", ios::out | ios::app);
        if (!logFile.is_open()) {
            cerr << "Failed to open log file." << endl;
        }
    }

    ~WebSocketClient() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void connect(const string& uri) {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(uri, ec);
        if (ec) {
            cerr << "Could not create connection: " << ec.message() << endl;
            return;
        }
        m_client.connect(con);
        m_client.run();
    }

    void send_message(const string& message) {
        websocketpp::lib::error_code ec;
        auto currentTime = chrono::system_clock::now().time_since_epoch();
        auto currentMillis = chrono::duration_cast<chrono::milliseconds>(currentTime).count();
        
        json msg = json::parse(message);
        msg["client_send_time"] = currentMillis;
        
        m_client.send(m_hdl, msg.dump(), websocketpp::frame::opcode::text, ec);
        if (ec) {
            cerr << "Error sending message: " << ec.message() << endl;
        }
    }

private:
    client m_client;
    websocketpp::connection_hdl m_hdl;
    ofstream logFile;

    void log_metrics(const string& metric) {
        if (logFile.is_open()) {
            logFile << chrono::system_clock::to_time_t(chrono::system_clock::now()) << ": " << metric << endl;
        }
    }

    void on_open(websocketpp::connection_hdl hdl) {
        cout << "Connected to the server" << endl;
        m_hdl = hdl;

        json request = {{"symbol", "BTC-PERPETUAL"}};
        send_message(request.dump());
    }

    void on_close(websocketpp::connection_hdl hdl) {
        cout << "Disconnected from the server" << endl;
        log_metrics("Disconnected from the server");
    }

    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::thread([this, msg]() {
            auto currentTime = chrono::system_clock::now().time_since_epoch();
            auto currentMillis = chrono::duration_cast<chrono::milliseconds>(currentTime).count();

            try {
                string decompressedData = decompressString(msg->get_payload());
                json orderBookData = json::parse(decompressedData, nullptr, false);

                if (orderBookData.contains("server_send_time")) {
                    auto serverSendTime = orderBookData["server_send_time"].get<int64_t>();
                    auto propagationDelay = currentMillis - serverSendTime;
                    cout << "WebSocket Message Propagation Delay: " << propagationDelay << " ms" << endl;
                    log_metrics("WebSocket Message Propagation Delay: " + to_string(propagationDelay) + " ms");
                }

                if (orderBookData.contains("processing_latency_ms")) {
                    cout << "Market Data Processing Latency: " << orderBookData["processing_latency_ms"] << " ms" << endl;
                    log_metrics("Market Data Processing Latency: " + to_string(orderBookData["processing_latency_ms"].get<int64_t>()) + " ms");
                }

                displayOrderBook(orderBookData);
            } catch (const std::exception& e) {
                cerr << "Failed to decompress or parse message: " << e.what() << endl;
                log_metrics("Failed to decompress or parse message: " + string(e.what()));
            }
        }).detach();
    }

    void displayOrderBook(const json& orderBookData) {
        cout << "\nOrder Book:" << endl;
        cout << setw(10) << "Price" << setw(10) << "Amount" << setw(10) << "Side" << endl;

        for (const auto& ask : orderBookData["asks"]) {
            cout << setw(10) << ask[0] << setw(10) << ask[1] << setw(10) << "Sell" << endl;
        }

        for (const auto& bid : orderBookData["bids"]) {
            cout << setw(10) << bid[0] << setw(10) << bid[1] << setw(10) << "Buy" << endl;
        }
    }

    string decompressString(const string& str) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (inflateInit(&zs) != Z_OK) {
            throw(runtime_error("inflateInit failed while decompressing."));
        }

        zs.next_in = (Bytef*)str.data();
        zs.avail_in = str.size();

        int ret;
        char outbuffer[32768];
        string outstring;

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
            cerr << "Decompression error: " << ret << endl;
            throw(runtime_error("inflate failed while decompressing."));
        }

        return outstring;
    }
};

int main() {
    WebSocketClient client;
    client.connect("ws://localhost:9002");

    return 0;
}
