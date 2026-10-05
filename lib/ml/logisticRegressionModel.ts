/**
 * ML: LOGISTIC REGRESSION CLASSIFICATION MODEL
 * 
 * Mathematical Formulation:
 * Logit z = β₀ + β₁·x₁ + β₂·x₂ + ... + βₖ·xₖ
 * Probability P(Risk = 1) = σ(z) = 1 / (1 + e^(-z))
 * 
 * Feature Weights (Calibrated for academic attendance risk):
 * - Threshold Gap (points below 75%): β = +0.12 (strongest risk driver)
 * - Consecutive Absences: β = +0.65 (each consecutive missed class rapidly spikes risk)
 * - Trend Slope (negative = declining): β = -4.50 (steeper downward slope multiplies probability)
 * - Recent Absence Frequency: β = +1.80 (elevated recent absences)
 * - Bias / Intercept: β₀ = -1.80 (baseline prior for a regular student)
 * 
 * Viva Explainability:
 * - Why Logistic Regression?
 *   1. Bounded output in [0, 1] representing a genuine mathematical probability.
 *   2. Highly explainable: The log-odds linear combination enables exact feature attribution
 *      (e.g., "The 3 consecutive absences contributed +0.19 to the risk probability").
 *   3. No black-box opacity.
 */

import { AttendanceFeatureVector } from "./featureEngineering";

export interface FeatureAttribution {
  feature: string;
  label: string;
  rawValue: number | string;
  weight: number;
  contributionLogOdds: number;
  impact: "INCREASES_RISK" | "NEUTRAL" | "REDUCES_RISK";
  explanation: string;
}

export interface LogisticRegressionOutput {
  modelName: "LogisticRegression";
  linearLogit: number;
  riskProbability: number; // 0.0 to 1.0
  riskCategory: "LOW" | "MEDIUM" | "HIGH" | "CRITICAL";
  attributions: FeatureAttribution[];
}

export function runLogisticRegression(
  features: AttendanceFeatureVector
): LogisticRegressionOutput {
  // Model weights (calibrated parameters)
  const bias = -1.8;
  const wThresholdGap = 0.12;
  const wConsecutive = 0.65;
  const wTrendSlope = -4.5; // Negative slope increases z
  const wRecentAbsenceFreq = 1.8;

  // Contributions
  const cGap = features.thresholdGap * wThresholdGap;
  const cConsecutive = features.consecutiveAbsences * wConsecutive;
  const cTrend = features.trendSlope * wTrendSlope;
  const cRecentFreq = features.recentAbsenceFrequency * wRecentAbsenceFreq;

  const z = bias + cGap + cConsecutive + cTrend + cRecentFreq;
  const probability = 1 / (1 + Math.exp(-z));
  const roundedProb = Number(Math.max(0.01, Math.min(0.99, probability)).toFixed(3));

  // Determine category based on probability and threshold
  let riskCategory: "LOW" | "MEDIUM" | "HIGH" | "CRITICAL" = "LOW";
  if (roundedProb >= 0.75 || features.overallPercentage < 60) {
    riskCategory = "CRITICAL";
  } else if (roundedProb >= 0.50 || features.overallPercentage < 75) {
    riskCategory = "HIGH";
  } else if (roundedProb >= 0.28 || features.overallPercentage < 80) {
    riskCategory = "MEDIUM";
  } else {
    riskCategory = "LOW";
  }

  const attributions: FeatureAttribution[] = [
    {
      feature: "overallPercentage",
      label: "Attendance vs Threshold (75%)",
      rawValue: `${features.overallPercentage}% (${features.thresholdGap > 0 ? `-${features.thresholdGap}% deficit` : `+${Math.abs(features.thresholdGap)}% buffer`})`,
      weight: wThresholdGap,
      contributionLogOdds: Number(cGap.toFixed(3)),
      impact: cGap > 0.1 ? "INCREASES_RISK" : cGap < -0.1 ? "REDUCES_RISK" : "NEUTRAL",
      explanation:
        features.thresholdGap > 0
          ? `Current attendance is ${features.thresholdGap}% below the required 75% threshold.`
          : `Current attendance is safely above the required threshold.`,
    },
    {
      feature: "consecutiveAbsences",
      label: "Consecutive Absences",
      rawValue: `${features.consecutiveAbsences} class${features.consecutiveAbsences === 1 ? "" : "es"}`,
      weight: wConsecutive,
      contributionLogOdds: Number(cConsecutive.toFixed(3)),
      impact: cConsecutive > 0.4 ? "INCREASES_RISK" : "NEUTRAL",
      explanation:
        features.consecutiveAbsences >= 2
          ? `Active streak of ${features.consecutiveAbsences} missed lectures signals disengagement.`
          : `No significant consecutive absence streak detected.`,
    },
    {
      feature: "trendSlope",
      label: "Attendance Momentum / Trend",
      rawValue: `Slope: ${features.trendSlope > 0 ? "+" : ""}${features.trendSlope}`,
      weight: wTrendSlope,
      contributionLogOdds: Number(cTrend.toFixed(3)),
      impact: cTrend > 0.2 ? "INCREASES_RISK" : cTrend < -0.2 ? "REDUCES_RISK" : "NEUTRAL",
      explanation:
        features.trendSlope < -0.05
          ? `Attendance trajectory has a negative downward trend.`
          : features.trendSlope > 0.05
          ? `Attendance has shown positive recovery in recent sessions.`
          : `Attendance trend is steady.`,
    },
    {
      feature: "recentAbsenceFrequency",
      label: "Recent Absence Density",
      rawValue: `${Math.round(features.recentAbsenceFrequency * 100)}% of last 5 classes`,
      weight: wRecentAbsenceFreq,
      contributionLogOdds: Number(cRecentFreq.toFixed(3)),
      impact: cRecentFreq > 0.3 ? "INCREASES_RISK" : "NEUTRAL",
      explanation:
        features.recentAbsenceFrequency > 0.4
          ? `High cluster of missed lectures in the recent 5-session window.`
          : `Recent attendance density is within normal parameters.`,
    },
  ];

  return {
    modelName: "LogisticRegression",
    linearLogit: Number(z.toFixed(3)),
    riskProbability: roundedProb,
    riskCategory,
    attributions,
  };
}
