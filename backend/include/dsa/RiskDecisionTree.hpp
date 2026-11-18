#pragma once

#include <string>
#include <vector>
#include <memory>
#include "models/Models.hpp"
#include "json.hpp"

namespace attendance::dsa {

struct TreeDecisionPathStep {
    std::string nodeId;
    std::string conditionDescription;
    double evaluatedValue;
    bool result;
};

struct TreeClassificationResult {
    attendance::models::RiskLevel riskCategory;
    double riskScore;
    std::string explanation;
    std::vector<TreeDecisionPathStep> decisionPath;
};

struct EvaluationFeatureInput {
    double attendancePercentage;
    double recentPercentage;
    int consecutiveAbsences;
    double trendSlope;
    int totalConducted;
    double threshold{75.0};
};

struct DecisionTreeNode {
    std::string id;
    std::string feature;
    std::string condition;
    std::string op; // "<=", ">", etc.
    double splitValue{0.0};

    // If leaf node
    bool isLeaf{false};
    attendance::models::RiskLevel riskCategory{attendance::models::RiskLevel::LOW};
    double riskScore{0.0};
    std::string explanation;

    std::shared_ptr<DecisionTreeNode> left{nullptr};
    std::shared_ptr<DecisionTreeNode> right{nullptr};
};

class RiskDecisionTree {
public:
    explicit RiskDecisionTree(double threshold = 75.0) {
        root_ = buildTree(threshold);
    }

    TreeClassificationResult classify(const EvaluationFeatureInput& input) const {
        std::vector<TreeDecisionPathStep> path;
        auto current = root_;

        while (current && !current->isLeaf) {
            double featureValue = 0.0;
            if (current->feature == "attendancePercentage") {
                featureValue = input.attendancePercentage;
            } else if (current->feature == "consecutiveAbsences") {
                featureValue = static_cast<double>(input.consecutiveAbsences);
            } else if (current->feature == "trendSlope") {
                featureValue = input.trendSlope;
            } else if (current->feature == "recentPercentage") {
                featureValue = input.recentPercentage;
            }

            bool conditionMet = false;
            if (current->op == "<=") {
                conditionMet = featureValue <= current->splitValue;
            } else if (current->op == ">") {
                conditionMet = featureValue > current->splitValue;
            } else if (current->op == ">=") {
                conditionMet = featureValue >= current->splitValue;
            }

            path.push_back({
                current->id,
                current->condition,
                featureValue,
                conditionMet
            });

            current = conditionMet ? current->left : current->right;
        }

        if (current && current->isLeaf) {
            return {
                current->riskCategory,
                current->riskScore,
                current->explanation,
                path
            };
        }

        return {
            attendance::models::RiskLevel::LOW,
            0.05,
            "Default evaluation: safe institutional baseline.",
            path
        };
    }

private:
    std::shared_ptr<DecisionTreeNode> buildTree(double threshold) {
        auto root = std::make_shared<DecisionTreeNode>();
        root->id = "node_root";
        root->feature = "attendancePercentage";
        root->condition = "Overall Attendance <= " + std::to_string(static_cast<int>(threshold)) + "%";
        root->op = "<=";
        root->splitValue = threshold;

        // Left branch: Attendance <= threshold (Defaulter / At Risk)
        auto belowThreshold = std::make_shared<DecisionTreeNode>();
        belowThreshold->id = "node_below_threshold";
        belowThreshold->feature = "attendancePercentage";
        belowThreshold->condition = "Overall Attendance <= " + std::to_string(static_cast<int>(threshold - 15)) + "% (Severely Low)";
        belowThreshold->op = "<=";
        belowThreshold->splitValue = threshold - 15.0; // e.g. 60%
        root->left = belowThreshold;

        // Very low (< 60%):
        auto criticalCheck = std::make_shared<DecisionTreeNode>();
        criticalCheck->id = "node_critical_check";
        criticalCheck->feature = "consecutiveAbsences";
        criticalCheck->condition = "Consecutive Absences >= 3";
        criticalCheck->op = ">=";
        criticalCheck->splitValue = 3.0;
        belowThreshold->left = criticalCheck;

        auto leafCritical = std::make_shared<DecisionTreeNode>();
        leafCritical->isLeaf = true;
        leafCritical->id = "leaf_critical_severe";
        leafCritical->riskCategory = attendance::models::RiskLevel::CRITICAL;
        leafCritical->riskScore = 0.95;
        leafCritical->explanation = "Attendance is critically below requirement with an active severe absence streak.";
        criticalCheck->left = leafCritical;

        auto leafHighSevere = std::make_shared<DecisionTreeNode>();
        leafHighSevere->isLeaf = true;
        leafHighSevere->id = "leaf_high_severe";
        leafHighSevere->riskCategory = attendance::models::RiskLevel::HIGH;
        leafHighSevere->riskScore = 0.82;
        leafHighSevere->explanation = "Attendance is severely below threshold, though no immediate 3+ consecutive absence streak is active.";
        criticalCheck->right = leafHighSevere;

        // Borderline low (60% to 75%):
        auto trendCheckLow = std::make_shared<DecisionTreeNode>();
        trendCheckLow->id = "node_trend_check_low";
        trendCheckLow->feature = "trendSlope";
        trendCheckLow->condition = "Attendance Trend is Declining (slope <= -0.05)";
        trendCheckLow->op = "<=";
        trendCheckLow->splitValue = -0.05;
        belowThreshold->right = trendCheckLow;

        auto leafHighDeclining = std::make_shared<DecisionTreeNode>();
        leafHighDeclining->isLeaf = true;
        leafHighDeclining->id = "leaf_high_declining";
        leafHighDeclining->riskCategory = attendance::models::RiskLevel::HIGH;
        leafHighDeclining->riskScore = 0.74;
        leafHighDeclining->explanation = "Attendance is currently below the required threshold and is actively declining.";
        trendCheckLow->left = leafHighDeclining;

        auto streakCheckLow = std::make_shared<DecisionTreeNode>();
        streakCheckLow->id = "node_streak_check_low";
        streakCheckLow->feature = "consecutiveAbsences";
        streakCheckLow->condition = "Consecutive Absences >= 3";
        streakCheckLow->op = ">=";
        streakCheckLow->splitValue = 3.0;
        trendCheckLow->right = streakCheckLow;

        auto leafHighStreak = std::make_shared<DecisionTreeNode>();
        leafHighStreak->isLeaf = true;
        leafHighStreak->id = "leaf_high_streak";
        leafHighStreak->riskCategory = attendance::models::RiskLevel::HIGH;
        leafHighStreak->riskScore = 0.70;
        leafHighStreak->explanation = "Attendance is borderline low with an active multi-session absence streak.";
        streakCheckLow->left = leafHighStreak;

        auto leafMediumLow = std::make_shared<DecisionTreeNode>();
        leafMediumLow->isLeaf = true;
        leafMediumLow->id = "leaf_medium_low";
        leafMediumLow->riskCategory = attendance::models::RiskLevel::MEDIUM;
        leafMediumLow->riskScore = 0.50;
        leafMediumLow->explanation = "Attendance is currently below threshold but stable without acute recent streak.";
        streakCheckLow->right = leafMediumLow;

        // Right branch: Attendance > threshold (Safe baseline)
        auto aboveThreshold = std::make_shared<DecisionTreeNode>();
        aboveThreshold->id = "node_streak_check_safe";
        aboveThreshold->feature = "consecutiveAbsences";
        aboveThreshold->condition = "Active Consecutive Absences >= 3 (Sudden Drop Alert)";
        aboveThreshold->op = ">=";
        aboveThreshold->splitValue = 3.0;
        root->right = aboveThreshold;

        auto leafMediumSudden = std::make_shared<DecisionTreeNode>();
        leafMediumSudden->isLeaf = true;
        leafMediumSudden->id = "leaf_medium_sudden";
        leafMediumSudden->riskCategory = attendance::models::RiskLevel::MEDIUM;
        leafMediumSudden->riskScore = 0.45;
        leafMediumSudden->explanation = "Overall percentage meets requirements, but a sudden consecutive absence streak indicates emerging risk.";
        aboveThreshold->left = leafMediumSudden;

        auto safeTrendCheck = std::make_shared<DecisionTreeNode>();
        safeTrendCheck->id = "node_trend_check_safe";
        safeTrendCheck->feature = "trendSlope";
        safeTrendCheck->condition = "Steep Negative Trend (slope <= -0.08)";
        safeTrendCheck->op = "<=";
        safeTrendCheck->splitValue = -0.08;
        aboveThreshold->right = safeTrendCheck;

        auto leafMediumTrend = std::make_shared<DecisionTreeNode>();
        leafMediumTrend->isLeaf = true;
        leafMediumTrend->id = "leaf_medium_trend";
        leafMediumTrend->riskCategory = attendance::models::RiskLevel::MEDIUM;
        leafMediumTrend->riskScore = 0.35;
        leafMediumTrend->explanation = "Currently above threshold, but experiencing steep downward momentum.";
        safeTrendCheck->left = leafMediumTrend;

        auto leafLowSafe = std::make_shared<DecisionTreeNode>();
        leafLowSafe->isLeaf = true;
        leafLowSafe->id = "leaf_low_safe";
        leafLowSafe->riskCategory = attendance::models::RiskLevel::LOW;
        leafLowSafe->riskScore = 0.05;
        leafLowSafe->explanation = "Healthy attendance pattern with regular attendance and consistent trajectory.";
        safeTrendCheck->right = leafLowSafe;

        return root;
    }

    std::shared_ptr<DecisionTreeNode> root_;
};

} // namespace attendance::dsa
