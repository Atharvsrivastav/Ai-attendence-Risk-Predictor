/**
 * ML: UNIFIED EXPLAINABLE RISK PREDICTOR
 * 
 * Blends:
 * 1. DSA Hierarchical Decision Tree (Tree structure rule-checking)
 * 2. Statistical Logistic Regression (Probabilistic scoring + Feature Attribution)
 * 3. Recovery Calculator (Actionable recommendations)
 * 
 * Requirement 16 (Explainable Prediction):
 * Every prediction answers: "Why is this student at risk?"
 * And provides actionable recommendations: "What must they do to recover?"
 */

import { AttendanceStatus, RiskCategory } from "../dsa/types";
import { RiskDecisionTree } from "../dsa/data-structures/RiskDecisionTree";
import { calculateRecovery } from "../dsa/algorithms/recoveryCalculator";
import { extractAttendanceFeatures } from "./featureEngineering";
import { runLogisticRegression, FeatureAttribution } from "./logisticRegressionModel";

export interface UnifiedRiskPrediction {
  riskCategory: RiskCategory;
  riskScore: number; // 0.00 to 1.00
  confidence: number;
  contributingFactors: string[];
  recommendation: string;
  classesNeededToRecover: number;
  classesCanBeMissed: number;
  decisionTreePath: Array<{
    condition: string;
    evaluatedValue: number | string;
    passed: boolean;
  }>;
  featureAttributions: FeatureAttribution[];
  modelVersion: string;
}

export function predictAttendanceRisk(
  chronologicalStatuses: AttendanceStatus[],
  threshold = 75
): UnifiedRiskPrediction {
  // 1. Extract feature vector using DSA algorithms
  const features = extractAttendanceFeatures(chronologicalStatuses, threshold);

  // 2. Evaluate with Hierarchical Decision Tree (Structural DSA)
  const tree = new RiskDecisionTree(threshold);
  const treeResult = tree.evaluate({
    attendancePercentage: features.overallPercentage,
    recentPercentage: features.recentPercentage,
    consecutiveAbsences: features.consecutiveAbsences,
    trendSlope: features.trendSlope,
    totalConducted: features.totalConducted,
    threshold,
  });

  // 3. Evaluate with Logistic Regression (Statistical ML)
  const mlResult = runLogisticRegression(features);

  // 4. Compute Recovery Metrics (Mathematical optimization)
  const presentSessions = Math.round(
    (features.overallPercentage / 100) * features.totalConducted
  );
  const recovery = calculateRecovery(presentSessions, features.totalConducted, threshold);

  // 5. Build Human-Readable Contributing Factors
  const factors: string[] = [];

  if (features.overallPercentage < threshold) {
    factors.push(
      `Attendance is at ${features.overallPercentage}%, which is ${features.thresholdGap}% below the required ${threshold}% eligibility threshold.`
    );
  } else {
    factors.push(
      `Attendance meets the required threshold (${features.overallPercentage}% vs ${threshold}% required).`
    );
  }

  if (features.consecutiveAbsences >= 2) {
    factors.push(
      `Detected ${features.consecutiveAbsences} consecutive absences in recent lectures.`
    );
  }

  if (features.trendSlope < -0.05) {
    factors.push("Attendance trend is actively declining over the recent session sequence.");
  } else if (features.trendSlope > 0.05) {
    factors.push("Attendance trend shows recent positive recovery.");
  }

  if (features.recentAbsenceFrequency >= 0.4) {
    factors.push(
      `High recent absence density: missed ${Math.round(features.recentAbsenceFrequency * 5)} of the last 5 sessions.`
    );
  }

  // 6. Generate Actionable Recommendation
  let recommendation = "";
  if (recovery.isBelowThreshold) {
    recommendation = `Must attend the next ${recovery.classesNeededToRecover} consecutive classes without absence to restore attendance above ${threshold}%.`;
  } else if (recovery.classesCanBeMissed <= 1) {
    recommendation = `Caution: Attendance buffer is thin. Missing even 2 more classes will drop you below ${threshold}%.`;
  } else {
    recommendation = `Maintain consistent attendance. You have a safe buffer of up to ${recovery.classesCanBeMissed} missable class${recovery.classesCanBeMissed === 1 ? "" : "es"}.`;
  }

  // 7. Ensemble Risk Score (Combined weighted average)
  const blendedScore = Number(
    (0.6 * mlResult.riskProbability + 0.4 * treeResult.riskScore).toFixed(3)
  );

  // Take the more severe category between Tree and Logistic Regression for safety
  const severityRank: Record<RiskCategory, number> = {
    LOW: 1,
    MEDIUM: 2,
    HIGH: 3,
    CRITICAL: 4,
  };

  const finalCategory =
    severityRank[treeResult.riskCategory] >= severityRank[mlResult.riskCategory]
      ? treeResult.riskCategory
      : mlResult.riskCategory;

  return {
    riskCategory: finalCategory,
    riskScore: blendedScore,
    confidence: 0.88,
    contributingFactors: factors,
    recommendation,
    classesNeededToRecover: recovery.classesNeededToRecover,
    classesCanBeMissed: recovery.classesCanBeMissed,
    decisionTreePath: treeResult.decisionPath.map((s) => ({
      condition: s.conditionDescription,
      evaluatedValue: s.evaluatedValue,
      passed: s.result,
    })),
    featureAttributions: mlResult.attributions,
    modelVersion: "Ensemble-DT-LogReg-v1.2",
  };
}
