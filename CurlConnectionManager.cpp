#include "CurlConnectionManager.h"
#include <iostream>

CurlConnectionManager::CurlConnectionManager(size_t poolSize) : poolSize(poolSize), bufferPool(1024, 100) { // Initialize buffer pool
    CURLcode global_init_res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (global_init_res != CURLE_OK) {
        std::cerr << "CURL global initialization failed: " 
                  << curl_easy_strerror(global_init_res) << std::endl;
        return;
    }

    initializePool();
}

CurlConnectionManager::~CurlConnectionManager() {
    for (CURL* handle : curlPool) {
        curl_easy_cleanup(handle);
    }
    curl_global_cleanup();
}

void CurlConnectionManager::initializePool() {
    for (size_t i = 0; i < poolSize; ++i) {
        CURL* handle = curl_easy_init();
        if (handle) {
            setupCurlOptions(handle);
            curlPool.push_back(handle);
            freeHandles.push_back(handle);
        }
    }
}

void CurlConnectionManager::setupCurlOptions(CURL* handle) {
    curl_easy_reset(handle);
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(handle, CURLOPT_TCP_NODELAY, 1L); // Disable Nagle's algorithm
    curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, 2000L); // Set timeout to 2 seconds
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L); // Set connection timeout to 1 second
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirections
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 3L); // Maximum redirections
    curl_easy_setopt(handle, CURLOPT_DNS_CACHE_TIMEOUT, 3600L); // DNS cache timeout
    curl_easy_setopt(handle, CURLOPT_PIPEWAIT, 1L); // Wait for pipe/multiplex
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L); // Enable SSL peer verification
    curl_easy_setopt(handle, CURLOPT_FORBID_REUSE, 0L); // Do not close connection after use
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0); // Use HTTP/2 for better performance
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 2L); // Set overall timeout to 2 seconds
    curl_easy_setopt(handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 500L); // Reduce timeout for 100-continue responses
    curl_easy_setopt(handle, CURLOPT_HEADEROPT, CURLHEADER_UNIFIED); // Optimize header processing
    curl_easy_setopt(handle, CURLOPT_HEADER, 1L); // Enable header output
}

CURL* CurlConnectionManager::getCurlHandle() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (freeHandles.empty()) {
        std::cerr << "No free CURL handles available!" << std::endl;
        return nullptr;
    }
    CURL* handle = freeHandles.back();
    freeHandles.pop_back();
    return handle;
}

void CurlConnectionManager::releaseCurlHandle(CURL* curl) {
    std::lock_guard<std::mutex> lock(poolMutex);
    freeHandles.push_back(curl);
}
