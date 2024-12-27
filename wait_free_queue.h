#ifndef WAIT_FREE_QUEUE_H
#define WAIT_FREE_QUEUE_H

#include <atomic>
#include <vector>

template <typename T>
class WaitFreeQueue {
public:
    WaitFreeQueue(size_t capacity) : capacity_(capacity), head_(0), tail_(0) {
        buffer_.resize(capacity);
    }

    bool enqueue(const T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (tail + 1) % capacity_;

        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue is full
        }

        buffer_[tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool dequeue(T& item) {
        size_t head = head_.load(std::memory_order_relaxed);

        if (head == tail_.load(std::memory_order_acquire)) {
            return false; // Queue is empty
        }

        item = buffer_[head];
        head_.store((head + 1) % capacity_, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_;
    std::atomic<size_t> head_, tail_;
};

#endif // WAIT_FREE_QUEUE_H
