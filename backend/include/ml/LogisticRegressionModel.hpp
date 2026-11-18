#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "ml/FeatureEngineering.hpp"

namespace attendance::ml {

struct FeatureContribution {
    std::string featureName;
    double featureValue;
    double weight;
    double logOddsImpact;
    double oddsRatio;
    std::string description;
};

struct LogisticRegressionOutput {
    double probability; // 0.0 - 1.0
    double logitZ;
    std::vector<FeatureContribution> contributions;
};

class LogisticRegressionModel {
public:
    static constexpr double INTERCEPT = -1.2;
    static constexpr double WEIGHT_DEFICIT = 4.8;
    static constexpr double WEIGHT_CONSECUTIVE = 0.65;
    static constexpr double WEIGHT_RECENT_ABSENCES = 2.1;
    static constexpr double WEIGHT_DECLINING_TREND = 1.4;

    static LogisticRegressionOutput predict(const StudentAttendanceFeatures& feat, double threshold = 75.0) {
        LogisticRegressionOutput output;
        output.contributions.clear();

        // Normalized feature values
        double normDeficit = feat.thresholdGap / 100.0;
        double normConsec = std::min(10.0, static_cast<double>(feat.consecutiveAbsences));
        double normRecent = feat.recentAbsenceFrequency;
        double normTrend = (feat.trendSlope < -0.04) ? 1.0 : 0.0;

        double z = INTERCEPT;

        // Deficit contribution
        double impactDeficit = WEIGHT_DEFICIT * normDeficit;
        z += impactDeficit;
        if (normDeficit > 0.0) {
            output.contributions.push_back({
                "Attendance Deficit",
                feat.thresholdGap,
                WEIGHT_DEFICIT,
                impactDeficit,
                std::exp(impactDeficit),
                "Attendance is " + std::to_string(static_cast<int>(feat.thresholdGap)) + "% below target threshold."
            });
        }

        // Consecutive absences contribution
        double impactConsec = WEIGHT_CONSECUTIVE * normConsec;
        z += impactConsec;
        if (feat.consecutiveAbsences > 0) {
            output.contributions.push_back({
                "Active Absence Streak",
                static_cast<double>(feat.consecutiveAbsences),
                WEIGHT_CONSECUTIVE,
                impactConsec,
                std::exp(impactConsec),
                std::to_string(feat.consecutiveAbsences) + " back-to-back missed sessions."
            });
        }

        // Recent absences contribution
        double impactRecent = WEIGHT_RECENT_ABSENCES * normRecent;
        z += impactRecent;
        if (normRecent > 0.2) {
            output.contributions.push_back({
                "Recent Window Absence Density",
                normRecent,
                WEIGHT_RECENT_ABSENCES,
                impactRecent,
                std::exp(impactRecent),
                "Elevated recent missed lecture frequency (" + std::to_string(static_cast<int>(normRecent * 100)) + "%)."
            });
        }

        // Trend contribution
        double impactTrend = WEIGHT_DECLINING_TREND * normTrend;
        z += impactTrend;
        if (normTrend > 0.0) {
            output.contributions.push_back({
                "Declining Momentum",
                feat.trendSlope,
                WEIGHT_DECLINING_TREND,
                impactTrend,
                std::exp(impactTrend),
                "Statistically negative OLS trajectory (slope: " + std::to_string(feat.trendSlope) + ")."
            });
        }

        output.logitZ = z;
        // Sigmoid activation: 1 / (1 + exp(-z))
        output.probability = 1.0 / (1.0 + std::exp(-z));
        output.probability = std::round(output.probability * 1000.0) / 1000.0;

        return output;
    }
};

} // namespace attendance::ml
