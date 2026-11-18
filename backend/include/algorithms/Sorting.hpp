#pragma once

#include <vector>
#include <functional>
#include <chrono>

namespace attendance::algorithms {

template <typename T>
struct SortMetrics {
    std::vector<T> sorted;
    size_t comparisons{0};
    size_t swaps{0};
    long long durationMicroseconds{0};
};

template <typename T, typename Comparator>
void merge(std::vector<T>& arr, int left, int mid, int right, Comparator comp, size_t& comparisons) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<T> L(n1);
    std::vector<T> R(n2);

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        comparisons++;
        if (comp(L[i], R[j])) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

template <typename T, typename Comparator>
void mergeSortHelper(std::vector<T>& arr, int left, int right, Comparator comp, size_t& comparisons) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(arr, left, mid, comp, comparisons);
        mergeSortHelper(arr, mid + 1, right, comp, comparisons);
        merge(arr, left, mid, right, comp, comparisons);
    }
}

template <typename T, typename Comparator = std::less<T>>
SortMetrics<T> mergeSortWithMetrics(const std::vector<T>& input, Comparator comp = Comparator()) {
    SortMetrics<T> metrics;
    metrics.sorted = input;
    if (metrics.sorted.size() <= 1) return metrics;

    auto start = std::chrono::high_resolution_clock::now();
    mergeSortHelper(metrics.sorted, 0, static_cast<int>(metrics.sorted.size() - 1), comp, metrics.comparisons);
    auto end = std::chrono::high_resolution_clock::now();

    metrics.durationMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return metrics;
}

template <typename T, typename Comparator>
int partition(std::vector<T>& arr, int low, int high, Comparator comp, size_t& comparisons, size_t& swaps) {
    T pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        comparisons++;
        if (comp(arr[j], pivot)) {
            i++;
            if (i != j) {
                std::swap(arr[i], arr[j]);
                swaps++;
            }
        }
    }
    if (i + 1 != high) {
        std::swap(arr[i + 1], arr[high]);
        swaps++;
    }
    return i + 1;
}

template <typename T, typename Comparator>
void quickSortHelper(std::vector<T>& arr, int low, int high, Comparator comp, size_t& comparisons, size_t& swaps) {
    if (low < high) {
        int pi = partition(arr, low, high, comp, comparisons, swaps);
        quickSortHelper(arr, low, pi - 1, comp, comparisons, swaps);
        quickSortHelper(arr, pi + 1, high, comp, comparisons, swaps);
    }
}

template <typename T, typename Comparator = std::less<T>>
SortMetrics<T> quickSortWithMetrics(const std::vector<T>& input, Comparator comp = Comparator()) {
    SortMetrics<T> metrics;
    metrics.sorted = input;
    if (metrics.sorted.size() <= 1) return metrics;

    auto start = std::chrono::high_resolution_clock::now();
    quickSortHelper(metrics.sorted, 0, static_cast<int>(metrics.sorted.size() - 1), comp, metrics.comparisons, metrics.swaps);
    auto end = std::chrono::high_resolution_clock::now();

    metrics.durationMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return metrics;
}

} // namespace attendance::algorithms
