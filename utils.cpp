#include "utils.h"
#include <cstddef> // For size_t
#include <iostream>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include <algorithm>  // For std::find
#include <future>  // For std::promise and std::future

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

MemoryPool::MemoryPool(size_t blockSize, size_t blockCount) : blockSize(blockSize), blockCount(blockCount) {
    memory = new char[blockSize * blockCount];
    for (size_t i = 0; i < blockCount; ++i) {
        freeBlocks.push_back(memory + i * blockSize);
    }
}

MemoryPool::~MemoryPool() {
    delete[] memory;
}

void* MemoryPool::allocate() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (freeBlocks.empty()) {
        std::cerr << "Memory pool exhausted" << std::endl;
        return nullptr;
    }
    void* ptr = freeBlocks.back();
    freeBlocks.pop_back();
    return ptr;
}

void MemoryPool::deallocate(void* ptr) {
    std::lock_guard<std::mutex> lock(poolMutex);
    freeBlocks.push_back(ptr);
}

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueueTask(const function<void()>& task) {
    {
        unique_lock<mutex> lock(queueMutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

void ThreadPool::workerThread() {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

ThreadPool threadPool(thread::hardware_concurrency()); // Global thread pool instance

nlohmann::json rapidjsonToNlohmannJson(const rapidjson::Value& value) {
    if (value.IsObject()) {
        nlohmann::json result = nlohmann::json::object();
        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            result[it->name.GetString()] = rapidjsonToNlohmannJson(it->value);
        }
        return result;
    } else if (value.IsArray()) {
        nlohmann::json result = nlohmann::json::array();
        for (auto it = value.Begin(); it != value.End(); ++it) {
            result.push_back(rapidjsonToNlohmannJson(*it));
        }
        return result;
    } else if (value.IsString()) {
        return value.GetString();
    } else if (value.IsBool()) {
        return value.GetBool();
    } else if (value.IsInt()) {
        return value.GetInt();
    } else if (value.IsUint()) {
        return value.GetUint();
    } else if (value.IsInt64()) {
        return value.GetInt64();
    } else if (value.IsUint64()) {
        return value.GetUint64();
    } else if (value.IsDouble()) {
        return value.GetDouble();
    } else {
        return nullptr;
    }
}

bool parseJSONResponse(const std::string& response, rapidjson::Document& document) {
    document.Parse(response.c_str());
    return !document.HasParseError();
}
//working //