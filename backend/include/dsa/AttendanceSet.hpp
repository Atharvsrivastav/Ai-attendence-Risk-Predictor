#pragma once

#include <vector>
#include <unordered_set>
#include <algorithm>

namespace attendance::dsa {

template <typename T>
class AttendanceSet {
public:
    AttendanceSet() = default;

    explicit AttendanceSet(const std::vector<T>& initialElements) {
        for (const auto& item : initialElements) {
            elements_.insert(item);
        }
    }

    bool add(const T& element) {
        return elements_.insert(element).second;
    }

    bool has(const T& element) const {
        return elements_.find(element) != elements_.end();
    }

    bool remove(const T& element) {
        return elements_.erase(element) > 0;
    }

    size_t size() const {
        return elements_.size();
    }

    bool isEmpty() const {
        return elements_.empty();
    }

    void clear() {
        elements_.clear();
    }

    std::vector<T> values() const {
        return std::vector<T>(elements_.begin(), elements_.end());
    }

    // Set Intersection: O(min(|A|, |B|))
    AttendanceSet<T> intersect(const AttendanceSet<T>& other) const {
        AttendanceSet<T> result;
        const auto& smaller = (this->size() < other.size()) ? this->elements_ : other.elements_;
        const auto& larger = (this->size() < other.size()) ? other.elements_ : this->elements_;

        for (const auto& item : smaller) {
            if (larger.find(item) != larger.end()) {
                result.add(item);
            }
        }
        return result;
    }

    // Set Union: O(|A| + |B|)
    AttendanceSet<T> unite(const AttendanceSet<T>& other) const {
        AttendanceSet<T> result = *this;
        for (const auto& item : other.elements_) {
            result.add(item);
        }
        return result;
    }

    // Set Difference: A \ B
    AttendanceSet<T> difference(const AttendanceSet<T>& other) const {
        AttendanceSet<T> result;
        for (const auto& item : elements_) {
            if (!other.has(item)) {
                result.add(item);
            }
        }
        return result;
    }

    // Is subset of: A ⊆ B
    bool isSubsetOf(const AttendanceSet<T>& other) const {
        if (this->size() > other.size()) return false;
        for (const auto& item : elements_) {
            if (!other.has(item)) return false;
        }
        return true;
    }

private:
    std::unordered_set<T> elements_;
};

} // namespace attendance::dsa
