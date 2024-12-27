# Project: WebSocket Server and Client with Latency Benchmarking and Optimization

## Introduction
This project aims to build a WebSocket server and client with benchmarking and optimize the performance of trading-related operations. The documentation covers system architecture, setup instructions, code overview, latency benchmarking, implemented optimizations, and detailed performance analysis.

## System Architecture
The project consists of two main components:
1. **WebSocket Server:** Handles WebSocket connections, processes market data, and sends updates to clients.
2. **WebSocket Client:** Connects to the WebSocket server, receives market data updates, and logs metrics.

 

## Setup and Installation

### Prerequisites
- **C++ Compiler:** Ensure you have a C++11 or later compiler installed.
- **Libraries:** Install the necessary libraries: Boost.Asio, WebSocket++, nlohmann/json, cURL, zlib.

### Installation Steps
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/your-repo/websocket-benchmark.git
   cd websocket-project

2. **Build the Server:**

   ```bash
    cd websocket_project/
    mkdir build
    cd build
    cmake ..
    make
    ./websocket_project

3. **Build the Client:**

   ```bash
    cd websocket_client
    mkdir build
    cd build
    cmake ..
    make
    ./websocket_client

4. **Run the Server:**
    ```bash
    ./websocket_server
    ```

5. ## Run the Client:
```bash
./websocket_client
```

# Code Overview

### WebSocket Server
The server handles WebSocket connections, processes incoming messages, fetches market data, and sends compressed order book updates to clients.

**Key Methods:**
- `run(uint16_t port)`: Starts the WebSocket server.
- `on_open(connection_hdl hdl)`: Handles new client connections.
- `on_close(connection_hdl hdl)`: Handles client disconnections.
- `on_message(connection_hdl hdl, server::message_ptr msg)`: Processes incoming messages.
- `send_order_book_updates(connection_hdl hdl, const string& symbol)`: Sends order book updates to clients.
- `getOrderBook(const string& symbol)`: Fetches order book data from the API.

### WebSocket Client
The client connects to the server, subscribes to market data updates, logs metrics, and displays the order book.

**Key Methods:**
- `connect(const string& uri)`: Connects to the WebSocket server.
- `send_message(const string& message)`: Sends messages to the server.
- `on_open(websocketpp::connection_hdl hdl)`: Handles successful connection.
- `on_close(websocketpp::connection_hdl hdl)`: Handles disconnection.
- `on_message(websocketpp::connection_hdl hdl, client::message_ptr msg)`: Processes incoming messages and logs metrics.
- `displayOrderBook(const json& orderBookData)`: Displays the order book data.

## Latency Benchmarking

We measure the following latency metrics:
- **Order Placement Latency**: Time taken from sending an order to receiving confirmation.
- **Market Data Processing Latency**: Time taken to fetch and process market data.
- **WebSocket Message Propagation Delay**: Time taken for a message to travel from the client to the server and back.
- **End-to-End Trading Loop Latency**: Total time taken for a complete trading loop.

### Code Snippet for Measuring Latency

```cpp
// Measure WebSocket message propagation delay
void on_message(connection_hdl hdl, server::message_ptr msg) {
    auto receivedTime = chrono::system_clock::now().time_since_epoch();
    auto receivedMillis = chrono::duration_cast<chrono::milliseconds>(receivedTime).count();
    ...
    // Log propagation delay
    if (request.contains("client_send_time")) {
        auto clientSendTime = request["client_send_time"].get<int64_t>();
        auto propagationDelay = receivedMillis - clientSendTime;
        log_metrics("WebSocket Message Propagation Delay: " + to_string(propagationDelay) + " ms");
    }
}
```

## Optimizations

### Memory Management
Used smart pointers to manage dynamic memory allocation, ensuring memory safety and avoiding memory leaks.

### Network Communication
Compressed order book data before sending it over the WebSocket to reduce bandwidth usage.

### Data Structure Selection
Used `unordered_map` for efficient management of client subscriptions, providing average O(1) complexity for lookups.

### Thread Management
Employed detached threads to handle incoming messages and avoid blocking the main thread.

### CPU Optimization
Optimized CPU-intensive tasks using parallel processing and efficient algorithms to minimize CPU usage.

## Bottlenecks and Performance Analysis

### Identified Bottlenecks
- High latency due to frequent API calls for order book data.
- Network bandwidth consumption due to large message sizes.

### Before and After Performance Metrics

**Before Optimization:**
- Market Data Processing Latency: 50ms
- WebSocket Message Propagation Delay: 900ms

**After Optimization:**
- Market Data Processing Latency: 2ms
- WebSocket Message Propagation Delay: 600ms

## Benchmarking Methodology

**Tools Used:** Chrono library for time measurement, cURL for API calls, zlib for compression.

**Process:**
- Measured latency metrics at different stages of message handling.
- Logged metrics to analyze performance before and after optimizations.

## Further Improvements
- Implement caching to reduce API call frequency.
- Use WebSocket Secure (WSS) for encrypted communication.
- Optimize thread management using thread pools.

## Usage Instructions

### Run the Server:
```bash
./websocket_server
```

### Run the Client:
```bash
./websocket_client
```

## Troubleshooting

**Issue:** Failed to open log file.  
**Solution:** Check file permissions and ensure the directory exists.

**Issue:** Connection errors.  
**Solution:** Verify the server is running and the correct URL is used.

## Conclusion

This project demonstrates a WebSocket-based system with benchmarking and optimizing trading-related operations. By measuring latency metrics and implementing optimizations, we achieved significant performance improvements.

