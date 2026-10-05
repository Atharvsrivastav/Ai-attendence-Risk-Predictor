/**
 * DSA: SLIDING WINDOW (RECENT ATTENDANCE ANALYSIS)
 * 
 * Purpose:
 * Analyzes the most recent N sessions (e.g., last 5 or 10 sessions)
 * using an array slice / sliding window technique to capture short-term behavioral shifts.
 * 
 * Viva Explainability:
 * - Technique: Fixed-size window over the tail of the attendance history array.
 * - Time Complexity: O(k) where k is window size (typically k = 5 or 10, thus O(1) bounded).
 * - Space Complexity: O(k) for the extracted window snapshot.
 */

import { AttendanceStatus } from "../types";

export interface RecentWindowAnalysisResult {
  windowSize: number;
  sessionsInWindow: number;
  presentCount: number;
  absentCount: number;
  lateCount: number;
  recentPercentage: number;
  recentStatuses: AttendanceStatus[];
  dropDetected: boolean;
}

export function analyzeRecentAttendanceWindow(
  chronologicalStatuses: AttendanceStatus[],
  windowSize = 5,
  overallPercentage = 100
): RecentWindowAnalysisResult {
  const total = chronologicalStatuses.length;
  const startIdx = Math.max(0, total - windowSize);
  const recentSlice = chronologicalStatuses.slice(startIdx);

  let present = 0;
  let absent = 0;
  let late = 0;

  for (let i = 0; i < recentSlice.length; i++) {
    const s = recentSlice[i];
    if (s === "PRESENT") present++;
    else if (s === "ABSENT") absent++;
    else if (s === "LATE") late++;
  }

  const effectivePresent = present + late * 0.5;
  const sessionsInWindow = present + absent + late;
  const recentPercentage =
    sessionsInWindow === 0 ? 100 : Number(((effectivePresent / sessionsInWindow) * 100).toFixed(2));

  // Drop detected if recent attendance is 15% lower than overall semester average
  const dropDetected = recentPercentage < overallPercentage - 15;

  return {
    windowSize,
    sessionsInWindow,
    presentCount: present,
    absentCount: absent,
    lateCount: late,
    recentPercentage,
    recentStatuses: recentSlice,
    dropDetected,
  };
}
