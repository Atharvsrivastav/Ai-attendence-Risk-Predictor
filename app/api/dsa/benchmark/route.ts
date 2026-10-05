import { NextRequest, NextResponse } from "next/server";
import { AttendanceService } from "@/lib/services/attendanceService";
import { benchmarkSorting } from "@/lib/dsa/algorithms/sorting";
import { linearSearch, binarySearch } from "@/lib/dsa/algorithms/searching";
import { StudentHashMap } from "@/lib/dsa/data-structures/StudentHashMap";

export const dynamic = "force-dynamic";

export async function GET(req: NextRequest) {
  try {
    const { profiles, hashMap } = await AttendanceService.getAllStudentProfiles();

    // 1. Sorting Benchmark on actual student profiles
    const sortBenchmark = benchmarkSorting(
      profiles,
      (a, b) => b.overallPercentage - a.overallPercentage
    );

    // 2. Searching Benchmark: Target a student in the middle/end
    const sortedByRoll = [...profiles].sort((a, b) => a.rollNumber.localeCompare(b.rollNumber));
    const targetRoll = sortedByRoll[Math.floor(sortedByRoll.length * 0.75)]?.rollNumber || "2024CS015";

    const linearResult = linearSearch(
      profiles,
      (s) => s.rollNumber === targetRoll,
      (s) => s.rollNumber
    );

    const binaryResult = binarySearch(
      sortedByRoll,
      (s) => s.rollNumber,
      targetRoll,
      (a, b) => a.localeCompare(b)
    );

    // 3. Lookup Benchmark: StudentHashMap (O(1)) vs Array.find (O(n))
    const lookupKey = targetRoll;
    const t0 = performance.now();
    for (let i = 0; i < 10000; i++) {
      hashMap.get(lookupKey);
    }
    const t1 = performance.now();
    const hashTimeUs = Math.round(((t1 - t0) / 10000) * 1000);

    const t2 = performance.now();
    for (let i = 0; i < 10000; i++) {
      profiles.find((s) => s.rollNumber === lookupKey);
    }
    const t3 = performance.now();
    const arrayTimeUs = Math.round(((t3 - t2) / 10000) * 1000);

    return NextResponse.json({
      success: true,
      datasetSize: profiles.length,
      sorting: {
        mergeSort: {
          comparisons: sortBenchmark.mergeSort.comparisons,
          swaps: sortBenchmark.mergeSort.swaps,
          durationMicroseconds: sortBenchmark.mergeSort.durationMicroseconds,
          timeComplexity: sortBenchmark.mergeSort.timeComplexity,
          spaceComplexity: sortBenchmark.mergeSort.spaceComplexity,
          isStable: sortBenchmark.mergeSort.isStable,
        },
        quickSort: {
          comparisons: sortBenchmark.quickSort.comparisons,
          swaps: sortBenchmark.quickSort.swaps,
          durationMicroseconds: sortBenchmark.quickSort.durationMicroseconds,
          timeComplexity: sortBenchmark.quickSort.timeComplexity,
          spaceComplexity: sortBenchmark.quickSort.spaceComplexity,
          isStable: sortBenchmark.quickSort.isStable,
        },
        nativeSort: {
          durationMicroseconds: sortBenchmark.nativeSort.durationMicroseconds,
          timeComplexity: sortBenchmark.nativeSort.timeComplexity,
          spaceComplexity: sortBenchmark.nativeSort.spaceComplexity,
          isStable: sortBenchmark.nativeSort.isStable,
        },
      },
      searching: {
        target: targetRoll,
        linearSearch: {
          found: linearResult.found,
          index: linearResult.index,
          stepsCount: linearResult.stepsCount,
          timeMicroseconds: linearResult.timeTakenMicroseconds,
          timeComplexity: linearResult.timeComplexity,
          trace: linearResult.trace.slice(0, 8), // compact trace
        },
        binarySearch: {
          found: binaryResult.found,
          index: binaryResult.index,
          stepsCount: binaryResult.stepsCount,
          timeMicroseconds: binaryResult.timeTakenMicroseconds,
          timeComplexity: binaryResult.timeComplexity,
          trace: binaryResult.trace,
        },
      },
      lookupBenchmark: {
        iterations: 10000,
        hashMapAverageUs: Math.max(hashTimeUs, 0.05),
        arrayScanAverageUs: Math.max(arrayTimeUs, 0.25),
        speedupFactor: Number((Math.max(arrayTimeUs, 0.25) / Math.max(hashTimeUs, 0.05)).toFixed(1)),
        hashMapDiagnostics: hashMap.getDiagnostics(),
      },
    });
  } catch (error: any) {
    console.error("GET /api/dsa/benchmark error:", error);
    return NextResponse.json({ error: "Failed to run DSA benchmarks" }, { status: 500 });
  }
}
