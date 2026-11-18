#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp"

namespace attendance::models {

enum class UserRole {
    ADMIN,
    FACULTY,
    STUDENT
};

inline std::string roleToString(UserRole r) {
    switch (r) {
        case UserRole::ADMIN: return "ADMIN";
        case UserRole::FACULTY: return "FACULTY";
        case UserRole::STUDENT: return "STUDENT";
    }
    return "STUDENT";
}

inline UserRole stringToRole(const std::string& s) {
    if (s == "ADMIN") return UserRole::ADMIN;
    if (s == "FACULTY") return UserRole::FACULTY;
    return UserRole::STUDENT;
}

enum class AttendanceStatus {
    PRESENT,
    ABSENT,
    LATE,
    EXCUSED
};

inline std::string statusToString(AttendanceStatus s) {
    switch (s) {
        case AttendanceStatus::PRESENT: return "PRESENT";
        case AttendanceStatus::ABSENT: return "ABSENT";
        case AttendanceStatus::LATE: return "LATE";
        case AttendanceStatus::EXCUSED: return "EXCUSED";
    }
    return "PRESENT";
}

inline AttendanceStatus stringToStatus(const std::string& s) {
    if (s == "ABSENT") return AttendanceStatus::ABSENT;
    if (s == "LATE") return AttendanceStatus::LATE;
    if (s == "EXCUSED") return AttendanceStatus::EXCUSED;
    return AttendanceStatus::PRESENT;
}

enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

inline std::string riskToString(RiskLevel r) {
    switch (r) {
        case RiskLevel::LOW: return "LOW";
        case RiskLevel::MEDIUM: return "MEDIUM";
        case RiskLevel::HIGH: return "HIGH";
        case RiskLevel::CRITICAL: return "CRITICAL";
    }
    return "LOW";
}

inline RiskLevel stringToRisk(const std::string& s) {
    if (s == "CRITICAL") return RiskLevel::CRITICAL;
    if (s == "HIGH") return RiskLevel::HIGH;
    if (s == "MEDIUM") return RiskLevel::MEDIUM;
    return RiskLevel::LOW;
}

enum class TrendType {
    IMPROVING,
    STABLE,
    DECLINING
};

inline std::string trendToString(TrendType t) {
    switch (t) {
        case TrendType::IMPROVING: return "IMPROVING";
        case TrendType::STABLE: return "STABLE";
        case TrendType::DECLINING: return "DECLINING";
    }
    return "STABLE";
}

struct User {
    std::string id;
    std::string email;
    std::string passwordHash;
    std::string name;
    UserRole role{UserRole::STUDENT};
    std::string createdAt;
    std::string updatedAt;

    // Optional navigation links
    std::string studentId;
    std::string facultyId;
};

struct Student {
    std::string id;
    std::string userId;
    std::string rollNumber;
    std::string department;
    int semester{5};
    std::string createdAt;
    std::string updatedAt;

    // Joined fields
    std::string name;
    std::string email;
};

struct Faculty {
    std::string id;
    std::string userId;
    std::string employeeId;
    std::string department;
    std::string createdAt;
    std::string updatedAt;

    // Joined fields
    std::string name;
    std::string email;
};

struct Subject {
    std::string id;
    std::string code;
    std::string name;
    std::string facultyId;
    int credits{3};
    std::string createdAt;
    std::string updatedAt;

    // Joined fields
    std::string facultyName;
    std::string facultyEmail;
    int totalSessions{0};
    int enrolledStudents{0};
    double averageAttendance{0.0};
};

struct Enrollment {
    std::string id;
    std::string studentId;
    std::string subjectId;
    std::string createdAt;
};

struct AttendanceSession {
    std::string id;
    std::string subjectId;
    std::string facultyId;
    std::string sessionDate;
    std::string topic;
    std::string createdAt;
    std::string updatedAt;

    std::string subjectCode;
    std::string subjectName;
};

struct AttendanceRecord {
    std::string id;
    std::string sessionId;
    std::string studentId;
    AttendanceStatus status{AttendanceStatus::PRESENT};
    std::string remarks;
    std::string createdAt;
    std::string updatedAt;

    // Extra session information for chronological sorting
    std::string sessionDate;
    std::string subjectId;
};

struct SubjectAttendance {
    std::string subjectId;
    std::string subjectCode;
    std::string subjectName;
    int totalConducted{0};
    int presentCount{0};
    int absentCount{0};
    int lateCount{0};
    int excusedCount{0};
    double percentage{0.0};
    bool isBelowThreshold{false};
    int consecutiveAbsences{0};
    std::string trend{"STABLE"};
    int classesNeededToRecover{0};
    int classesCanBeMissed{0};
};

struct StudentAttendanceProfile {
    std::string studentId;
    std::string userId;
    std::string name;
    std::string email;
    std::string rollNumber;
    std::string department;
    int semester{5};

    int totalSessionsConducted{0};
    int totalSessionsAttended{0};
    int totalSessionsAbsent{0};
    double overallPercentage{0.0};

    std::string trend{"STABLE"};
    double trendSlope{0.0};
    int currentConsecutiveAbsences{0};
    int maxConsecutiveAbsences{0};
    double recentWindowPercentage{0.0};
    int recentAbsenceCount{0};
    int classesNeededToRecover{0};
    int classesCanBeMissed{0};

    std::unordered_map<std::string, SubjectAttendance> subjectWise;

    std::string riskCategory{"LOW"};
    double riskScore{0.0};
    std::vector<std::string> contributingFactors;
    std::string recommendation;
};

struct UndoAction {
    std::string id;
    std::string recordId;
    std::string sessionId;
    std::string studentId;
    std::string studentName;
    std::string subjectCode;
    AttendanceStatus previousStatus;
    AttendanceStatus newStatus;
    std::string timestamp;
    std::string facultyId;
    std::string reason;
};

struct AlertEvent {
    std::string id;
    std::string type;
    std::string studentId;
    std::string studentName;
    std::string subjectCode;
    std::string message;
    std::string timestamp;
    std::string severity;
};

} // namespace attendance::models
