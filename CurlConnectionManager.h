#ifndef CURL_CONNECTION_MANAGER_H
#define CURL_CONNECTION_MANAGER_H

#include <curl/curl.h>
#include <vector>
#include <mutex>
#include <algorithm>
#include "utils.h" // Include the memory pool header

class CurlConnectionManager {
public:
    CurlConnectionManager(size_t poolSize = 10); // Constructor with pool size
    ~CurlConnectionManager();

    CURL* getCurlHandle();
    void releaseCurlHandle(CURL* curl);

private:
    std::vector<CURL*> curlPool;
    std::vector<CURL*> freeHandles;
    std::mutex poolMutex;
    size_t poolSize;

    MemoryPool bufferPool; // Memory pool for managing temporary buffers

    void initializePool();
    void setupCurlOptions(CURL* handle); // Method to set up CURL options
};

#endif // CURL_CONNECTION_MANAGER_H
