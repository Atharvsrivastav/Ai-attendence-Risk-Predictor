#pragma once

#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include "json.hpp"

namespace attendance::algorithms {

struct SearchTraceStep {
    int index;
    int low{0};
    int mid{0};
    int high{0};
    std::string valueExamined;
    bool isMatch;
    std::string actionTaken;
};

struct SearchMetrics {
    std::string algorithm;
    bool found{false};
    int index{-1};
    size_t stepsCount{0};
    long long timeMicroseconds{0};
    std::string timeComplexity;
    std::vector<SearchTraceStep> trace;
};

template <typename T, typename Predicate, typename KeyExtractor>
SearchMetrics linearSearch(const std::vector<T>& list, Predicate predicate, KeyExtractor keyExtractor) {
    SearchMetrics metrics;
    metrics.algorithm = "Linear Search";
    metrics.timeComplexity = "O(n)";

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < list.size(); i++) {
        metrics.stepsCount++;
        std::string keyVal = keyExtractor(list[i]);
        bool match = predicate(list[i]);

        metrics.trace.push_back({
            static_cast<int>(i),
            0, 0, 0,
            keyVal,
            match,
            match ? "Match found at index " + std::to_string(i) : "Not a match. Advancing to next index."
        });

        if (match) {
            metrics.found = true;
            metrics.index = static_cast<int>(i);
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return metrics;
}

template <typename T, typename KeyExtractor>
SearchMetrics binarySearch(const std::vector<T>& sortedList, const std::string& targetKey, KeyExtractor keyExtractor) {
    SearchMetrics metrics;
    metrics.algorithm = "Binary Search";
    metrics.timeComplexity = "O(log n)";

    auto start = std::chrono::high_resolution_clock::now();

    int low = 0;
    int high = static_cast<int>(sortedList.size()) - 1;

    while (low <= high) {
        metrics.stepsCount++;
        int mid = low + (high - low) / 2;
        std::string midVal = keyExtractor(sortedList[mid]);

        if (midVal == targetKey) {
            metrics.trace.push_back({
                mid, low, mid, high,
                midVal,
                true,
                "Match at mid index " + std::to_string(mid) + "! Key matches target."
            });
            metrics.found = true;
            metrics.index = mid;
            break;
        } else if (midVal < targetKey) {
            metrics.trace.push_back({
                mid, low, mid, high,
                midVal,
                false,
                "Mid value (" + midVal + ") < target (" + targetKey + "). Discarding left half; updating low = " + std::to_string(mid + 1) + "."
            });
            low = mid + 1;
        } else {
            metrics.trace.push_back({
                mid, low, mid, high,
                midVal,
                false,
                "Mid value (" + midVal + ") > target (" + targetKey + "). Discarding right half; updating high = " + std::to_string(mid - 1) + "."
            });
            high = mid - 1;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return metrics;
}

} // namespace attendance::algorithms
