/**
 * DSA: TREE (HIERARCHICAL DECISION TREE)
 * 
 * Purpose:
 * Provides deterministic, explainable risk classification logic.
 * Every prediction traces a root-to-leaf path through binary/n-ary decision nodes
 * evaluating attendance thresholds, recent trends, and absence streaks.
 * 
 * Viva Explainability:
 * - Structure: Hierarchical Binary/Branching Tree
 * - Traversal Complexity: O(h) where h is the tree height. Since the tree is balanced with depth ~4,
 *   classification runs in O(1) bounded steps.
 * - Relation to ML: Mirrors a supervised Machine Learning Decision Tree (CART/ID3) with explicit
 *   threshold split points, ensuring 100% white-box explainability (no black box).
 */

import { DecisionTreeNode, RiskCategory } from "../types";

export interface EvaluationFeatureInput {
  attendancePercentage: number;
  recentPercentage: number;
  consecutiveAbsences: number;
  trendSlope: number;
  totalConducted: number;
  threshold: number; // default 75%
}

export interface TreeDecisionPathStep {
  nodeId: string;
  conditionDescription: string;
  evaluatedValue: number | string;
  result: boolean;
}

export interface TreeClassificationResult {
  riskCategory: RiskCategory;
  riskScore: number;
  explanation: string;
  decisionPath: TreeDecisionPathStep[];
}

export class RiskDecisionTree {
  private root: DecisionTreeNode;

  constructor(defaultThreshold = 75) {
    this.root = this.buildTree(defaultThreshold);
  }

  /**
   * Constructs the hierarchical decision tree structure based on academic thresholds.
   */
  public buildTree(threshold: number): DecisionTreeNode {
    // Root Node: Primary check against configurable eligibility threshold (75%)
    return {
      id: "node_root",
      feature: "attendancePercentage",
      condition: `Overall Attendance < ${threshold}%`,
      operator: "<=",
      threshold: threshold,
      // Left Branch: Attendance <= Threshold (At Risk)
      left: {
        id: "node_below_threshold",
        feature: "attendancePercentage",
        condition: `Overall Attendance < ${threshold - 15}% (Severely low)`,
        operator: "<=",
        threshold: threshold - 15, // e.g. < 60%
        // Left-Left: Very Low (< 60%)
        left: {
          id: "node_critical_check",
          feature: "consecutiveAbsences",
          condition: "Consecutive Absences >= 3",
          operator: ">",
          threshold: 2,
          left: {
            id: "leaf_critical_severe",
            riskCategory: "CRITICAL",
            riskScore: 0.95,
            explanation: `Attendance is critically below requirement (${threshold}%) with an active severe absence streak. Immediate intervention needed.`,
          },
          right: {
            id: "leaf_high_severe",
            riskCategory: "HIGH",
            riskScore: 0.82,
            explanation: `Attendance is severely below threshold (${threshold}%), though no immediate consecutive absence streak is active.`,
          },
        },
        // Left-Right: Borderline Low (60% to 75%)
        right: {
          id: "node_trend_check_low",
          feature: "trendSlope",
          condition: "Attendance Trend is Declining (slope < -0.05)",
          operator: "<=",
          threshold: -0.05,
          left: {
            id: "leaf_high_declining",
            riskCategory: "HIGH",
            riskScore: 0.74,
            explanation: `Attendance is currently below the required ${threshold}% and is actively declining in recent sessions.`,
          },
          right: {
            id: "node_streak_check_low",
            feature: "consecutiveAbsences",
            condition: "Consecutive Absences >= 2",
            operator: ">",
            threshold: 1,
            left: {
              id: "leaf_high_streak",
              riskCategory: "HIGH",
              riskScore: 0.68,
              explanation: `Attendance is marginally below ${threshold}% with ongoing consecutive missed lectures.`,
            },
            right: {
              id: "leaf_medium_borderline",
              riskCategory: "MEDIUM",
              riskScore: 0.48,
              explanation: `Attendance is slightly under ${threshold}%, but the trend is stable or recovering.`,
            },
          },
        },
      },
      // Right Branch: Attendance > Threshold (Safe / Moderately Safe)
      right: {
        id: "node_above_threshold",
        feature: "attendancePercentage",
        condition: `Overall Attendance >= ${threshold + 10}% (Safe margin)`,
        operator: ">",
        threshold: threshold + 10, // e.g. >= 85%
        // High buffer: >= 85%
        left: {
          id: "node_streak_safe",
          feature: "consecutiveAbsences",
          condition: "Consecutive Absences >= 3",
          operator: ">",
          threshold: 2,
          left: {
            id: "leaf_medium_sudden_drop",
            riskCategory: "MEDIUM",
            riskScore: 0.38,
            explanation: `Overall attendance is high, but a sudden streak of 3+ consecutive absences triggers an early warning.`,
          },
          right: {
            id: "leaf_low_safe",
            riskCategory: "LOW",
            riskScore: 0.08,
            explanation: `Excellent attendance well above the ${threshold}% requirement with stable participation.`,
          },
        },
        // Buffer margin is thin: 75% to 85%
        right: {
          id: "node_thin_margin_trend",
          feature: "trendSlope",
          condition: "Attendance Trend is Declining",
          operator: "<=",
          threshold: -0.05,
          left: {
            id: "leaf_medium_thin_margin",
            riskCategory: "MEDIUM",
            riskScore: 0.45,
            explanation: `Attendance satisfies ${threshold}%, but is trending downward and risks falling below eligibility if unchecked.`,
          },
          right: {
            id: "leaf_low_satisfactory",
            riskCategory: "LOW",
            riskScore: 0.22,
            explanation: `Satisfies the ${threshold}% attendance requirement with consistent recent attendance.`,
          },
        },
      },
    };
  }

  /**
   * Evaluates a student against the tree and records the exact traversal path.
   * Time Complexity: O(h) where h <= 5 -> O(1)
   */
  public evaluate(input: EvaluationFeatureInput): TreeClassificationResult {
    const path: TreeDecisionPathStep[] = [];
    let current: DecisionTreeNode | undefined = this.root;

    while (current && !current.riskCategory) {
      const val = this.getFeatureValue(current.feature, input);
      const isLeft = this.evaluateCondition(val, current.operator, current.threshold);

      path.push({
        nodeId: current.id,
        conditionDescription: current.condition || "",
        evaluatedValue: val,
        result: isLeft,
      });

      current = isLeft ? current.left : current.right;
    }

    if (!current || !current.riskCategory) {
      return {
        riskCategory: "MEDIUM",
        riskScore: 0.5,
        explanation: "Default evaluation path reached.",
        decisionPath: path,
      };
    }

    return {
      riskCategory: current.riskCategory,
      riskScore: current.riskScore ?? 0.5,
      explanation: current.explanation ?? "",
      decisionPath: path,
    };
  }

  private getFeatureValue(feature: string | undefined, input: EvaluationFeatureInput): number {
    switch (feature) {
      case "attendancePercentage":
        return input.attendancePercentage;
      case "recentPercentage":
        return input.recentPercentage;
      case "consecutiveAbsences":
        return input.consecutiveAbsences;
      case "trendSlope":
        return input.trendSlope;
      default:
        return 0;
    }
  }

  private evaluateCondition(val: number, op: "<=" | ">" | "==" | undefined, threshold: number | undefined): boolean {
    if (threshold === undefined) return false;
    switch (op) {
      case "<=":
        return val <= threshold;
      case ">":
        return val > threshold;
      case "==":
        return val === threshold;
      default:
        return false;
    }
  }

  public getRootNode(): DecisionTreeNode {
    return this.root;
  }
}
