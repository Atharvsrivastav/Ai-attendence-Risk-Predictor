/**
 * DSA: ATTENDANCE PERCENTAGE CALCULATION ALGORITHM
 * 
 * Formula:
 * Attendance % = (Effective Present Sessions / Total Conducted Sessions) * 100
 * 
 * Viva Explainability:
 * - Time Complexity: O(n) where n is the number of sessions recorded for this student.
 * - Space Complexity: O(1) auxiliary variables.
 * - Traversal: Single-pass linear scan across array of attendance status codes.
 */

import { AttendanceStatus } from "../types";

export interface AttendanceCalculationResult {
  totalConducted: number;
  presentCount: number;
  absentCount: number;
  lateCount: number;
  excusedCount: number;
  effectivePresentCount: number;
  percentage: number;
  isEligible: boolean;
}

export function calculateAttendancePercentage(
  statuses: AttendanceStatus[],
  threshold = 75,
  lateWeight = 0.5 // college policy: late counts as 0.5 present
): AttendanceCalculationResult {
  let present = 0;
  let absent = 0;
  let late = 0;
  let excused = 0;

  for (let i = 0; i < statuses.length; i++) {
    const s = statuses[i];
    switch (s) {
      case "PRESENT":
        present++;
        break;
      case "ABSENT":
        absent++;
        break;
      case "LATE":
        late++;
        break;
      case "EXCUSED":
        excused++;
        break;
    }
  }

  // Conducted excludes excused medical/college duty leaves
  const totalConducted = present + absent + late;
  const effectivePresent = present + late * lateWeight;

  const percentage =
    totalConducted === 0 ? 100 : Number(((effectivePresent / totalConducted) * 100).toFixed(2));

  return {
    totalConducted,
    presentCount: present,
    absentCount: absent,
    lateCount: late,
    excusedCount: excused,
    effectivePresentCount: effectivePresent,
    percentage,
    isEligible: percentage >= threshold,
  };
}
