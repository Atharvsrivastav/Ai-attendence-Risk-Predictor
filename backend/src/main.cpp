#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>

#include "crow_all.h"
#include "json.hpp"
#include "database/Database.hpp"
#include "repositories/AttendanceRepository.hpp"
#include "services/AttendanceService.hpp"
#include "auth/AuthService.hpp"
#include "algorithms/Sorting.hpp"
#include "algorithms/Searching.hpp"
#include "algorithms/RecoveryCalculator.hpp"
#include "ml/RiskPredictor.hpp"
#include "dsa/AttendanceQueue.hpp"
#include "dsa/AttendanceStack.hpp"
#include "dsa/AttendanceGraph.hpp"
#include "dsa/StudentHashMap.hpp"

using json = nlohmann::json;

// Helper to extract JWT token from Cookie or Authorization header
static std::string extractToken(const crow::request& req) {
    auto authHeader = req.get_header_value("authorization");
    if (!authHeader.empty() && authHeader.rfind("Bearer ", 0) == 0) {
        return authHeader.substr(7);
    }
    auto cookieHeader = req.get_header_value("cookie");
    if (!cookieHeader.empty()) {
        size_t pos = cookieHeader.find("auth_token=");
        if (pos != std::string::npos) {
            size_t start = pos + 11;
            size_t end = cookieHeader.find(';', start);
            return cookieHeader.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        }
    }
    return "";
}

// CORS helper to append headers
static crow::response makeJsonResponse(int code, const json& j) {
    crow::response res(code, j.dump());
    res.set_header("Content-Type", "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie");
    res.set_header("Access-Control-Allow-Credentials", "true");
    return res;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::string dbPath = "database/attendance.db";
    auto& db = attendance::db::Database::instance();
    if (!db.open(dbPath)) {
        std::cerr << "Fatal: Could not open database at " << dbPath << std::endl;
        return 1;
    }

    attendance::repositories::AttendanceRepository repo(db);
    attendance::services::AttendanceService service(repo);
    const std::string jwtSecret = "attendance-risk-prediction-jwt-secret-key-987654321";

    crow::SimpleApp app;

    // Global OPTIONS handler for CORS preflight
    CROW_ROUTE(app, "/api/<path>")
        .methods("OPTIONS"_method)
    ([](const crow::request&, crow::response& res, std::string) {
        res.code = 204;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie");
        res.set_header("Access-Control-Allow-Credentials", "true");
        res.end();
    });

    // Health check
    CROW_ROUTE(app, "/health")
    ([]() {
        json j = {{"status", "healthy"}, {"engine", "C++20 Crow REST Server"}};
        return makeJsonResponse(200, j);
    });

    // 1. POST /api/auth/login
    CROW_ROUTE(app, "/api/auth/login")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string email = body.value("email", "");
            std::string password = body.value("password", "");

            if (email.empty() || password.empty()) {
                return makeJsonResponse(400, {{"error", "Email and password are required"}});
            }

            auto userOpt = repo.findUserByEmail(email);
            if (!userOpt) {
                return makeJsonResponse(401, {{"error", "Invalid email or password"}});
            }

            const auto& user = *userOpt;
            if (!attendance::auth::AuthService::verifyPassword(password, user.passwordHash)) {
                return makeJsonResponse(401, {{"error", "Invalid email or password"}});
            }

            attendance::auth::SessionClaims claims;
            claims.userId = user.id;
            claims.email = user.email;
            claims.name = user.name;
            claims.role = user.role;
            claims.studentId = user.studentId;
            claims.facultyId = user.facultyId;

            std::string token = attendance::auth::AuthService::generateToken(claims, jwtSecret);

            json userJson = {
                {"id", user.id},
                {"email", user.email},
                {"name", user.name},
                {"role", attendance::models::roleToString(user.role)},
                {"studentId", user.studentId.empty() ? nullptr : json(user.studentId)},
                {"facultyId", user.facultyId.empty() ? nullptr : json(user.facultyId)}
            };

            auto res = makeJsonResponse(200, {
                {"success", true},
                {"user", userJson},
                {"token", token}
            });

            // Set-Cookie header
            res.add_header("Set-Cookie", "auth_token=" + token + "; Path=/; Max-Age=604800; SameSite=Lax");
            return res;
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Invalid JSON payload"}});
        }
    });

    // 2. POST /api/auth/logout
    CROW_ROUTE(app, "/api/auth/logout")
        .methods("POST"_method)
    ([]() {
        auto res = makeJsonResponse(200, {{"success", true}, {"message", "Logged out successfully"}});
        res.add_header("Set-Cookie", "auth_token=; Path=/; Max-Age=0; SameSite=Lax");
        return res;
    });

    // 3. GET /api/auth/me
    CROW_ROUTE(app, "/api/auth/me")
    ([&](const crow::request& req) {
        std::string token = extractToken(req);
        if (token.empty()) {
            return makeJsonResponse(401, {{"authenticated", false}, {"user", nullptr}});
        }
        auto claims = attendance::auth::AuthService::verifyToken(token, jwtSecret);
        if (!claims) {
            return makeJsonResponse(401, {{"authenticated", false}, {"user", nullptr}});
        }

        auto userOpt = repo.findUserById(claims->userId);
        if (!userOpt) {
            return makeJsonResponse(401, {{"authenticated", false}, {"user", nullptr}});
        }
        const auto& u = *userOpt;
        json userJson = {
            {"id", u.id},
            {"email", u.email},
            {"name", u.name},
            {"role", attendance::models::roleToString(u.role)},
            {"studentId", u.studentId.empty() ? nullptr : json(u.studentId)},
            {"facultyId", u.facultyId.empty() ? nullptr : json(u.facultyId)}
        };
        return makeJsonResponse(200, {{"authenticated", true}, {"user", userJson}});
    });

    // 4. GET /api/analytics
    CROW_ROUTE(app, "/api/analytics")
    ([&](const crow::request& req) {
        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }
        json analytics = service.getClassAnalytics(threshold);
        return makeJsonResponse(200, analytics);
    });

    // 5. GET /api/subjects
    CROW_ROUTE(app, "/api/subjects")
    ([&]() {
        auto subjects = repo.findAllSubjects();
        json arr = json::array();
        for (const auto& s : subjects) {
            arr.push_back({
                {"id", s.id},
                {"code", s.code},
                {"name", s.name},
                {"facultyId", s.facultyId},
                {"credits", s.credits},
                {"facultyName", s.facultyName},
                {"facultyEmail", s.facultyEmail},
                {"totalSessions", s.totalSessions},
                {"enrolledStudents", s.enrolledStudents}
            });
        }
        return makeJsonResponse(200, {{"subjects", arr}});
    });

    // 6. GET /api/students
    CROW_ROUTE(app, "/api/students")
    ([&](const crow::request& req) {
        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }
        std::string search = req.url_params.get("search") ? req.url_params.get("search") : "";
        std::string sortBy = req.url_params.get("sortBy") ? req.url_params.get("sortBy") : "attendance";
        std::string sortOrder = req.url_params.get("sortOrder") ? req.url_params.get("sortOrder") : "asc";
        std::string sortAlgorithm = req.url_params.get("sortAlgorithm") ? req.url_params.get("sortAlgorithm") : "mergeSort";
        std::string filterRisk = req.url_params.get("filterRisk") ? req.url_params.get("filterRisk") : "";
        bool belowThresholdOnly = req.url_params.get("belowThreshold") && std::string(req.url_params.get("belowThreshold")) == "true";

        auto allProfiles = service.getAllStudentProfiles(threshold);
        auto filtered = allProfiles.profiles;

        // Linear search metric
        json searchMetric = nullptr;
        if (!search.empty()) {
            std::string sLow = search;
            std::transform(sLow.begin(), sLow.end(), sLow.begin(), ::tolower);

            auto searchRes = attendance::algorithms::linearSearch(
                filtered,
                [&](const attendance::models::StudentAttendanceProfile& p) {
                    std::string n = p.name;
                    std::string r = p.rollNumber;
                    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
                    return n.find(sLow) != std::string::npos || r.find(sLow) != std::string::npos;
                },
                [](const attendance::models::StudentAttendanceProfile& p) {
                    return p.rollNumber + ": " + p.name;
                }
            );

            searchMetric = {
                {"algorithm", searchRes.algorithm},
                {"timeTakenMicroseconds", searchRes.timeMicroseconds},
                {"stepsCount", searchRes.stepsCount},
                {"timeComplexity", searchRes.timeComplexity}
            };

            std::vector<attendance::models::StudentAttendanceProfile> matched;
            for (const auto& p : filtered) {
                std::string n = p.name, r = p.rollNumber;
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                std::transform(r.begin(), r.end(), r.begin(), ::tolower);
                if (n.find(sLow) != std::string::npos || r.find(sLow) != std::string::npos) {
                    matched.push_back(p);
                }
            }
            filtered = matched;
        }

        if (!filterRisk.empty()) {
            std::vector<attendance::models::StudentAttendanceProfile> rf;
            for (const auto& p : filtered) {
                if (p.riskCategory == filterRisk) rf.push_back(p);
            }
            filtered = rf;
        }

        if (belowThresholdOnly) {
            std::vector<attendance::models::StudentAttendanceProfile> bf;
            for (const auto& p : filtered) {
                if (p.overallPercentage < threshold) bf.push_back(p);
            }
            filtered = bf;
        }

        // Sorting
        auto comp = [&](const attendance::models::StudentAttendanceProfile& a, const attendance::models::StudentAttendanceProfile& b) {
            if (sortBy == "riskScore") {
                return (sortOrder == "desc") ? (a.riskScore > b.riskScore) : (a.riskScore < b.riskScore);
            } else if (sortBy == "absences") {
                return (sortOrder == "desc") ? (a.totalSessionsAbsent > b.totalSessionsAbsent) : (a.totalSessionsAbsent < b.totalSessionsAbsent);
            } else if (sortBy == "rollNumber") {
                return (sortOrder == "desc") ? (a.rollNumber > b.rollNumber) : (a.rollNumber < b.rollNumber);
            } else {
                return (sortOrder == "desc") ? (a.overallPercentage > b.overallPercentage) : (a.overallPercentage < b.overallPercentage);
            }
        };

        size_t comparisonsCount = 0;
        size_t swapsCount = 0;
        long long durationUs = 0;

        if (sortAlgorithm == "quickSort") {
            auto qs = attendance::algorithms::quickSortWithMetrics(filtered, comp);
            filtered = qs.sorted;
            comparisonsCount = qs.comparisons;
            swapsCount = qs.swaps;
            durationUs = qs.durationMicroseconds;
        } else {
            auto ms = attendance::algorithms::mergeSortWithMetrics(filtered, comp);
            filtered = ms.sorted;
            comparisonsCount = ms.comparisons;
            durationUs = ms.durationMicroseconds;
        }

        json stuArray = json::array();
        for (const auto& p : filtered) {
            json sw = json::object();
            for (const auto& [subId, sa] : p.subjectWise) {
                sw[subId] = {
                    {"subjectId", sa.subjectId},
                    {"subjectCode", sa.subjectCode},
                    {"subjectName", sa.subjectName},
                    {"totalConducted", sa.totalConducted},
                    {"presentCount", sa.presentCount},
                    {"absentCount", sa.absentCount},
                    {"lateCount", sa.lateCount},
                    {"excusedCount", sa.excusedCount},
                    {"percentage", sa.percentage},
                    {"isBelowThreshold", sa.isBelowThreshold},
                    {"consecutiveAbsences", sa.consecutiveAbsences},
                    {"trend", sa.trend},
                    {"classesNeededToRecover", sa.classesNeededToRecover},
                    {"classesCanBeMissed", sa.classesCanBeMissed}
                };
            }

            stuArray.push_back({
                {"id", p.studentId},
                {"studentId", p.studentId},
                {"userId", p.userId},
                {"name", p.name},
                {"email", p.email},
                {"rollNumber", p.rollNumber},
                {"department", p.department},
                {"semester", p.semester},
                {"totalSessionsConducted", p.totalSessionsConducted},
                {"totalSessionsAttended", p.totalSessionsAttended},
                {"totalSessionsAbsent", p.totalSessionsAbsent},
                {"overallPercentage", p.overallPercentage},
                {"trend", p.trend},
                {"trendSlope", p.trendSlope},
                {"currentConsecutiveAbsences", p.currentConsecutiveAbsences},
                {"maxConsecutiveAbsences", p.maxConsecutiveAbsences},
                {"recentWindowPercentage", p.recentWindowPercentage},
                {"recentAbsenceCount", p.recentAbsenceCount},
                {"classesNeededToRecover", p.classesNeededToRecover},
                {"classesCanBeMissed", p.classesCanBeMissed},
                {"subjectWise", sw},
                {"riskCategory", p.riskCategory},
                {"riskScore", p.riskScore},
                {"contributingFactors", p.contributingFactors},
                {"recommendation", p.recommendation}
            });
        }

        json dsaMetrics = {
            {"sort", {
                {"algorithm", (sortAlgorithm == "quickSort") ? "QuickSort" : "MergeSort"},
                {"comparisons", comparisonsCount},
                {"swaps", swapsCount},
                {"timeTakenMicroseconds", std::max(1LL, durationUs)},
                {"timeComplexity", (sortAlgorithm == "quickSort") ? "O(n log n) avg, O(n^2) worst" : "O(n log n) guaranteed"},
                {"spaceComplexity", (sortAlgorithm == "quickSort") ? "O(log n)" : "O(n)"}
            }},
            {"search", searchMetric},
            {"hashMapStats", allProfiles.hashMap.getDiagnostics()}
        };

        return makeJsonResponse(200, {
            {"students", stuArray},
            {"total", stuArray.size()},
            {"threshold", threshold},
            {"dsaMetrics", dsaMetrics}
        });
    });

    // 7. POST /api/students (Student Enrollment)
    CROW_ROUTE(app, "/api/students")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");
            std::string email = body.value("email", "");
            std::string rollNumber = body.value("rollNumber", "");
            std::string department = body.value("department", "Computer Science & Engineering");
            int semester = body.value("semester", 5);

            if (name.empty() || email.empty() || rollNumber.empty()) {
                return makeJsonResponse(400, {{"error", "Name, email, and roll number are required"}});
            }

            std::string userId = "usr_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::string studentId = "stu_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

            attendance::models::User u;
            u.id = userId;
            u.email = email;
            u.name = name;
            u.passwordHash = attendance::auth::AuthService::hashPassword("student123");
            u.role = attendance::models::UserRole::STUDENT;

            if (!repo.createUser(u)) {
                return makeJsonResponse(400, {{"error", "Email or roll number already exists"}});
            }

            attendance::models::Student s;
            s.id = studentId;
            s.userId = userId;
            s.rollNumber = rollNumber;
            s.department = department;
            s.semester = semester;
            repo.createStudent(s);

            auto subjects = repo.findAllSubjects();
            for (const auto& sub : subjects) {
                repo.createEnrollment(studentId, sub.id);
            }

            return makeJsonResponse(201, {
                {"success", true},
                {"student", {
                    {"id", studentId},
                    {"userId", userId},
                    {"rollNumber", rollNumber},
                    {"department", department},
                    {"semester", semester}
                }}
            });
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Failed to enroll student"}});
        }
    });

    // 8. GET /api/students/<id>
    CROW_ROUTE(app, "/api/students/<string>")
    ([&](const crow::request& req, std::string studentId) {
        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }

        auto profOpt = service.getStudentProfile(studentId, threshold);
        if (!profOpt) {
            return makeJsonResponse(404, {{"error", "Student not found"}});
        }

        const auto& p = *profOpt;
        json sw = json::object();
        for (const auto& [subId, sa] : p.subjectWise) {
            sw[subId] = {
                {"subjectId", sa.subjectId},
                {"subjectCode", sa.subjectCode},
                {"subjectName", sa.subjectName},
                {"totalConducted", sa.totalConducted},
                {"presentCount", sa.presentCount},
                {"absentCount", sa.absentCount},
                {"lateCount", sa.lateCount},
                {"excusedCount", sa.excusedCount},
                {"percentage", sa.percentage},
                {"isBelowThreshold", sa.isBelowThreshold},
                {"consecutiveAbsences", sa.consecutiveAbsences},
                {"trend", sa.trend},
                {"classesNeededToRecover", sa.classesNeededToRecover},
                {"classesCanBeMissed", sa.classesCanBeMissed}
            };
        }

        json profileJson = {
            {"id", p.studentId},
            {"studentId", p.studentId},
            {"userId", p.userId},
            {"name", p.name},
            {"email", p.email},
            {"rollNumber", p.rollNumber},
            {"department", p.department},
            {"semester", p.semester},
            {"totalSessionsConducted", p.totalSessionsConducted},
            {"totalSessionsAttended", p.totalSessionsAttended},
            {"totalSessionsAbsent", p.totalSessionsAbsent},
            {"overallPercentage", p.overallPercentage},
            {"trend", p.trend},
            {"trendSlope", p.trendSlope},
            {"currentConsecutiveAbsences", p.currentConsecutiveAbsences},
            {"maxConsecutiveAbsences", p.maxConsecutiveAbsences},
            {"recentWindowPercentage", p.recentWindowPercentage},
            {"recentAbsenceCount", p.recentAbsenceCount},
            {"classesNeededToRecover", p.classesNeededToRecover},
            {"classesCanBeMissed", p.classesCanBeMissed},
            {"subjectWise", sw},
            {"riskCategory", p.riskCategory},
            {"riskScore", p.riskScore},
            {"contributingFactors", p.contributingFactors},
            {"recommendation", p.recommendation}
        };

        return makeJsonResponse(200, {{"profile", profileJson}});
    });

    // 9. POST /api/attendance (Mark session attendance)
    CROW_ROUTE(app, "/api/attendance")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string subjectId = body.value("subjectId", "");
            std::string topic = body.value("topic", "Lecture Session");
            auto records = body.value("records", json::array());

            if (subjectId.empty() || records.empty()) {
                return makeJsonResponse(400, {{"error", "Subject ID and records array required"}});
            }

            auto subOpt = repo.findSubjectById(subjectId);
            std::string subjectCode = subOpt ? subOpt->code : "CS301";
            std::string facultyId = subOpt ? subOpt->facultyId : "fac_demo";

            std::string sessionId = "sess_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            attendance::models::AttendanceSession session;
            session.id = sessionId;
            session.subjectId = subjectId;
            session.facultyId = facultyId;
            session.sessionDate = "2026-11-18";
            session.topic = topic;
            repo.createSession(session);

            int count = 0;
            for (const auto& r : records) {
                std::string studentId = r.value("studentId", "");
                std::string statusStr = r.value("status", "PRESENT");
                auto status = attendance::models::stringToStatus(statusStr);

                std::string recordId = "rec_" + sessionId + "_" + studentId;
                attendance::models::AttendanceRecord ar;
                ar.id = recordId;
                ar.sessionId = sessionId;
                ar.studentId = studentId;
                ar.status = status;
                ar.remarks = r.value("remarks", "");
                repo.createRecord(ar);

                auto stu = repo.findStudentById(studentId);
                std::string stuName = stu ? stu->name : "Student";

                // Push to FIFO Alert Queue
                attendance::models::AlertEvent evt;
                evt.id = "evt_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                evt.type = "RECORD_CREATED";
                evt.studentId = studentId;
                evt.studentName = stuName;
                evt.subjectCode = subjectCode;
                evt.message = "Attendance marked " + statusStr + " for " + stuName + " in " + subjectCode;
                evt.severity = (statusStr == "ABSENT") ? "WARNING" : "INFO";
                attendance::dsa::getGlobalAlertQueue().enqueue(evt);
                repo.saveAlertQueueItem(evt);

                // Push to LIFO Undo Stack
                attendance::models::UndoAction undo;
                undo.id = "undo_" + recordId;
                undo.recordId = recordId;
                undo.sessionId = sessionId;
                undo.studentId = studentId;
                undo.studentName = stuName;
                undo.subjectCode = subjectCode;
                undo.previousStatus = attendance::models::AttendanceStatus::ABSENT;
                undo.newStatus = status;
                undo.facultyId = facultyId;
                undo.reason = "Initial roll call";
                attendance::dsa::getGlobalUndoStack().push(undo);
                repo.saveUndoRecord(undo);

                count++;
            }

            return makeJsonResponse(200, {
                {"success", true},
                {"sessionId", sessionId},
                {"recordsCount", count},
                {"queueSize", attendance::dsa::getGlobalAlertQueue().size()},
                {"undoStackDepth", attendance::dsa::getGlobalUndoStack().size()}
            });
        } catch (...) {
            return makeJsonResponse(500, {{"error", "Failed to mark attendance"}});
        }
    });

    // 10. POST /api/attendance/undo
    CROW_ROUTE(app, "/api/attendance/undo")
        .methods("POST"_method)
    ([&]() {
        auto actionOpt = repo.popLatestUndoRecord();
        if (!actionOpt) {
            auto memOpt = attendance::dsa::getGlobalUndoStack().pop();
            if (!memOpt) {
                return makeJsonResponse(200, {
                    {"success", false},
                    {"message", "No operations in the AttendanceStack to undo."},
                    {"stackSize", 0}
                });
            }
            actionOpt = memOpt;
        }

        const auto& act = *actionOpt;
        repo.updateRecordStatus(act.recordId, act.previousStatus);

        return makeJsonResponse(200, {
            {"success", true},
            {"message", "Reverted attendance for " + act.studentName + " in " + act.subjectCode +
                        " back to " + attendance::models::statusToString(act.previousStatus)},
            {"remainingStackDepth", attendance::dsa::getGlobalUndoStack().size()}
        });
    });

    // 11. GET /api/attendance/undo
    CROW_ROUTE(app, "/api/attendance/undo")
    ([]() {
        auto stackArr = attendance::dsa::getGlobalUndoStack().toArray();
        json history = json::array();
        for (const auto& a : stackArr) {
            history.push_back({
                {"id", a.id},
                {"recordId", a.recordId},
                {"studentName", a.studentName},
                {"subjectCode", a.subjectCode},
                {"previousStatus", attendance::models::statusToString(a.previousStatus)},
                {"newStatus", attendance::models::statusToString(a.newStatus)},
                {"reason", a.reason}
            });
        }
        return makeJsonResponse(200, {
            {"stackSize", stackArr.size()},
            {"history", history}
        });
    });

    // 12. POST /api/prediction (What-If Simulation)
    CROW_ROUTE(app, "/api/prediction")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string studentId = body.value("studentId", "");
            int attendNext = body.value("attendNext", 5);
            int missNext = body.value("missNext", 0);
            double threshold = body.value("threshold", 75.0);

            auto profOpt = service.getStudentProfile(studentId, threshold);
            if (!profOpt) {
                return makeJsonResponse(404, {{"error", "Student profile not found"}});
            }
            const auto& p = *profOpt;

            auto sim = attendance::algorithms::simulateAttendance(
                p.totalSessionsAttended, p.totalSessionsConducted, attendNext, missNext, threshold
            );

            // Predict hypothetical projected risk
            std::vector<attendance::models::AttendanceStatus> simulatedRecords;
            for (int i = 0; i < p.totalSessionsAttended + attendNext; i++) {
                simulatedRecords.push_back(attendance::models::AttendanceStatus::PRESENT);
            }
            for (int i = 0; i < p.totalSessionsAbsent + missNext; i++) {
                simulatedRecords.push_back(attendance::models::AttendanceStatus::ABSENT);
            }
            auto projectedRisk = attendance::ml::RiskPredictor::predict(simulatedRecords, threshold);

            return makeJsonResponse(200, json{
                {"success", true},
                {"current", {
                    {"attendancePercentage", p.overallPercentage},
                    {"riskCategory", p.riskCategory},
                    {"riskScore", p.riskScore}
                }},
                {"simulation", {
                    {"additionalAttended", sim.additionalAttended},
                    {"additionalMissed", sim.additionalMissed},
                    {"projectedTotalSessions", sim.projectedTotalSessions},
                    {"projectedAttendedSessions", sim.projectedAttendedSessions},
                    {"projectedPercentage", sim.projectedPercentage},
                    {"meetsThreshold", sim.meetsThreshold},
                    {"deltaPercentage", sim.deltaPercentage}
                }},
                {"projectedRisk", {
                    {"category", attendance::models::riskToString(projectedRisk.riskCategory)},
                    {"score", projectedRisk.riskScore},
                    {"recommendation", projectedRisk.recommendation},
                    {"contributingFactors", projectedRisk.contributingFactors}
                }}
            });
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Failed to run prediction simulation"}});
        }
    });

    // 13. GET /api/dsa/benchmark
    CROW_ROUTE(app, "/api/dsa/benchmark")
    ([&]() {
        auto all = service.getAllStudentProfiles(75.0);
        const auto& profiles = all.profiles;

        // Sorting comparison
        auto compAsc = [](const attendance::models::StudentAttendanceProfile& a, const attendance::models::StudentAttendanceProfile& b) {
            return a.overallPercentage < b.overallPercentage;
        };

        auto ms = attendance::algorithms::mergeSortWithMetrics(profiles, compAsc);
        auto qs = attendance::algorithms::quickSortWithMetrics(profiles, compAsc);

        // Native std::sort benchmark
        std::vector<attendance::models::StudentAttendanceProfile> nativeArr = profiles;
        auto nStart = std::chrono::high_resolution_clock::now();
        std::sort(nativeArr.begin(), nativeArr.end(), compAsc);
        auto nEnd = std::chrono::high_resolution_clock::now();
        long long nativeUs = std::chrono::duration_cast<std::chrono::microseconds>(nEnd - nStart).count();

        // Searching comparison on roll numbers
        std::string target = "2024CS019";
        auto linSearch = attendance::algorithms::linearSearch(
            profiles,
            [&](const attendance::models::StudentAttendanceProfile& p) { return p.rollNumber == target; },
            [](const attendance::models::StudentAttendanceProfile& p) { return p.rollNumber; }
        );

        // Roll number sorted list for binary search
        auto sortedByRoll = profiles;
        std::sort(sortedByRoll.begin(), sortedByRoll.end(), [](const auto& a, const auto& b) {
            return a.rollNumber < b.rollNumber;
        });
        auto binSearch = attendance::algorithms::binarySearch(
            sortedByRoll,
            target,
            [](const attendance::models::StudentAttendanceProfile& p) { return p.rollNumber; }
        );

        // HashMap vs Array Lookup speedup benchmark
        attendance::dsa::StudentHashMap<std::string, std::string> benchMap;
        for (const auto& p : profiles) {
            benchMap.set(p.rollNumber, p.name);
        }

        int iterations = 10000;
        auto mapStart = std::chrono::high_resolution_clock::now();
        std::string outVal;
        for (int i = 0; i < iterations; i++) {
            benchMap.get("2024CS015", outVal);
        }
        auto mapEnd = std::chrono::high_resolution_clock::now();
        double mapAvgUs = std::max(0.01, std::chrono::duration_cast<std::chrono::microseconds>(mapEnd - mapStart).count() / static_cast<double>(iterations));

        auto arrStart = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            for (const auto& p : profiles) {
                if (p.rollNumber == "2024CS015") break;
            }
        }
        auto arrEnd = std::chrono::high_resolution_clock::now();
        double arrAvgUs = std::max(0.05, std::chrono::duration_cast<std::chrono::microseconds>(arrEnd - arrStart).count() / static_cast<double>(iterations));

        json linTrace = json::array();
        for (const auto& t : linSearch.trace) {
            linTrace.push_back({
                {"index", t.index},
                {"valueExamined", t.valueExamined},
                {"isMatch", t.isMatch},
                {"actionTaken", t.actionTaken}
            });
        }

        json binTrace = json::array();
        for (const auto& t : binSearch.trace) {
            binTrace.push_back({
                {"index", t.index},
                {"low", t.low},
                {"mid", t.mid},
                {"high", t.high},
                {"valueExamined", t.valueExamined},
                {"isMatch", t.isMatch},
                {"actionTaken", t.actionTaken}
            });
        }

        return makeJsonResponse(200, {
            {"success", true},
            {"datasetSize", profiles.size()},
            {"sorting", {
                {"mergeSort", {
                    {"comparisons", ms.comparisons},
                    {"swaps", ms.swaps},
                    {"durationMicroseconds", std::max(1LL, ms.durationMicroseconds)},
                    {"timeComplexity", "O(n log n)"},
                    {"spaceComplexity", "O(n)"},
                    {"isStable", true}
                }},
                {"quickSort", {
                    {"comparisons", qs.comparisons},
                    {"swaps", qs.swaps},
                    {"durationMicroseconds", std::max(1LL, qs.durationMicroseconds)},
                    {"timeComplexity", "O(n log n) avg, O(n^2) worst"},
                    {"spaceComplexity", "O(log n)"},
                    {"isStable", false}
                }},
                {"nativeSort", {
                    {"durationMicroseconds", std::max(1LL, nativeUs)},
                    {"timeComplexity", "O(n log n) (Introsort)"},
                    {"spaceComplexity", "O(log n)"},
                    {"isStable", false}
                }}
            }},
            {"searching", {
                {"target", target},
                {"linearSearch", {
                    {"found", linSearch.found},
                    {"index", linSearch.index},
                    {"stepsCount", linSearch.stepsCount},
                    {"timeMicroseconds", linSearch.timeMicroseconds},
                    {"timeComplexity", linSearch.timeComplexity},
                    {"trace", linTrace}
                }},
                {"binarySearch", {
                    {"found", binSearch.found},
                    {"index", binSearch.index},
                    {"stepsCount", binSearch.stepsCount},
                    {"timeMicroseconds", binSearch.timeMicroseconds},
                    {"timeComplexity", binSearch.timeComplexity},
                    {"trace", binTrace}
                }}
            }},
            {"lookupBenchmark", {
                {"iterations", iterations},
                {"hashMapAverageUs", mapAvgUs},
                {"arrayScanAverageUs", arrAvgUs},
                {"speedupFactor", std::max(1, static_cast<int>(arrAvgUs / mapAvgUs))},
                {"hashMapDiagnostics", all.hashMap.getDiagnostics()}
            }}
        });
    });

    // 14. GET /api/dsa/graph
    CROW_ROUTE(app, "/api/dsa/graph")
    ([&]() {
        attendance::dsa::AttendanceGraph graph;
        auto facultyList = repo.findAllFaculty();
        auto subjectList = repo.findAllSubjects();
        auto studentList = repo.findAllStudents();

        std::string rootFacultyId;
        for (const auto& f : facultyList) {
            std::string fid = "fac_" + f.id;
            if (rootFacultyId.empty()) rootFacultyId = fid;
            graph.addNode({fid, f.name, "FACULTY", {{"employeeId", f.employeeId}, {"department", f.department}}});
        }

        for (const auto& s : subjectList) {
            std::string sid = "sub_" + s.id;
            graph.addNode({sid, s.code + ": " + s.name, "SUBJECT", {{"code", s.code}, {"credits", s.credits}}});
            graph.addEdge({"fac_" + s.facultyId, sid, "TEACHES"});
        }

        for (const auto& st : studentList) {
            std::string stid = "stu_" + st.id;
            graph.addNode({stid, st.rollNumber + ": " + st.name, "STUDENT", {{"roll", st.rollNumber}}});
            auto enrolledSubs = repo.findEnrolledSubjectIds(st.id);
            for (const auto& subId : enrolledSubs) {
                graph.addEdge({"sub_" + subId, stid, "ENROLLED_IN"});
            }
        }

        auto bfsRes = graph.bfs(rootFacultyId);
        auto dfsRes = graph.dfs(rootFacultyId);

        json nodesArr = json::array();
        for (const auto& [nid, n] : graph.getNodes()) {
            nodesArr.push_back({
                {"id", n.id},
                {"label", n.label},
                {"type", n.type},
                {"metadata", n.metadata}
            });
        }

        json edgesArr = json::array();
        for (const auto& e : graph.getEdges()) {
            edgesArr.push_back({
                {"source", e.source},
                {"target", e.target},
                {"relationship", e.relationship}
            });
        }

        json levelsJson = json::object();
        for (const auto& [k, v] : bfsRes.levels) {
            levelsJson[k] = v;
        }

        return makeJsonResponse(200, {
            {"success", true},
            {"startNode", rootFacultyId},
            {"graph", {
                {"nodes", nodesArr},
                {"edges", edgesArr}
            }},
            {"traversals", {
                {"bfs", {
                    {"algorithm", "Breadth-First Search (BFS)"},
                    {"startNode", rootFacultyId},
                    {"traversalOrder", bfsRes.order},
                    {"nodesVisited", bfsRes.nodesVisited},
                    {"levels", levelsJson},
                    {"timeComplexity", "O(V + E)"},
                    {"spaceComplexity", "O(V)"},
                    {"dataStructureUsed", "Queue (FIFO)"}
                }},
                {"dfs", {
                    {"algorithm", "Depth-First Search (DFS)"},
                    {"startNode", rootFacultyId},
                    {"traversalOrder", dfsRes.order},
                    {"nodesVisited", dfsRes.nodesVisited},
                    {"timeComplexity", "O(V + E)"},
                    {"spaceComplexity", "O(V)"},
                    {"dataStructureUsed", "Stack / Recursion (LIFO)"}
                }}
            }}
        });
    });

    std::cout << "========================================================\n"
              << "🚀 AI Attendance Risk Predictor — C++ Backend (Crow)\n"
              << "========================================================\n"
              << "Listening on port " << port << " ...\n"
              << "Database connected: " << dbPath << "\n"
              << "All REST APIs and DSA benchmarks online!\n"
              << "========================================================" << std::endl;

    app.port(port).multithreaded().run();
    return 0;
}
