#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "models/Models.hpp"
#include "dsa/RiskDecisionTree.hpp"
#include "ml/FeatureEngineering.hpp"
#include "ml/LogisticRegressionModel.hpp"
#include "algorithms/RecoveryCalculator.hpp"

namespace attendance::ml {

/**
 * ARCHITECTURAL NOTE ON RISK PREDICTION:
 * The risk prediction engine combines a fixed-weight logistic scoring model (60%)
 * with a hierarchical decision tree (40%). This architecture provides transparent,
 * deterministic, and explainable risk scores with explicit factor attributions.
 * It is not trained via gradient descent on labeled historical datasets.
 */

struct RiskModelConfig {
    double logisticWeight{0.60};
    double decisionTreeWeight{0.40};
    double criticalScoreCutoff{0.82};
    double highScoreCutoff{0.58};
    double mediumScoreCutoff{0.35};
    int minSessionsForFullConfidence{20};
};

struct RiskPredictionResult {
    attendance::models::RiskLevel riskCategory{attendance::models::RiskLevel::LOW};
    double riskScore{0.0}; // 0.0 - 1.0 blended risk probability
    double confidence{0.5}; // Calculated based on sample size
    std::vector<std::string> contributingFactors;
    std::string recommendation;
    std::string modelVersion{"rule-ensemble-scoring-v2.1"};
    std::string methodology{"Fixed-weight Logistic Scoring + Hierarchical Decision Tree Ensemble"};
    nlohmann::json telemetry;
};

class RiskPredictor {
public:
    static RiskPredictionResult predict(
        const std::vector<attendance::models::AttendanceStatus>& records,
        double threshold = 75.0,
        const RiskModelConfig& config = RiskModelConfig()) 
    {
        RiskPredictionResult result;
        result.modelVersion = "rule-ensemble-scoring-v2.1";
        result.methodology = "Fixed-weight Logistic Scoring + Hierarchical Decision Tree Ensemble";

        // 1. Feature Extraction
        auto features = extractFeatures(records, threshold);

        // Calculate sample-size-based statistical confidence
        if (features.totalConducted == 0) {
            result.confidence = 0.20;
            result.riskCategory = attendance::models::RiskLevel::LOW;
            result.riskScore = 0.0;
            result.contributingFactors.push_back("No classes conducted yet; baseline default risk assigned.");
            result.recommendation = "Begin attending classes regularly once sessions commence.";
            return result;
        }

        double sessionRatio = static_cast<double>(std::min(features.totalConducted, config.minSessionsForFullConfidence)) / 
                              static_cast<double>(config.minSessionsForFullConfidence);
        result.confidence = std::round((0.40 + 0.55 * sessionRatio) * 100.0) / 100.0;

        // 2. Decision Tree Evaluation
        attendance::dsa::RiskDecisionTree tree(threshold);
        attendance::dsa::EvaluationFeatureInput treeInput{
            features.overallAttendancePct,
            features.recentAttendancePct,
            features.consecutiveAbsences,
            features.trendSlope,
            features.totalConducted,
            threshold
        };
        auto treeResult = tree.classify(treeInput);

        // 3. Logistic Scoring Evaluation
        auto lrResult = LogisticRegressionModel::predict(features, threshold);

        // 4. Ensemble Voting & Probability Blending
        double ensembleScore = (lrResult.probability * config.logisticWeight) + 
                               (treeResult.riskScore * config.decisionTreeWeight);
        ensembleScore = std::max(0.0, std::min(1.0, ensembleScore));
        ensembleScore = std::round(ensembleScore * 1000.0) / 1000.0;
        result.riskScore = ensembleScore;

        // Categorization based on threshold bounds and decision tree
        if (treeResult.riskCategory == attendance::models::RiskLevel::CRITICAL || 
            ensembleScore >= config.criticalScoreCutoff || 
            features.overallAttendancePct < 50.0) 
        {
            result.riskCategory = attendance::models::RiskLevel::CRITICAL;
        } else if (treeResult.riskCategory == attendance::models::RiskLevel::HIGH || 
                   ensembleScore >= config.highScoreCutoff || 
                   features.overallAttendancePct < threshold - 5.0) 
        {
            result.riskCategory = attendance::models::RiskLevel::HIGH;
        } else if (treeResult.riskCategory == attendance::models::RiskLevel::MEDIUM || 
                   ensembleScore >= config.mediumScoreCutoff || 
                   features.overallAttendancePct < threshold) 
        {
            result.riskCategory = attendance::models::RiskLevel::MEDIUM;
        } else {
            result.riskCategory = attendance::models::RiskLevel::LOW;
        }

        // 5. Explainable AI: Factor Attributions
        std::vector<std::string> factors;

        if (features.overallAttendancePct < threshold) {
            double deficit = threshold - features.overallAttendancePct;
            std::stringstream ss;
            ss << "Attendance is " << std::fixed << std::setprecision(1) << features.overallAttendancePct 
               << "% (" << deficit << "% deficit below mandatory " << threshold << "% threshold)";
            factors.push_back(ss.str());
        } else {
            std::stringstream ss;
            ss << "Overall attendance (" << std::fixed << std::setprecision(1) << features.overallAttendancePct 
               << "%) satisfies institutional requirement of " << threshold << "%";
            factors.push_back(ss.str());
        }

        if (features.consecutiveAbsences >= 2) {
            factors.push_back("Active consecutive absence streak of " + 
                              std::to_string(features.consecutiveAbsences) + " sessions detected");
        }

        if (features.recentAttendancePct < features.overallAttendancePct - 15.0) {
            std::stringstream ss;
            ss << "Sharp short-term momentum drop: recent 5 sessions average is " 
               << std::fixed << std::setprecision(1) << features.recentAttendancePct << "%";
            factors.push_back(ss.str());
        }

        if (features.trendSlope < -0.04) {
            factors.push_back("Downward regression trend: absence frequency is accelerating over time");
        }

        if (features.totalConducted < 5) {
            factors.push_back("Small sample size (" + std::to_string(features.totalConducted) + 
                              " sessions); estimates will stabilize with more conducted lectures.");
        }

        result.contributingFactors = factors;

        // 6. Actionable Recovery Recommendations
        double attendedEst = (features.overallAttendancePct / 100.0) * features.totalConducted;
        auto recovery = attendance::algorithms::calculateRecovery(
            attendedEst, features.totalConducted, threshold
        );

        if (recovery.isBelowThreshold) {
            result.recommendation = "Action Required: Must attend the next " + 
                std::to_string(recovery.classesNeededToRecover) + 
                " consecutive lectures without absence to restore eligibility to " + 
                std::to_string(static_cast<int>(threshold)) + "%.";
        } else {
            if (recovery.classesCanBeMissed > 0) {
                result.recommendation = "Safe Standing: Attendance meets guidelines. Up to " + 
                    std::to_string(recovery.classesCanBeMissed) + 
                    " additional lecture(s) can be missed before falling below " + 
                    std::to_string(static_cast<int>(threshold)) + "%.";
            } else {
                result.recommendation = "Borderline Standing: Currently at or near threshold. Any absence will result in attendance shortage.";
            }
        }

        // Telemetry details
        result.telemetry = nlohmann::json{
            {"overallAttendancePct", features.overallAttendancePct},
            {"recentAttendancePct", features.recentAttendancePct},
            {"consecutiveAbsences", features.consecutiveAbsences},
            {"totalConducted", features.totalConducted},
            {"trendSlope", features.trendSlope},
            {"decisionPathLength", treeResult.decisionPath.size()},
            {"logisticProbability", lrResult.probability},
            {"classesNeededToRecover", recovery.classesNeededToRecover},
            {"classesCanBeMissed", recovery.classesCanBeMissed}
        };

        return result;
    }
};

} // namespace attendance::ml
