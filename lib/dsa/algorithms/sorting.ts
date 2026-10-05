/**
 * DSA: SORTING ALGORITHMS
 * 
 * Implementations:
 * 1. MergeSort (Divide-and-Conquer, Stable, Guaranteed O(n log n))
 * 2. QuickSort (Partitioning, In-Place, Average O(n log n))
 * 
 * Purpose:
 * Sorts students by:
 * - Attendance Percentage (ascending: most at-risk first)
 * - Risk Score (descending: highest risk first)
 * - Total Absences (descending: highest absences first)
 * - Roll Number (ascending: alphabetical/numerical order for binary search)
 * 
 * Viva Explainability:
 * - MergeSort:
 *    * Divide-and-Conquer: recursively halves arrays, then merges two sorted halves in O(n).
 *    * Time Complexity: Best O(n log n), Average O(n log n), Worst O(n log n).
 *    * Space Complexity: O(n) auxiliary memory for merge buffer.
 *    * Stability: Stable (preserves original order of equal elements).
 * - QuickSort:
 *    * Partitioning: picks pivot, arranges elements <= pivot to left and > pivot to right.
 *    * Time Complexity: Best O(n log n), Average O(n log n), Worst O(n^2) (e.g., bad pivot).
 *    * Space Complexity: O(log n) call stack auxiliary memory.
 *    * In-Place: Yes.
 */

export interface SortBenchmarkResult<T> {
  algorithm: "MergeSort" | "QuickSort" | "NativeSort";
  sortedArray: T[];
  comparisons: number;
  swaps: number;
  durationMicroseconds: number;
  timeComplexity: string;
  spaceComplexity: string;
  isStable: boolean;
}

export type Comparator<T> = (a: T, b: T) => number;

/**
 * MergeSort implementation with step and comparison tracking.
 */
export function mergeSortWithMetrics<T>(
  arr: T[],
  comparator: Comparator<T>
): { sorted: T[]; comparisons: number } {
  let comparisons = 0;

  function merge(left: T[], right: T[]): T[] {
    const result: T[] = [];
    let i = 0;
    let j = 0;

    while (i < left.length && j < right.length) {
      comparisons++;
      if (comparator(left[i], right[j]) <= 0) {
        result.push(left[i]);
        i++;
      } else {
        result.push(right[j]);
        j++;
      }
    }

    while (i < left.length) {
      result.push(left[i]);
      i++;
    }
    while (j < right.length) {
      result.push(right[j]);
      j++;
    }

    return result;
  }

  function sort(items: T[]): T[] {
    if (items.length <= 1) {
      return items;
    }
    const mid = Math.floor(items.length / 2);
    const left = sort(items.slice(0, mid));
    const right = sort(items.slice(mid));
    return merge(left, right);
  }

  const sorted = sort([...arr]);
  return { sorted, comparisons };
}

/**
 * QuickSort implementation with Lomuto partition, tracking comparisons and swaps.
 */
export function quickSortWithMetrics<T>(
  arr: T[],
  comparator: Comparator<T>
): { sorted: T[]; comparisons: number; swaps: number } {
  const items = [...arr];
  let comparisons = 0;
  let swaps = 0;

  function swap(i: number, j: number) {
    const temp = items[i];
    items[i] = items[j];
    items[j] = temp;
    swaps++;
  }

  function partition(low: number, high: number): number {
    const pivot = items[high];
    let i = low - 1;

    for (let j = low; j < high; j++) {
      comparisons++;
      if (comparator(items[j], pivot) <= 0) {
        i++;
        if (i !== j) {
          swap(i, j);
        }
      }
    }
    swap(i + 1, high);
    return i + 1;
  }

  function sort(low: number, high: number) {
    if (low < high) {
      const pi = partition(low, high);
      sort(low, pi - 1);
      sort(pi + 1, high);
    }
  }

  sort(0, items.length - 1);
  return { sorted: items, comparisons, swaps };
}

/**
 * Benchmarks MergeSort vs QuickSort vs Native Sort for Viva display.
 */
export function benchmarkSorting<T>(
  arr: T[],
  comparator: Comparator<T>
): {
  mergeSort: SortBenchmarkResult<T>;
  quickSort: SortBenchmarkResult<T>;
  nativeSort: SortBenchmarkResult<T>;
} {
  // MergeSort
  const t0 = performance.now();
  const msResult = mergeSortWithMetrics(arr, comparator);
  const t1 = performance.now();
  const msDuration = Math.round((t1 - t0) * 1000); // us

  // QuickSort
  const t2 = performance.now();
  const qsResult = quickSortWithMetrics(arr, comparator);
  const t3 = performance.now();
  const qsDuration = Math.round((t3 - t2) * 1000); // us

  // Native JS Sort (V8 TimSort)
  const t4 = performance.now();
  const nativeSorted = [...arr].sort(comparator);
  const t5 = performance.now();
  const nativeDuration = Math.round((t5 - t4) * 1000); // us

  return {
    mergeSort: {
      algorithm: "MergeSort",
      sortedArray: msResult.sorted,
      comparisons: msResult.comparisons,
      swaps: 0,
      durationMicroseconds: Math.max(msDuration, 1),
      timeComplexity: "O(n log n)",
      spaceComplexity: "O(n)",
      isStable: true,
    },
    quickSort: {
      algorithm: "QuickSort",
      sortedArray: qsResult.sorted,
      comparisons: qsResult.comparisons,
      swaps: qsResult.swaps,
      durationMicroseconds: Math.max(qsDuration, 1),
      timeComplexity: "O(n log n) avg, O(n²) worst",
      spaceComplexity: "O(log n)",
      isStable: false,
    },
    nativeSort: {
      algorithm: "NativeSort",
      sortedArray: nativeSorted,
      comparisons: -1,
      swaps: -1,
      durationMicroseconds: Math.max(nativeDuration, 1),
      timeComplexity: "O(n log n) (TimSort)",
      spaceComplexity: "O(n)",
      isStable: true,
    },
  };
}
