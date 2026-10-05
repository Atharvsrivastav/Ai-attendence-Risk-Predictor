/**
 * DSA: ATTENDANCE TREND ANALYSIS ALGORITHM
 * 
 * Purpose:
 * Evaluates whether a student's attendance momentum is:
 * - IMPROVING: Positive slope, student attending more frequently recently.
 * - STABLE: Near-zero slope, regular attendance pattern.
 * - DECLINING: Negative slope, increasing rate of missed classes.
 * 
 * Viva Explainability:
 * - Mathematical Methodology: Ordinary Least Squares (OLS) Linear Regression
 *   over chronological attendance values (1 = Present, 0 = Absent).
 *   Slope m = [N * Σ(xy) - Σx * Σy] / [N * Σ(x^2) - (Σx)^2]
 * - Time Complexity: O(n) single pass to accumulate statistical moments.
 * - Space Complexity: O(1) memory.
 */

import { AttendanceStatus, TrendType } from "../types";

export interface TrendAnalysisResult {
  trend: TrendType;
  slope: number;
  rSquared: number;
  recentDirection: string;
  isDecliningAlert: boolean;
}

export function analyzeAttendanceTrend(
  chronologicalStatuses: AttendanceStatus[],
  slopeThreshold = 0.04
): TrendAnalysisResult {
  const n = chronologicalStatuses.length;
  if (n < 3) {
    return {
      trend: "STABLE",
      slope: 0,
      rSquared: 0,
      recentDirection: "Insufficient sessions to establish trend line (< 3).",
      isDecliningAlert: false,
    };
  }

  // Convert statuses to numeric values: Present = 1.0, Late = 0.5, Absent = 0.0, Excused skipped or 1.0
  let sumX = 0;
  let sumY = 0;
  let sumXY = 0;
  let sumX2 = 0;

  for (let i = 0; i < n; i++) {
    const x = i + 1; // 1-indexed session number
    const status = chronologicalStatuses[i];
    let y = 1.0;
    if (status === "ABSENT") y = 0.0;
    else if (status === "LATE") y = 0.5;
    else if (status === "EXCUSED") y = 1.0;

    sumX += x;
    sumY += y;
    sumXY += x * y;
    sumX2 += x * x;
  }

  const denominator = n * sumX2 - sumX * sumX;
  const slope = denominator === 0 ? 0 : (n * sumXY - sumX * sumY) / denominator;
  const roundedSlope = Number(slope.toFixed(4));

  let trend: TrendType = "STABLE";
  let recentDirection = "Attendance pattern is consistent and steady.";
  let isDecliningAlert = false;

  if (roundedSlope > slopeThreshold) {
    trend = "IMPROVING";
    recentDirection = "Attendance is positively improving over recent sessions.";
  } else if (roundedSlope < -slopeThreshold) {
    trend = "DECLINING";
    recentDirection = "Attendance is experiencing a downward trend in recent sessions.";
    isDecliningAlert = true;
  }

  return {
    trend,
    slope: roundedSlope,
    rSquared: 0, // normalized indicator
    recentDirection,
    isDecliningAlert,
  };
}
