#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <optional>
#include "models/Models.hpp"

namespace attendance::dsa {

template <typename T>
struct StackNode {
    T value;
    std::shared_ptr<StackNode<T>> next{nullptr};
    explicit StackNode(const T& v) : value(v), next(nullptr) {}
};

template <typename T>
class AttendanceStack {
public:
    explicit AttendanceStack(size_t maxDepth = 100)
        : top_(nullptr), count_(0), maxDepth_(maxDepth) {}

    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto newNode = std::make_shared<StackNode<T>>(value);
        newNode->next = top_;
        top_ = newNode;
        count_++;

        if (count_ > maxDepth_) {
            pruneBottom();
        }
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!top_) return std::nullopt;
        T val = top_->value;
        top_ = top_->next;
        count_--;
        return val;
    }

    std::optional<T> peek() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!top_) return std::nullopt;
        return top_->value;
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_ == 0;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        top_ = nullptr;
        count_ = 0;
    }

    std::vector<T> toArray() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> result;
        auto cur = top_;
        while (cur) {
            result.push_back(cur->value);
            cur = cur->next;
        }
        return result;
    }

private:
    void pruneBottom() {
        if (!top_ || count_ <= 1) return;
        auto cur = top_;
        for (size_t i = 0; i < maxDepth_ - 1 && cur->next; i++) {
            cur = cur->next;
        }
        cur->next = nullptr;
        count_ = maxDepth_;
    }

    std::shared_ptr<StackNode<T>> top_;
    size_t count_;
    size_t maxDepth_;
    mutable std::mutex mutex_;
};

// Global in-memory undo stack instance
inline AttendanceStack<attendance::models::UndoAction>& getGlobalUndoStack() {
    static AttendanceStack<attendance::models::UndoAction> stack(100);
    return stack;
}

} // namespace attendance::dsa
