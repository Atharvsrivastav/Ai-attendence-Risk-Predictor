#pragma once

#include <vector>
#include <cmath>
#include "models/Models.hpp"
#include "algorithms/AttendanceCalculator.hpp"

namespace attendance::algorithms {

struct RecentWindowResult {
    double recentPercentage{100.0};
    int recentAbsenceCount{0};
    int windowSizeExamined{0};
    bool isSuddenDrop{false};
    std::string commentary;
};

inline RecentWindowResult analyzeRecentAttendanceWindow(
    const std::vector<attendance::models::AttendanceStatus>& records,
    double overallPercentage = 100.0,
    int windowSize = 5,
    double dropThreshold = 15.0) 
{
    RecentWindowResult result;
    if (records.empty()) return result;

    // Filter out EXCUSED records to get true recent conducted sessions
    std::vector<attendance::models::AttendanceStatus> conductedOnly;
    for (const auto& s : records) {
        if (s != attendance::models::AttendanceStatus::EXCUSED) {
            conductedOnly.push_back(s);
        }
    }

    if (conductedOnly.empty()) return result;

    int actualSize = std::min(windowSize, static_cast<int>(conductedOnly.size()));
    result.windowSizeExamined = actualSize;

    std::vector<attendance::models::AttendanceStatus> windowSlice(
        conductedOnly.end() - actualSize, conductedOnly.end()
    );

    auto windowCalc = calculateAttendancePercentage(windowSlice);
    result.recentPercentage = windowCalc.percentage;
    result.recentAbsenceCount = windowCalc.absentCount;

    // Detect sudden momentum drop
    if (actualSize >= 3 && (overallPercentage - result.recentPercentage) >= dropThreshold) {
        result.isSuddenDrop = true;
        result.commentary = "Sharp attendance deceleration: recent " + 
            std::to_string(actualSize) + "-lecture average (" + 
            std::to_string(static_cast<int>(result.recentPercentage)) + "%) is significantly below semester baseline (" + 
            std::to_string(static_cast<int>(overallPercentage)) + "%).";
    } else {
        result.commentary = "Recent attendance momentum aligns with historical baseline.";
    }

    return result;
}

} // namespace attendance::algorithms
