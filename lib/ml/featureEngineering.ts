/**
 * ML: FEATURE ENGINEERING PIPELINE
 * 
 * Purpose:
 * Transforms raw DSA attendance arrays and profile metrics into a normalized,
 * mathematically consistent feature vector for ML risk prediction.
 * 
 * Features:
 * 1. overallPercentage (0 - 100)
 * 2. recentPercentage (0 - 100, last 5 sessions)
 * 3. consecutiveAbsences (0 - N)
 * 4. totalAbsences (0 - N)
 * 5. trendSlope (-1.0 to +1.0)
 * 6. recentAbsenceFrequency (0.0 to 1.0)
 * 7. thresholdGap (percentage points below/above target threshold, e.g. 75 - overall)
 */

import { AttendanceStatus } from "../dsa/types";
import { calculateAttendancePercentage } from "../dsa/algorithms/attendanceCalculator";
import { detectConsecutiveAbsences } from "../dsa/algorithms/consecutiveAbsences";
import { analyzeAttendanceTrend } from "../dsa/algorithms/trendAnalysis";
import { analyzeRecentAttendanceWindow } from "../dsa/algorithms/recentWindowAnalysis";

export interface AttendanceFeatureVector {
  overallPercentage: number;
  recentPercentage: number;
  consecutiveAbsences: number;
  totalAbsences: number;
  trendSlope: number;
  recentAbsenceFrequency: number;
  thresholdGap: number; // positive = below threshold by X%, negative = above threshold by X%
  totalConducted: number;
}

export function extractAttendanceFeatures(
  chronologicalStatuses: AttendanceStatus[],
  threshold = 75
): AttendanceFeatureVector {
  const calc = calculateAttendancePercentage(chronologicalStatuses, threshold);
  const consecutive = detectConsecutiveAbsences(chronologicalStatuses);
  const trend = analyzeAttendanceTrend(chronologicalStatuses);
  const recentWindow = analyzeRecentAttendanceWindow(chronologicalStatuses, 5, calc.percentage);

  const recentAbsenceFrequency =
    recentWindow.sessionsInWindow === 0
      ? 0
      : Number((recentWindow.absentCount / recentWindow.sessionsInWindow).toFixed(3));

  const thresholdGap = Number((threshold - calc.percentage).toFixed(2));

  return {
    overallPercentage: calc.percentage,
    recentPercentage: recentWindow.recentPercentage,
    consecutiveAbsences: consecutive.currentStreak,
    totalAbsences: calc.absentCount,
    trendSlope: trend.slope,
    recentAbsenceFrequency,
    thresholdGap,
    totalConducted: calc.totalConducted,
  };
}
