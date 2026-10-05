/**
 * DSA: ATTENDANCE RECOVERY & SIMULATION ALGORITHM
 * 
 * Mathematical Formulations:
 * Let T = Total Conducted Sessions
 * Let P = Total Attended Sessions (effective)
 * Let R = Target Threshold fraction (e.g. 0.75 for 75%)
 * 
 * 1. Classes needed to recover to target threshold:
 *    (P + X) / (T + X) >= R
 *    X >= (R * T - P) / (1 - R)
 *    X = ceil( max(0, (R * T - P) / (1 - R)) )
 * 
 * 2. Classes that can still be missed (Buffer):
 *    P / (T + M) >= R
 *    M <= (P - R * T) / R
 *    M = floor( max(0, (P - R * T) / R) )
 * 
 * 3. What-if Simulation:
 *    Simulated % = ((P + attendNext) / (T + attendNext + missNext)) * 100
 * 
 * Viva Explainability:
 * - Time Complexity: O(1) direct algebraic evaluation.
 * - Space Complexity: O(1).
 */

export interface RecoveryCalculationResult {
  currentPercentage: number;
  threshold: number;
  totalConducted: number;
  presentCount: number;
  absentCount: number;
  classesNeededToRecover: number;
  classesCanBeMissed: number;
  isBelowThreshold: boolean;
  statusSummary: string;
}

export interface SimulationResult {
  additionalAttended: number;
  additionalMissed: number;
  projectedTotalSessions: number;
  projectedAttendedSessions: number;
  projectedPercentage: number;
  meetsThreshold: boolean;
  deltaPercentage: number;
}

export function calculateRecovery(
  presentSessions: number,
  totalSessions: number,
  threshold = 75
): RecoveryCalculationResult {
  const currentPercentage =
    totalSessions === 0 ? 100 : Number(((presentSessions / totalSessions) * 100).toFixed(2));

  const R = threshold / 100;
  const isBelowThreshold = currentPercentage < threshold;

  let classesNeededToRecover = 0;
  let classesCanBeMissed = 0;
  let statusSummary = "";

  if (totalSessions === 0) {
    return {
      currentPercentage: 100,
      threshold,
      totalConducted: 0,
      presentCount: 0,
      absentCount: 0,
      classesNeededToRecover: 0,
      classesCanBeMissed: 0,
      isBelowThreshold: false,
      statusSummary: "No classes conducted yet.",
    };
  }

  if (isBelowThreshold) {
    // Formula: ceil((R * T - P) / (1 - R))
    const needed = (R * totalSessions - presentSessions) / (1 - R);
    classesNeededToRecover = Math.max(1, Math.ceil(needed));
    classesCanBeMissed = 0;
    statusSummary = `Needs to attend the next ${classesNeededToRecover} consecutive classes to reach ${threshold}%.`;
  } else {
    // Formula: floor((P - R * T) / R)
    const canMiss = (presentSessions - R * totalSessions) / R;
    classesCanBeMissed = Math.max(0, Math.floor(canMiss));
    classesNeededToRecover = 0;
    statusSummary = `Currently safe. Can miss up to ${classesCanBeMissed} class${classesCanBeMissed === 1 ? "" : "es"} without dropping below ${threshold}%.`;
  }

  return {
    currentPercentage,
    threshold,
    totalConducted: totalSessions,
    presentCount: presentSessions,
    absentCount: totalSessions - presentSessions,
    classesNeededToRecover,
    classesCanBeMissed,
    isBelowThreshold,
    statusSummary,
  };
}

/**
 * Interactive What-If Simulator
 */
export function simulateAttendance(
  currentPresent: number,
  currentTotal: number,
  attendNext: number,
  missNext = 0,
  threshold = 75
): SimulationResult {
  const projectedTotal = currentTotal + attendNext + missNext;
  const projectedPresent = currentPresent + attendNext;

  const currentPercentage =
    currentTotal === 0 ? 100 : (currentPresent / currentTotal) * 100;

  const projectedPercentage =
    projectedTotal === 0 ? 100 : Number(((projectedPresent / projectedTotal) * 100).toFixed(2));

  return {
    additionalAttended: attendNext,
    additionalMissed: missNext,
    projectedTotalSessions: projectedTotal,
    projectedAttendedSessions: projectedPresent,
    projectedPercentage,
    meetsThreshold: projectedPercentage >= threshold,
    deltaPercentage: Number((projectedPercentage - currentPercentage).toFixed(2)),
  };
}
