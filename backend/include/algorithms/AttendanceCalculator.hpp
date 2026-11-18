#pragma once

#include <vector>
#include <cmath>
#include "models/Models.hpp"

namespace attendance::algorithms {

struct AttendanceCalculationResult {
    int totalConducted{0};
    int presentCount{0};
    int absentCount{0};
    int lateCount{0};
    int excusedCount{0};
    double attendedPoints{0.0};
    double percentage{100.0};
    bool isBelowThreshold{false};
};

inline AttendanceCalculationResult calculateAttendancePercentage(
    const std::vector<attendance::models::AttendanceStatus>& records,
    double threshold = 75.0) 
{
    AttendanceCalculationResult result;

    for (const auto& status : records) {
        switch (status) {
            case attendance::models::AttendanceStatus::PRESENT:
                result.presentCount++;
                result.attendedPoints += 1.0;
                result.totalConducted++;
                break;
            case attendance::models::AttendanceStatus::LATE:
                result.lateCount++;
                result.attendedPoints += 0.5;
                result.totalConducted++;
                break;
            case attendance::models::AttendanceStatus::ABSENT:
                result.absentCount++;
                result.totalConducted++;
                break;
            case attendance::models::AttendanceStatus::EXCUSED:
                result.excusedCount++;
                // Excused records do not count towards the conducted denominator
                break;
        }
    }

    if (result.totalConducted == 0) {
        result.percentage = 100.0;
    } else {
        double rawPct = (result.attendedPoints / result.totalConducted) * 100.0;
        result.percentage = std::round(rawPct * 100.0) / 100.0;
    }

    result.isBelowThreshold = result.percentage < threshold;
    return result;
}

} // namespace attendance::algorithms
