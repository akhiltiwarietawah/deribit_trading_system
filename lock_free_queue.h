#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <atomic>
#include <memory>

template <typename T>
class LockFreeQueue {
public:
    LockFreeQueue();
    ~LockFreeQueue();

    void enqueue(T value);
    bool dequeue(T& result);

private:
    struct Node {
        T data;
        std::shared_ptr<Node> next;
        Node(T value) : data(value), next(nullptr) {}
    };

    std::shared_ptr<Node> head, tail;
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue() : head(new Node(T())), tail(head) {}

template <typename T>
LockFreeQueue<T>::~LockFreeQueue() {}

template <typename T>
void LockFreeQueue<T>::enqueue(T value) {
    std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
    std::shared_ptr<Node> old_tail;

    while (true) {
        old_tail = tail;
        std::shared_ptr<Node> tail_next = old_tail->next;
        if (old_tail == tail) {
            if (tail_next == nullptr) {
                if (std::atomic_compare_exchange_weak(&old_tail->next, &tail_next, new_node)) {
                    break;
                }
            } else {
                std::atomic_compare_exchange_weak(&tail, &old_tail, tail_next);
            }
        }
    }
    std::atomic_compare_exchange_weak(&tail, &old_tail, new_node);
}

template <typename T>
bool LockFreeQueue<T>::dequeue(T& result) {
    std::shared_ptr<Node> old_head;
    while (true) {
        old_head = head;
        std::shared_ptr<Node> tail = tail;
        std::shared_ptr<Node> head_next = old_head->next;

        if (old_head == head) {
            if (old_head == tail) {
                if (head_next == nullptr) {
                    return false;
                }
                std::atomic_compare_exchange_weak(&tail, &tail, head_next);
            } else {
                result = head_next->data;
                if (std::atomic_compare_exchange_weak(&head, &old_head, head_next)) {
                    break;
                }
            }
        }
    }
    return true;
}

#endif // LOCK_FREE_QUEUE_H
