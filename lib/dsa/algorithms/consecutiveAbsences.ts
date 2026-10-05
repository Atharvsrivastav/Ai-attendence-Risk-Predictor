/**
 * DSA: CONSECUTIVE ABSENCE DETECTION ALGORITHM
 * 
 * Purpose:
 * Detects:
 * 1. Current active consecutive absence streak (backwards from most recent class)
 * 2. Maximum historical absence streak
 * 
 * Viva Explainability:
 * - Time Complexity: O(n) where n is total session records.
 * - Current streak: O(k) where k is number of recent absent sessions, scanning from end.
 * - Space Complexity: O(1) in-place tracking variables.
 */

import { AttendanceStatus } from "../types";

export interface ConsecutiveAbsenceResult {
  currentStreak: number;
  maxStreak: number;
  hasActiveStreakWarning: boolean;
}

export function detectConsecutiveAbsences(
  chronologicalStatuses: AttendanceStatus[],
  warningThreshold = 2
): ConsecutiveAbsenceResult {
  if (chronologicalStatuses.length === 0) {
    return { currentStreak: 0, maxStreak: 0, hasActiveStreakWarning: false };
  }

  // 1. Current Active Streak: Scan backwards from the most recent session
  let currentStreak = 0;
  for (let i = chronologicalStatuses.length - 1; i >= 0; i--) {
    const status = chronologicalStatuses[i];
    if (status === "ABSENT") {
      currentStreak++;
    } else if (status === "EXCUSED") {
      // Excused does not break or increase unexcused absence streak
      continue;
    } else {
      // PRESENT or LATE breaks the current absence streak
      break;
    }
  }

  // 2. Max Historical Streak: Forward single-pass traversal
  let maxStreak = 0;
  let runningStreak = 0;

  for (let i = 0; i < chronologicalStatuses.length; i++) {
    const status = chronologicalStatuses[i];
    if (status === "ABSENT") {
      runningStreak++;
      if (runningStreak > maxStreak) {
        maxStreak = runningStreak;
      }
    } else if (status !== "EXCUSED") {
      runningStreak = 0;
    }
  }

  return {
    currentStreak,
    maxStreak,
    hasActiveStreakWarning: currentStreak >= warningThreshold,
  };
}
