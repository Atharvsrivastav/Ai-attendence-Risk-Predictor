#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include "models/Models.hpp"
#include "algorithms/AttendanceCalculator.hpp"
#include "algorithms/ConsecutiveAbsences.hpp"
#include "algorithms/RecentWindowAnalysis.hpp"
#include "algorithms/TrendAnalysis.hpp"

namespace attendance::ml {

struct StudentAttendanceFeatures {
    double overallAttendancePct{100.0};
    double recentAttendancePct{100.0};
    int consecutiveAbsences{0};
    int maxConsecutiveAbsences{0};
    int totalAbsences{0};
    int totalConducted{0};
    double recentAbsenceFrequency{0.0};
    double trendSlope{0.0};
    double thresholdGap{0.0}; // threshold - overallPct (positive if below threshold)
    attendance::models::TrendType trend{attendance::models::TrendType::STABLE};
};

inline StudentAttendanceFeatures extractFeatures(
    const std::vector<attendance::models::AttendanceStatus>& records,
    double threshold = 75.0) 
{
    StudentAttendanceFeatures feat;

    // 1. Overall Attendance %
    auto calc = attendance::algorithms::calculateAttendancePercentage(records, threshold);
    feat.overallAttendancePct = calc.percentage;
    feat.totalAbsences = calc.absentCount;
    feat.totalConducted = calc.totalConducted;
    feat.thresholdGap = std::max(0.0, threshold - calc.percentage);

    // 2. Consecutive Absences
    auto consec = attendance::algorithms::detectConsecutiveAbsences(records);
    feat.consecutiveAbsences = consec.currentConsecutive;
    feat.maxConsecutiveAbsences = consec.maxConsecutive;

    // 3. Recent Window Analysis (last 5 sessions)
    auto recent = attendance::algorithms::analyzeRecentAttendanceWindow(records, calc.percentage, 5);
    feat.recentAttendancePct = recent.recentPercentage;
    feat.recentAbsenceFrequency = (recent.windowSizeExamined == 0) ? 0.0 
        : static_cast<double>(recent.recentAbsenceCount) / recent.windowSizeExamined;

    // 4. Trend Analysis
    auto trendRes = attendance::algorithms::analyzeAttendanceTrend(records);
    feat.trendSlope = trendRes.slope;
    feat.trend = trendRes.trend;

    return feat;
}

} // namespace attendance::ml
