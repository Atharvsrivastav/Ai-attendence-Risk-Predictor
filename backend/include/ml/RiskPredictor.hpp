#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include "models/Models.hpp"
#include "dsa/RiskDecisionTree.hpp"
#include "ml/FeatureEngineering.hpp"
#include "ml/LogisticRegressionModel.hpp"
#include "algorithms/RecoveryCalculator.hpp"

namespace attendance::ml {

struct RiskPredictionResult {
    attendance::models::RiskLevel riskCategory;
    double riskScore; // 0.0 - 1.0 probability
    double confidence;
    std::vector<std::string> contributingFactors;
    std::string recommendation;
    std::string modelVersion;
    nlohmann::json telemetry;
};

class RiskPredictor {
public:
    static RiskPredictionResult predict(
        const std::vector<attendance::models::AttendanceStatus>& records,
        double threshold = 75.0) 
    {
        RiskPredictionResult result;
        result.modelVersion = "ensemble-dsa-tree-lr-v2.0";
        result.confidence = 0.88;

        // 1. Feature Extraction
        auto features = extractFeatures(records, threshold);

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

        // 3. Logistic Regression Evaluation
        auto lrResult = LogisticRegressionModel::predict(features, threshold);

        // 4. Ensemble Voting & Probability Blending
        // Score blend: 60% Logistic Probability + 40% Tree Node Score
        double ensembleScore = (lrResult.probability * 0.60) + (treeResult.riskScore * 0.40);
        ensembleScore = std::round(ensembleScore * 1000.0) / 1000.0;
        result.riskScore = ensembleScore;

        if (treeResult.riskCategory == attendance::models::RiskLevel::CRITICAL || ensembleScore >= 0.82) {
            result.riskCategory = attendance::models::RiskLevel::CRITICAL;
        } else if (treeResult.riskCategory == attendance::models::RiskLevel::HIGH || ensembleScore >= 0.60) {
            result.riskCategory = attendance::models::RiskLevel::HIGH;
        } else if (treeResult.riskCategory == attendance::models::RiskLevel::MEDIUM || ensembleScore >= 0.30) {
            result.riskCategory = attendance::models::RiskLevel::MEDIUM;
        } else {
            result.riskCategory = attendance::models::RiskLevel::LOW;
        }

        // 5. Explainable Contributing Factors
        std::vector<std::string> factors;

        if (features.overallAttendancePct < threshold) {
            double deficit = threshold - features.overallAttendancePct;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << "Attendance is at " << features.overallAttendancePct << "%, which is "
               << deficit << "% below the required " << threshold << "% eligibility threshold.";
            factors.push_back(ss.str());
        } else {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << "Attendance meets the required threshold (" << features.overallAttendancePct 
               << "% vs " << threshold << "% required).";
            factors.push_back(ss.str());
        }

        if (features.consecutiveAbsences > 0) {
            factors.push_back("Detected " + std::to_string(features.consecutiveAbsences) + 
                              " consecutive absences in recent lectures.");
        }

        if (features.recentAbsenceFrequency >= 0.4) {
            int recentMissed = static_cast<int>(std::round(features.recentAbsenceFrequency * 5.0));
            factors.push_back("High recent absence density: missed " + std::to_string(recentMissed) + 
                              " of the last 5 sessions.");
        }

        if (features.trend == attendance::models::TrendType::DECLINING) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(4)
               << "Negative OLS trajectory slope (" << features.trendSlope 
               << ") indicates deteriorating attendance pace.";
            factors.push_back(ss.str());
        }

        result.contributingFactors = factors;

        // 6. Actionable Recommendation via Recovery Calculator
        auto recovery = attendance::algorithms::calculateRecovery(
            features.totalConducted - features.totalAbsences,
            features.totalConducted,
            threshold
        );

        if (recovery.isBelowThreshold) {
            result.recommendation = "Must attend the next " + 
                std::to_string(recovery.classesNeededToRecover) + 
                " consecutive classes without absence to restore attendance above " + 
                std::to_string(static_cast<int>(threshold)) + "%.";
        } else if (recovery.classesCanBeMissed <= 2) {
            result.recommendation = "Caution: Attendance buffer is thin. Missing even " + 
                std::to_string(recovery.classesCanBeMissed + 1) + 
                " more class(es) will drop you below " + 
                std::to_string(static_cast<int>(threshold)) + "%.";
        } else {
            result.recommendation = "Maintain consistent attendance. You have a safe buffer of up to " + 
                std::to_string(recovery.classesCanBeMissed) + " missable classes.";
        }

        // Telemetry details for DSA Lab & inspector
        result.telemetry = {
            {"decisionTree", {
                {"category", attendance::models::riskToString(treeResult.riskCategory)},
                {"treeScore", treeResult.riskScore},
                {"explanation", treeResult.explanation}
            }},
            {"logisticRegression", {
                {"probability", lrResult.probability},
                {"logitZ", lrResult.logitZ}
            }},
            {"features", {
                {"overallPct", features.overallAttendancePct},
                {"recentPct", features.recentAttendancePct},
                {"consecutiveAbsences", features.consecutiveAbsences},
                {"trendSlope", features.trendSlope}
            }}
        };

        return result;
    }
};

} // namespace attendance::ml
