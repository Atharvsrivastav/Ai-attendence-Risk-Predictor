#pragma once

#include <cmath>
#include <string>
#include <algorithm>

namespace attendance::algorithms {

struct RecoveryCalculationResult {
    double currentPercentage{100.0};
    double threshold{75.0};
    int totalConducted{0};
    double presentCount{0.0};
    int absentCount{0};
    int classesNeededToRecover{0};
    int classesCanBeMissed{0};
    bool isBelowThreshold{false};
    std::string statusSummary;
};

struct SimulationResult {
    int additionalAttended{0};
    int additionalMissed{0};
    int projectedTotalSessions{0};
    double projectedAttendedSessions{0.0};
    double projectedPercentage{100.0};
    bool meetsThreshold{true};
    double deltaPercentage{0.0};
};

inline RecoveryCalculationResult calculateRecovery(
    double presentSessions,
    int totalSessions,
    double threshold = 75.0) 
{
    RecoveryCalculationResult result;
    result.threshold = threshold;
    result.totalConducted = totalSessions;
    result.presentCount = presentSessions;
    result.absentCount = std::max(0, totalSessions - static_cast<int>(std::round(presentSessions)));

    if (totalSessions == 0) {
        result.currentPercentage = 100.0;
        result.classesNeededToRecover = 0;
        result.classesCanBeMissed = 0;
        result.isBelowThreshold = false;
        result.statusSummary = "No classes conducted yet.";
        return result;
    }

    double currentPercentage = (presentSessions / totalSessions) * 100.0;
    result.currentPercentage = std::round(currentPercentage * 100.0) / 100.0;
    result.isBelowThreshold = result.currentPercentage < threshold;

    double R = threshold / 100.0;

    if (result.isBelowThreshold) {
        // Formula: ceil((R * T - P) / (1 - R))
        double needed = (R * totalSessions - presentSessions) / (1.0 - R);
        result.classesNeededToRecover = std::max(1, static_cast<int>(std::ceil(needed)));
        result.classesCanBeMissed = 0;
        result.statusSummary = "Needs to attend the next " + 
            std::to_string(result.classesNeededToRecover) + " consecutive classes to reach " + 
            std::to_string(static_cast<int>(threshold)) + "%.";
    } else {
        // Formula: floor((P - R * T) / R)
        double canMiss = (presentSessions - R * totalSessions) / R;
        result.classesCanBeMissed = std::max(0, static_cast<int>(std::floor(canMiss)));
        result.classesNeededToRecover = 0;
        result.statusSummary = "Currently safe. Can miss up to " + 
            std::to_string(result.classesCanBeMissed) + " class(es) without dropping below " + 
            std::to_string(static_cast<int>(threshold)) + "%.";
    }

    return result;
}

inline SimulationResult simulateAttendance(
    double currentPresent,
    int currentTotal,
    int attendNext,
    int missNext,
    double threshold = 75.0) 
{
    SimulationResult sim;
    sim.additionalAttended = attendNext;
    sim.additionalMissed = missNext;
    sim.projectedTotalSessions = currentTotal + attendNext + missNext;
    sim.projectedAttendedSessions = currentPresent + attendNext;

    if (sim.projectedTotalSessions == 0) {
        sim.projectedPercentage = 100.0;
    } else {
        double raw = (sim.projectedAttendedSessions / sim.projectedTotalSessions) * 100.0;
        sim.projectedPercentage = std::round(raw * 100.0) / 100.0;
    }

    sim.meetsThreshold = sim.projectedPercentage >= threshold;

    double currentPct = currentTotal == 0 ? 100.0 : (currentPresent / currentTotal) * 100.0;
    sim.deltaPercentage = std::round((sim.projectedPercentage - currentPct) * 100.0) / 100.0;

    return sim;
}

} // namespace attendance::algorithms
