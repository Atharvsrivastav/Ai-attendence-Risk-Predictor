#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <optional>
#include "models/Models.hpp"

namespace attendance::dsa {

template <typename T>
struct QueueNode {
    T value;
    std::shared_ptr<QueueNode<T>> next{nullptr};
    explicit QueueNode(const T& v) : value(v), next(nullptr) {}
};

template <typename T>
class AttendanceQueue {
public:
    AttendanceQueue() : head_(nullptr), tail_(nullptr), length_(0) {}

    void enqueue(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto newNode = std::make_shared<QueueNode<T>>(value);
        if (!tail_) {
            head_ = newNode;
            tail_ = newNode;
        } else {
            tail_->next = newNode;
            tail_ = newNode;
        }
        length_++;
    }

    std::optional<T> dequeue() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!head_) {
            return std::nullopt;
        }
        T val = head_->value;
        head_ = head_->next;
        if (!head_) {
            tail_ = nullptr;
        }
        length_--;
        return val;
    }

    std::optional<T> peek() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!head_) return std::nullopt;
        return head_->value;
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return length_ == 0;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return length_;
    }

    std::vector<T> toArray() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> result;
        auto cur = head_;
        while (cur) {
            result.push_back(cur->value);
            cur = cur->next;
        }
        return result;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = nullptr;
        tail_ = nullptr;
        length_ = 0;
    }

private:
    std::shared_ptr<QueueNode<T>> head_;
    std::shared_ptr<QueueNode<T>> tail_;
    size_t length_;
    mutable std::mutex mutex_;
};

// Global in-memory queue instance
inline AttendanceQueue<attendance::models::AlertEvent>& getGlobalAlertQueue() {
    static AttendanceQueue<attendance::models::AlertEvent> queue;
    return queue;
}

} // namespace attendance::dsa
