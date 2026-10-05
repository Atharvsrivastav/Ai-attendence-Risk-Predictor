/**
 * DSA: SEARCHING ALGORITHMS
 * 
 * Implementations:
 * 1. Linear Search (Unsorted sequences, O(n) Time, O(1) Space)
 * 2. Binary Search (Sorted collections, O(log n) Time, O(1) Space)
 * 
 * Viva Explainability:
 * - Linear Search:
 *    * Use case: Searching by Student Name or unindexed attributes in an arbitrary list.
 *    * Precondition: Works on ANY array, regardless of sort order.
 *    * Time Complexity: Best O(1), Average O(n/2) -> O(n), Worst O(n).
 * - Binary Search:
 *    * Use case: Fast searching by Roll Number or ID when records are sorted.
 *    * Precondition: REQUIRES the array to be monotonically sorted.
 *    * Mechanism: Repeatedly divides search interval in half.
 *    * Time Complexity: Best O(1), Average O(log n), Worst O(log n).
 *    * For n = 1,000,000 records:
 *        Linear Search worst case = 1,000,000 checks.
 *        Binary Search worst case = ~20 checks!
 */

export interface SearchTraceStep {
  index: number;
  low?: number;
  mid?: number;
  high?: number;
  valueExamined: string | number;
  isMatch: boolean;
  actionTaken: string;
}

export interface SearchResult<T> {
  algorithm: "LinearSearch" | "BinarySearch";
  found: boolean;
  index: number;
  item: T | null;
  stepsCount: number;
  timeTakenMicroseconds: number;
  timeComplexity: string;
  spaceComplexity: string;
  trace: SearchTraceStep[];
}

/**
 * Linear Search with step recording.
 */
export function linearSearch<T>(
  arr: T[],
  predicate: (item: T) => boolean,
  displayExtractor: (item: T) => string | number
): SearchResult<T> {
  const t0 = performance.now();
  const trace: SearchTraceStep[] = [];

  for (let i = 0; i < arr.length; i++) {
    const val = displayExtractor(arr[i]);
    const match = predicate(arr[i]);

    trace.push({
      index: i,
      valueExamined: val,
      isMatch: match,
      actionTaken: match ? "Target found! Terminating search." : "Not a match. Advancing to next index.",
    });

    if (match) {
      const t1 = performance.now();
      return {
        algorithm: "LinearSearch",
        found: true,
        index: i,
        item: arr[i],
        stepsCount: i + 1,
        timeTakenMicroseconds: Math.max(Math.round((t1 - t0) * 1000), 1),
        timeComplexity: "O(n)",
        spaceComplexity: "O(1)",
        trace,
      };
    }
  }

  const t1 = performance.now();
  return {
    algorithm: "LinearSearch",
    found: false,
    index: -1,
    item: null,
    stepsCount: arr.length,
    timeTakenMicroseconds: Math.max(Math.round((t1 - t0) * 1000), 1),
    timeComplexity: "O(n)",
    spaceComplexity: "O(1)",
    trace,
  };
}

/**
 * Binary Search on sorted array with divide-and-conquer trace.
 */
export function binarySearch<T, K extends string | number>(
  sortedArr: T[],
  keyExtractor: (item: T) => K,
  targetKey: K,
  comparator: (a: K, b: K) => number
): SearchResult<T> {
  const t0 = performance.now();
  const trace: SearchTraceStep[] = [];
  let low = 0;
  let high = sortedArr.length - 1;
  let steps = 0;

  while (low <= high) {
    steps++;
    const mid = Math.floor((low + high) / 2);
    const midItem = sortedArr[mid];
    const midKey = keyExtractor(midItem);
    const cmp = comparator(midKey, targetKey);

    if (cmp === 0) {
      trace.push({
        index: mid,
        low,
        mid,
        high,
        valueExamined: midKey,
        isMatch: true,
        actionTaken: `Match at mid index ${mid}! Key matches target.`,
      });

      const t1 = performance.now();
      return {
        algorithm: "BinarySearch",
        found: true,
        index: mid,
        item: midItem,
        stepsCount: steps,
        timeTakenMicroseconds: Math.max(Math.round((t1 - t0) * 1000), 1),
        timeComplexity: "O(log n)",
        spaceComplexity: "O(1)",
        trace,
      };
    }

    if (cmp < 0) {
      trace.push({
        index: mid,
        low,
        mid,
        high,
        valueExamined: midKey,
        isMatch: false,
        actionTaken: `Mid value (${midKey}) < target (${targetKey}). Discarding left half; updating low = ${mid + 1}.`,
      });
      low = mid + 1;
    } else {
      trace.push({
        index: mid,
        low,
        mid,
        high,
        valueExamined: midKey,
        isMatch: false,
        actionTaken: `Mid value (${midKey}) > target (${targetKey}). Discarding right half; updating high = ${mid - 1}.`,
      });
      high = mid - 1;
    }
  }

  const t1 = performance.now();
  return {
    algorithm: "BinarySearch",
    found: false,
    index: -1,
    item: null,
    stepsCount: steps,
    timeTakenMicroseconds: Math.max(Math.round((t1 - t0) * 1000), 1),
    timeComplexity: "O(log n)",
    spaceComplexity: "O(1)",
    trace,
  };
}
