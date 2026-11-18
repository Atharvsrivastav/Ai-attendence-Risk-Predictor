#include "services/AttendanceService.hpp"
#include "algorithms/AttendanceCalculator.hpp"
#include "algorithms/ConsecutiveAbsences.hpp"
#include "algorithms/RecentWindowAnalysis.hpp"
#include "algorithms/TrendAnalysis.hpp"
#include "algorithms/RecoveryCalculator.hpp"
#include "ml/RiskPredictor.hpp"
#include "dsa/AttendanceStack.hpp"
#include "dsa/AttendanceQueue.hpp"

namespace attendance::services {

std::optional<attendance::models::StudentAttendanceProfile> AttendanceService::getStudentProfile(
    const std::string& studentId, double threshold) 
{
    auto studentOpt = repo_.findStudentById(studentId);
    if (!studentOpt) return std::nullopt;
    const auto& student = *studentOpt;

    auto allRecords = repo_.findRecordsByStudent(studentId);
    auto allSubjects = repo_.findAllSubjects();

    attendance::models::StudentAttendanceProfile prof;
    prof.studentId = student.id;
    prof.userId = student.userId;
    prof.name = student.name;
    prof.email = student.email;
    prof.rollNumber = student.rollNumber;
    prof.department = student.department;
    prof.semester = student.semester;

    // Collect chronological statuses for overall profile
    std::vector<attendance::models::AttendanceStatus> allStatuses;
    for (const auto& r : allRecords) {
        allStatuses.push_back(r.status);
    }

    // Subject wise grouping using HashMap
    attendance::dsa::StudentHashMap<std::string, std::vector<attendance::models::AttendanceStatus>> subjectMap;
    for (const auto& sub : allSubjects) {
        subjectMap.set(sub.id, {});
    }
    for (const auto& r : allRecords) {
        std::vector<attendance::models::AttendanceStatus> list;
        subjectMap.get(r.subjectId, list);
        list.push_back(r.status);
        subjectMap.set(r.subjectId, list);
    }

    // 1. Overall stats
    auto overallCalc = attendance::algorithms::calculateAttendancePercentage(allStatuses, threshold);
    prof.totalSessionsConducted = overallCalc.totalConducted;
    prof.totalSessionsAttended = static_cast<int>(std::round(overallCalc.attendedPoints));
    prof.totalSessionsAbsent = overallCalc.absentCount;
    prof.overallPercentage = overallCalc.percentage;

    // 2. Streaks and Momentum
    auto consec = attendance::algorithms::detectConsecutiveAbsences(allStatuses);
    prof.currentConsecutiveAbsences = consec.currentConsecutive;
    prof.maxConsecutiveAbsences = consec.maxConsecutive;

    auto recent = attendance::algorithms::analyzeRecentAttendanceWindow(allStatuses, overallCalc.percentage, 5);
    prof.recentWindowPercentage = recent.recentPercentage;
    prof.recentAbsenceCount = recent.recentAbsenceCount;

    auto trendRes = attendance::algorithms::analyzeAttendanceTrend(allStatuses);
    prof.trend = attendance::models::trendToString(trendRes.trend);
    prof.trendSlope = trendRes.slope;

    // 3. Recovery
    auto recovery = attendance::algorithms::calculateRecovery(overallCalc.attendedPoints, overallCalc.totalConducted, threshold);
    prof.classesNeededToRecover = recovery.classesNeededToRecover;
    prof.classesCanBeMissed = recovery.classesCanBeMissed;

    // 4. Subject Wise Breakdown
    for (const auto& sub : allSubjects) {
        std::vector<attendance::models::AttendanceStatus> subStatuses;
        subjectMap.get(sub.id, subStatuses);

        auto subCalc = attendance::algorithms::calculateAttendancePercentage(subStatuses, threshold);
        auto subConsec = attendance::algorithms::detectConsecutiveAbsences(subStatuses);
        auto subTrend = attendance::algorithms::analyzeAttendanceTrend(subStatuses);
        auto subRec = attendance::algorithms::calculateRecovery(subCalc.attendedPoints, subCalc.totalConducted, threshold);

        attendance::models::SubjectAttendance sa;
        sa.subjectId = sub.id;
        sa.subjectCode = sub.code;
        sa.subjectName = sub.name;
        sa.totalConducted = subCalc.totalConducted;
        sa.presentCount = subCalc.presentCount;
        sa.absentCount = subCalc.absentCount;
        sa.lateCount = subCalc.lateCount;
        sa.excusedCount = subCalc.excusedCount;
        sa.percentage = subCalc.percentage;
        sa.isBelowThreshold = subCalc.isBelowThreshold;
        sa.consecutiveAbsences = subConsec.currentConsecutive;
        sa.trend = attendance::models::trendToString(subTrend.trend);
        sa.classesNeededToRecover = subRec.classesNeededToRecover;
        sa.classesCanBeMissed = subRec.classesCanBeMissed;

        prof.subjectWise[sub.id] = sa;
    }

    // 5. ML Risk Prediction
    auto pred = attendance::ml::RiskPredictor::predict(allStatuses, threshold);
    prof.riskCategory = attendance::models::riskToString(pred.riskCategory);
    prof.riskScore = pred.riskScore;
    prof.contributingFactors = pred.contributingFactors;
    prof.recommendation = pred.recommendation;

    return prof;
}

AllProfilesResult AttendanceService::getAllStudentProfiles(double threshold) {
    AllProfilesResult result;
    auto students = repo_.findAllStudents();

    for (const auto& stu : students) {
        auto p = getStudentProfile(stu.id, threshold);
        if (p) {
            result.profiles.push_back(*p);
            result.hashMap.set(p->rollNumber, *p);
            result.hashMap.set(p->studentId, *p);
        }
    }
    return result;
}

nlohmann::json AttendanceService::getClassAnalytics(double threshold) {
    auto all = getAllStudentProfiles(threshold);
    const auto& profiles = all.profiles;

    int totalStudents = static_cast<int>(profiles.size());
    double sumAttendance = 0.0;
    int belowThresholdCount = 0;

    int countLow = 0, countMedium = 0, countHigh = 0, countCritical = 0;

    for (const auto& p : profiles) {
        sumAttendance += p.overallPercentage;
        if (p.overallPercentage < threshold) {
            belowThresholdCount++;
        }
        if (p.riskCategory == "CRITICAL") countCritical++;
        else if (p.riskCategory == "HIGH") countHigh++;
        else if (p.riskCategory == "MEDIUM") countMedium++;
        else countLow++;
    }

    double avgAttendance = (totalStudents == 0) ? 0.0 : std::round((sumAttendance / totalStudents) * 100.0) / 100.0;

    // Subject breakdown
    auto subjects = repo_.findAllSubjects();
    nlohmann::json subBreakdown = nlohmann::json::array();
    for (const auto& sub : subjects) {
        double subPctSum = 0.0;
        int subCount = 0;
        for (const auto& p : profiles) {
            auto it = p.subjectWise.find(sub.id);
            if (it != p.subjectWise.end()) {
                subPctSum += it->second.percentage;
                subCount++;
            }
        }
        double subAvg = (subCount == 0) ? 0.0 : std::round((subPctSum / subCount) * 100.0) / 100.0;
        subBreakdown.push_back({
            {"subjectId", sub.id},
            {"subjectCode", sub.code},
            {"subjectName", sub.name},
            {"faculty", sub.facultyName},
            {"averageAttendance", subAvg},
            {"enrolledStudents", subCount}
        });
    }

    // Timeline simulation slices
    nlohmann::json timeline = nlohmann::json::array({
        {{"session", "Sess 1-20"}, {"avg", 79.2}, {"atRisk", 3}},
        {{"session", "Sess 21-40"}, {"avg", 79.2}, {"atRisk", 3}},
        {{"session", "Sess 41-60"}, {"avg", 66.7}, {"atRisk", 15}},
        {{"session", "Sess 61-80"}, {"avg", 76.0}, {"atRisk", 3}},
        {{"session", "Sess 81-100"}, {"avg", 60.4}, {"atRisk", 7}}
    });

    // Recent alerts from queue
    auto recentAlerts = repo_.findRecentAlerts(8);
    nlohmann::json alertsJson = nlohmann::json::array();
    for (const auto& a : recentAlerts) {
        alertsJson.push_back({
            {"id", a.id},
            {"type", a.type},
            {"studentName", a.studentName},
            {"subjectCode", a.subjectCode},
            {"message", a.message},
            {"severity", a.severity},
            {"timestamp", a.timestamp}
        });
    }

    return {
        {"totalStudents", totalStudents},
        {"averageAttendance", avgAttendance},
        {"threshold", threshold},
        {"belowThresholdCount", belowThresholdCount},
        {"atRiskCount", belowThresholdCount},
        {"riskDistribution", {
            {"low", countLow},
            {"medium", countMedium},
            {"high", countHigh},
            {"critical", countCritical}
        }},
        {"subjectBreakdown", subBreakdown},
        {"sessionTimeline", timeline},
        {"recentAlerts", alertsJson},
        {"alertQueueSize", attendance::dsa::getGlobalAlertQueue().size()},
        {"undoStackDepth", attendance::dsa::getGlobalUndoStack().size()}
    };
}

std::string AttendanceService::generateStudentsCsv(double threshold) {
    auto all = getAllStudentProfiles(threshold);
    std::stringstream ss;
    ss << "Roll Number,Student Name,Email,Department,Semester,Conducted,Attended,Attendance %,Status,Risk Category,Risk Score,Streak,Recovery Needed\n";
    for (const auto& p : all.profiles) {
        ss << "\"" << p.rollNumber << "\","
           << "\"" << p.name << "\","
           << "\"" << p.email << "\","
           << "\"" << p.department << "\","
           << p.semester << ","
           << p.totalSessionsConducted << ","
           << p.totalSessionsAttended << ","
           << std::fixed << std::setprecision(1) << p.overallPercentage << ","
           << (p.overallPercentage >= threshold ? "Eligible" : "Deficit") << ","
           << p.riskCategory << ","
           << std::fixed << std::setprecision(2) << p.riskScore << ","
           << p.currentConsecutiveAbsences << ","
           << p.classesNeededToRecover << "\n";
    }
    return ss.str();
}

std::string AttendanceService::generateRiskReportCsv(double threshold) {
    auto all = getAllStudentProfiles(threshold);
    std::stringstream ss;
    ss << "Roll Number,Student Name,Department,Semester,Overall %,Risk Category,Risk Score,Absence Streak,Consecutive Classes Needed to Recover,Top Contributing Factor\n";
    for (const auto& p : all.profiles) {
        if (p.overallPercentage < threshold || p.riskCategory == "HIGH" || p.riskCategory == "CRITICAL") {
            std::string topFactor = p.contributingFactors.empty() ? "None" : p.contributingFactors[0];
            std::replace(topFactor.begin(), topFactor.end(), ',', ';');
            ss << "\"" << p.rollNumber << "\","
               << "\"" << p.name << "\","
               << "\"" << p.department << "\","
               << p.semester << ","
               << std::fixed << std::setprecision(1) << p.overallPercentage << ","
               << p.riskCategory << ","
               << std::fixed << std::setprecision(2) << p.riskScore << ","
               << p.currentConsecutiveAbsences << ","
               << p.classesNeededToRecover << ","
               << "\"" << topFactor << "\"\n";
        }
    }
    return ss.str();
}

std::string AttendanceService::generateSubjectsReportCsv() {
    auto subjects = repo_.findAllSubjects();
    std::stringstream ss;
    ss << "Subject Code,Course Name,Faculty In-Charge,Credits,Enrolled Students,Total Sessions Held,Average Attendance %\n";
    for (const auto& s : subjects) {
        ss << "\"" << s.code << "\","
           << "\"" << s.name << "\","
           << "\"" << s.facultyName << "\","
           << s.credits << ","
           << s.enrolledStudents << ","
           << s.totalSessions << ","
           << std::fixed << std::setprecision(1) << s.averageAttendance << "\n";
    }
    return ss.str();
}

std::string AttendanceService::generateFacultyReportCsv() {
    auto facultyList = repo_.findAllFaculty();
    std::stringstream ss;
    ss << "Employee ID,Faculty Name,Email,Department,Assigned Course Codes\n";
    for (const auto& f : facultyList) {
        auto subList = repo_.findSubjectsByFacultyId(f.id);
        std::string subCodes = "";
        for (size_t i = 0; i < subList.size(); i++) {
            if (i > 0) subCodes += "; ";
            subCodes += subList[i].code;
        }
        ss << "\"" << f.employeeId << "\","
           << "\"" << f.name << "\","
           << "\"" << f.email << "\","
           << "\"" << f.department << "\","
           << "\"" << subCodes << "\"\n";
    }
    return ss.str();
}

} // namespace attendance::services
