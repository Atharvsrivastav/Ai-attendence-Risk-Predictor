#pragma once

#include <vector>
#include <cmath>
#include "models/Models.hpp"

namespace attendance::algorithms {

struct TrendAnalysisResult {
    attendance::models::TrendType trend{attendance::models::TrendType::STABLE};
    double slope{0.0};
    double rSquared{0.0};
    std::string recentDirection;
    bool isDecliningAlert{false};
};

inline TrendAnalysisResult analyzeAttendanceTrend(
    const std::vector<attendance::models::AttendanceStatus>& chronologicalStatuses,
    double slopeThreshold = 0.04) 
{
    TrendAnalysisResult result;
    size_t n = chronologicalStatuses.size();

    if (n < 3) {
        result.trend = attendance::models::TrendType::STABLE;
        result.slope = 0.0;
        result.recentDirection = "Insufficient sessions to establish trend line (< 3).";
        result.isDecliningAlert = false;
        return result;
    }

    double sumX = 0.0;
    double sumY = 0.0;
    double sumXY = 0.0;
    double sumX2 = 0.0;

    for (size_t i = 0; i < n; i++) {
        double x = static_cast<double>(i + 1); // 1-indexed session number
        auto status = chronologicalStatuses[i];
        double y = 1.0;
        if (status == attendance::models::AttendanceStatus::ABSENT) y = 0.0;
        else if (status == attendance::models::AttendanceStatus::LATE) y = 0.5;
        else if (status == attendance::models::AttendanceStatus::EXCUSED) y = 1.0;

        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double denominator = (n * sumX2) - (sumX * sumX);
    double slope = (denominator == 0.0) ? 0.0 : ((n * sumXY) - (sumX * sumY)) / denominator;
    slope = std::round(slope * 10000.0) / 10000.0;

    result.slope = slope;

    if (slope > slopeThreshold) {
        result.trend = attendance::models::TrendType::IMPROVING;
        result.recentDirection = "Attendance is positively improving over recent sessions.";
        result.isDecliningAlert = false;
    } else if (slope < -slopeThreshold) {
        result.trend = attendance::models::TrendType::DECLINING;
        result.recentDirection = "Attendance is experiencing a downward trend in recent sessions.";
        result.isDecliningAlert = true;
    } else {
        result.trend = attendance::models::TrendType::STABLE;
        result.recentDirection = "Attendance pattern is consistent and steady.";
        result.isDecliningAlert = false;
    }

    return result;
}

} // namespace attendance::algorithms
