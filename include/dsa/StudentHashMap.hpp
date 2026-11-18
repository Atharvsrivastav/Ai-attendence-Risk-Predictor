#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include "json.hpp"

namespace attendance::dsa {

template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    std::shared_ptr<HashNode<K, V>> next{nullptr};

    HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
};

template <typename K, typename V>
class StudentHashMap {
public:
    explicit StudentHashMap(size_t initialCapacity = 32)
        : capacity_(initialCapacity), size_(0), collisionCount_(0) {
        buckets_.resize(capacity_, nullptr);
    }

    // Polynomial rolling hash function (djb2 algorithm)
    size_t hash(const std::string& key) const {
        unsigned long hashVal = 5381;
        for (unsigned char c : key) {
            hashVal = ((hashVal << 5) + hashVal) + c; // hashVal * 33 + c
        }
        return hashVal % capacity_;
    }

    void set(const K& key, const V& value) {
        size_t index = hash(key);
        auto current = buckets_[index];

        // Check if key already exists (update in-place)
        while (current) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // New node insertion at bucket head
        if (buckets_[index] != nullptr) {
            collisionCount_++;
        }

        auto newNode = std::make_shared<HashNode<K, V>>(key, value);
        newNode->next = buckets_[index];
        buckets_[index] = newNode;
        size_++;

        // Auto-rehash if load factor exceeds 0.75
        if (static_cast<double>(size_) / capacity_ > 0.75) {
            resize(capacity_ * 2);
        }
    }

    bool get(const K& key, V& outValue) const {
        size_t index = hash(key);
        auto current = buckets_[index];
        while (current) {
            if (current->key == key) {
                outValue = current->value;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool has(const K& key) const {
        V temp;
        return get(key, temp);
    }

    bool remove(const K& key) {
        size_t index = hash(key);
        auto current = buckets_[index];
        std::shared_ptr<HashNode<K, V>> prev = nullptr;

        while (current) {
            if (current->key == key) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    buckets_[index] = current->next;
                }
                size_--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t collisionCount() const { return collisionCount_; }
    bool isEmpty() const { return size_ == 0; }

    std::vector<K> keys() const {
        std::vector<K> keyList;
        for (const auto& head : buckets_) {
            auto current = head;
            while (current) {
                keyList.push_back(current->key);
                current = current->next;
            }
        }
        return keyList;
    }

    std::vector<V> values() const {
        std::vector<V> valList;
        for (const auto& head : buckets_) {
            auto current = head;
            while (current) {
                valList.push_back(current->value);
                current = current->next;
            }
        }
        return valList;
    }

    void clear() {
        buckets_.clear();
        capacity_ = 32;
        buckets_.resize(capacity_, nullptr);
        size_ = 0;
        collisionCount_ = 0;
    }

    nlohmann::json getDiagnostics() const {
        size_t occupiedBuckets = 0;
        size_t emptyBuckets = 0;
        size_t maxChainLength = 0;

        for (const auto& head : buckets_) {
            if (!head) {
                emptyBuckets++;
            } else {
                occupiedBuckets++;
                size_t chainLen = 0;
                auto cur = head;
                while (cur) {
                    chainLen++;
                    cur = cur->next;
                }
                if (chainLen > maxChainLength) {
                    maxChainLength = chainLen;
                }
            }
        }

        double loadFactor = capacity_ == 0 ? 0.0 : static_cast<double>(size_) / capacity_;

        return {
            {"size", size_},
            {"capacity", capacity_},
            {"loadFactor", std::round(loadFactor * 100.0) / 100.0},
            {"emptyBuckets", emptyBuckets},
            {"occupiedBuckets", occupiedBuckets},
            {"maxChainLength", maxChainLength},
            {"collisionCount", collisionCount_}
        };
    }

private:
    void resize(size_t newCapacity) {
        std::vector<std::shared_ptr<HashNode<K, V>>> oldBuckets = buckets_;
        capacity_ = newCapacity;
        buckets_.clear();
        buckets_.resize(capacity_, nullptr);
        size_ = 0;
        collisionCount_ = 0;

        for (const auto& head : oldBuckets) {
            auto current = head;
            while (current) {
                set(current->key, current->value);
                current = current->next;
            }
        }
    }

    std::vector<std::shared_ptr<HashNode<K, V>>> buckets_;
    size_t capacity_;
    size_t size_;
    size_t collisionCount_;
};

} // namespace attendance::dsa
