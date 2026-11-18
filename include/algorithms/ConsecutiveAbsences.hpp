#pragma once

#include <vector>
#include <algorithm>
#include "models/Models.hpp"

namespace attendance::algorithms {

struct ConsecutiveAbsencesResult {
    int currentConsecutive{0};
    int maxConsecutive{0};
    bool isSevereStreak{false};
    std::string alertMessage;
};

inline ConsecutiveAbsencesResult detectConsecutiveAbsences(
    const std::vector<attendance::models::AttendanceStatus>& records,
    int severeThreshold = 3) 
{
    ConsecutiveAbsencesResult result;
    if (records.empty()) return result;

    // 1. Current streak: scan backwards from the most recent session
    int currentStreak = 0;
    for (int i = static_cast<int>(records.size()) - 1; i >= 0; i--) {
        auto status = records[i];
        if (status == attendance::models::AttendanceStatus::EXCUSED) {
            continue; // Excused leaves do not break nor increment streak
        }
        if (status == attendance::models::AttendanceStatus::ABSENT) {
            currentStreak++;
        } else {
            // PRESENT or LATE breaks the current active streak
            break;
        }
    }
    result.currentConsecutive = currentStreak;

    // 2. Maximum streak across entire history
    int maxStreak = 0;
    int runStreak = 0;
    for (const auto& status : records) {
        if (status == attendance::models::AttendanceStatus::EXCUSED) {
            continue;
        }
        if (status == attendance::models::AttendanceStatus::ABSENT) {
            runStreak++;
            if (runStreak > maxStreak) {
                maxStreak = runStreak;
            }
        } else {
            runStreak = 0;
        }
    }
    result.maxConsecutive = maxStreak;
    result.isSevereStreak = result.currentConsecutive >= severeThreshold;

    if (result.isSevereStreak) {
        result.alertMessage = "Critical Alert: Student has missed " + 
            std::to_string(result.currentConsecutive) + " consecutive sessions!";
    } else if (result.currentConsecutive > 0) {
        result.alertMessage = "Notice: Current absence streak of " + 
            std::to_string(result.currentConsecutive) + " class(es).";
    } else {
        result.alertMessage = "No active consecutive absence streak.";
    }

    return result;
}

} // namespace attendance::algorithms
