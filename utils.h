#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <memory>
#include <cstddef> // For size_t
#include <string>
#include <curl/curl.h>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <rapidjson/document.h>
#include <nlohmann/json.hpp>

// Function to write data received from curl into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

class MemoryPool {
public:
    MemoryPool(size_t blockSize, size_t blockCount);
    ~MemoryPool();

    void* allocate();
    void deallocate(void* ptr);

private:
    size_t blockSize;
    size_t blockCount;
    std::vector<void*> freeBlocks;
    std::mutex poolMutex;
    char* memory;
};

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueueTask(const std::function<void()>& task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    void workerThread();
};

// Declare the global thread pool instance
extern ThreadPool threadPool;

// Template for async execution
template <typename T>
T executeAsyncTask(std::function<T()> task) {
    std::promise<T> promise;
    auto future = promise.get_future();

    threadPool.enqueueTask([&]() {
        promise.set_value(task());
    });

    return future.get();
}

// JSON conversion function
nlohmann::json rapidjsonToNlohmannJson(const rapidjson::Value& value);

// JSON parsing function
bool parseJSONResponse(const std::string& response, rapidjson::Document& document);

#endif // UTILS_H
// working