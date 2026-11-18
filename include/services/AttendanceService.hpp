#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "models/Models.hpp"
#include "repositories/AttendanceRepository.hpp"
#include "dsa/StudentHashMap.hpp"
#include "json.hpp"

namespace attendance::services {

struct AllProfilesResult {
    std::vector<attendance::models::StudentAttendanceProfile> profiles;
    attendance::dsa::StudentHashMap<std::string, attendance::models::StudentAttendanceProfile> hashMap;
};

class AttendanceService {
public:
    explicit AttendanceService(attendance::repositories::AttendanceRepository& repo) : repo_(repo) {}

    std::optional<attendance::models::StudentAttendanceProfile> getStudentProfile(
        const std::string& studentId, double threshold = 75.0);

    AllProfilesResult getAllStudentProfiles(double threshold = 75.0);

    nlohmann::json getClassAnalytics(double threshold = 75.0);

    // CSV Reports Generator
    std::string generateStudentsCsv(double threshold = 75.0);
    std::string generateRiskReportCsv(double threshold = 75.0);
    std::string generateSubjectsReportCsv();
    std::string generateFacultyReportCsv();

private:
    attendance::repositories::AttendanceRepository& repo_;
};

} // namespace attendance::services
