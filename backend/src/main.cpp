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

// Authenticate caller
static std::optional<attendance::auth::SessionClaims> authenticateRequest(const crow::request& req) {
    std::string token = extractToken(req);
    if (token.empty()) return std::nullopt;
    return attendance::auth::AuthService::verifyToken(token);
}

// Role-based authorization checker
static bool checkRole(const std::optional<attendance::auth::SessionClaims>& claims,
                      const std::vector<attendance::models::UserRole>& allowedRoles) 
{
    if (!claims.has_value()) return false;
    for (const auto& r : allowedRoles) {
        if (claims->role == r) return true;
    }
    return false;
}

// Student record privacy authorization: Students can only view their own data
static bool checkStudentAccess(const std::optional<attendance::auth::SessionClaims>& claims,
                               const std::string& targetStudentId) 
{
    if (!claims.has_value()) return false;
    if (claims->role == attendance::models::UserRole::ADMIN || 
        claims->role == attendance::models::UserRole::FACULTY) 
    {
        return true;
    }
    if (claims->role == attendance::models::UserRole::STUDENT) {
        return claims->studentId == targetStudentId;
    }
    return false;
}

// CORS helper to append headers
static crow::response makeJsonResponse(int code, const json& j) {
    crow::response res(code, j.dump());
    res.set_header("Content-Type", "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie");
    res.set_header("Access-Control-Allow-Credentials", "true");
    return res;
}

int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    int port = 8080;
    const char* envPort = std::getenv("BACKEND_PORT");
    if (envPort) {
        try { port = std::stoi(envPort); } catch (...) {}
    }

    std::string dbPath = "database/attendance.db";
    const char* envDb = std::getenv("CPP_DATABASE_PATH");
    if (envDb) {
        dbPath = std::string(envDb);
    }

    auto& db = attendance::db::Database::instance();
    if (!db.open(dbPath)) {
        std::cerr << "Fatal: Could not open database at " << dbPath << std::endl;
        return 1;
    }

    attendance::repositories::AttendanceRepository repo(db);
    attendance::services::AttendanceService service(repo);

    // Warm up in-memory AttendanceStack from persistent undo history on server startup
    auto activeUndos = repo.findActiveUndoActions(50);
    // Push in chronological order so the newest ends up on top of LIFO stack
    for (auto it = activeUndos.rbegin(); it != activeUndos.rend(); ++it) {
        attendance::dsa::getGlobalUndoStack().push(*it);
    }

    // CORS preflight OPTIONS route handler
    CROW_ROUTE(app, "/api/<path>")
        .methods("OPTIONS"_method)
    ([](const crow::request&, std::string) {
        crow::response res(204);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie");
        res.set_header("Access-Control-Allow-Credentials", "true");
        return res;
    });

    // 1. Health Check
    CROW_ROUTE(app, "/health")
    ([]() {
        json j = {
            {"status", "healthy"},
            {"engine", "C++20 Crow REST Server with RBAC"},
            {"version", "2.1.0"}
        };
        return makeJsonResponse(200, j);
    });

    // 2. POST /api/auth/login
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

            // Valid password authenticated! Generate signed cryptographic JWT
            attendance::auth::SessionClaims claims;
            claims.userId = user.id;
            claims.email = user.email;
            claims.name = user.name;
            claims.role = user.role;
            claims.studentId = user.studentId;
            claims.facultyId = user.facultyId;

            std::string token = attendance::auth::AuthService::generateToken(claims);

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
                {"token", token},
                {"user", userJson}
            });

            std::string cookieStr = "auth_token=" + token + "; Path=/; HttpOnly; SameSite=Lax; Max-Age=" + std::to_string(7 * 24 * 3600);
            res.add_header("Set-Cookie", cookieStr);
            return res;
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Invalid JSON payload"}});
        }
    });

    // 3. POST /api/auth/logout
    CROW_ROUTE(app, "/api/auth/logout")
        .methods("POST"_method)
    ([]() {
        auto res = makeJsonResponse(200, {{"success", true}, {"message", "Logged out successfully"}});
        res.add_header("Set-Cookie", "auth_token=; Path=/; HttpOnly; SameSite=Lax; Max-Age=0");
        return res;
    });

    // 4. GET /api/auth/me
    CROW_ROUTE(app, "/api/auth/me")
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
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

    // 5. GET /api/analytics
    CROW_ROUTE(app, "/api/analytics")
    ([&](const crow::request& req) {
        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }
        auto analytics = service.getClassAnalytics(threshold);
        return makeJsonResponse(200, analytics);
    });

    // 6. GET /api/subjects
    CROW_ROUTE(app, "/api/subjects")
    ([&]() {
        auto subjects = repo.findAllSubjects();
        json arr = json::array();
        for (const auto& s : subjects) {
            arr.push_back({
                {"id", s.id},
                {"code", s.code},
                {"name", s.name},
                {"credits", s.credits},
                {"facultyId", s.facultyId},
                {"facultyName", s.facultyName},
                {"facultyEmail", s.facultyEmail},
                {"totalSessions", s.totalSessions},
                {"enrolledCount", s.enrolledStudents},
                {"enrolledStudents", s.enrolledStudents},
                {"averageAttendance", s.averageAttendance}
            });
        }
        return makeJsonResponse(200, {{"subjects", arr}});
    });

    // 7. POST /api/subjects (Protected: Admin or Faculty)
    CROW_ROUTE(app, "/api/subjects")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin or Faculty role required"}});
        }

        try {
            auto body = json::parse(req.body);
            std::string code = body.value("code", "");
            std::string name = body.value("name", "");
            int credits = body.value("credits", 3);
            std::string facultyId = body.value("facultyId", "");

            if (code.empty() || name.empty()) {
                return makeJsonResponse(400, {{"error", "Subject code and name are required"}});
            }

            if (facultyId.empty()) {
                facultyId = claims->facultyId;
            }

            auto existingSub = repo.findSubjectByCode(code);
            if (existingSub) {
                return makeJsonResponse(400, {{"error", "Subject code already exists"}});
            }

            std::string subId = "sub_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            attendance::models::Subject s;
            s.id = subId;
            s.code = code;
            s.name = name;
            s.facultyId = facultyId;
            s.credits = credits;

            if (!repo.createSubject(s)) {
                return makeJsonResponse(500, {{"error", "Failed to create subject in database"}});
            }

            return makeJsonResponse(201, {
                {"success", true},
                {"subject", {
                    {"id", s.id},
                    {"code", s.code},
                    {"name", s.name},
                    {"credits", s.credits},
                    {"facultyId", s.facultyId}
                }}
            });
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Invalid JSON payload"}});
        }
    });

    // 8. GET /api/students (Protected: Admin or Faculty)
    CROW_ROUTE(app, "/api/students")
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        // If caller is authenticated as Student, they cannot browse the entire student directory
        if (claims.has_value() && claims->role == attendance::models::UserRole::STUDENT) {
            return makeJsonResponse(403, {{"error", "Forbidden: Students cannot browse peer directory"}});
        }

        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }

        std::string subjectId = req.url_params.get("subjectId") ? req.url_params.get("subjectId") : "";
        std::string riskFilter = req.url_params.get("risk") ? req.url_params.get("risk") : "";
        std::string search = req.url_params.get("search") ? req.url_params.get("search") : "";
        std::string sortBy = req.url_params.get("sortBy") ? req.url_params.get("sortBy") : "";
        std::string sortOrder = req.url_params.get("sortOrder") ? req.url_params.get("sortOrder") : "asc";

        auto allStudents = repo.findAllStudents();
        std::vector<attendance::models::StudentAttendanceProfile> profiles;

        for (const auto& s : allStudents) {
            auto profOpt = service.getStudentProfile(s.id, threshold);
            if (!profOpt) continue;
            const auto& p = *profOpt;

            if (!subjectId.empty() && p.subjectWise.find(subjectId) == p.subjectWise.end()) {
                continue;
            }
            if (!riskFilter.empty() && p.riskCategory != riskFilter) {
                continue;
            }
            if (!search.empty()) {
                std::string sLow = search;
                std::transform(sLow.begin(), sLow.end(), sLow.begin(), ::tolower);
                std::string nLow = p.name;
                std::transform(nLow.begin(), nLow.end(), nLow.begin(), ::tolower);
                std::string rLow = p.rollNumber;
                std::transform(rLow.begin(), rLow.end(), rLow.begin(), ::tolower);

                if (nLow.find(sLow) == std::string::npos && rLow.find(sLow) == std::string::npos) {
                    continue;
                }
            }
            profiles.push_back(p);
        }

        // Sorting
        if (sortBy == "attendance") {
            if (sortOrder == "desc") {
                auto sorted = attendance::algorithms::mergeSortWithMetrics(profiles, [](const auto& a, const auto& b) {
                    return a.overallPercentage > b.overallPercentage;
                });
                profiles = sorted.sorted;
            } else {
                auto sorted = attendance::algorithms::mergeSortWithMetrics(profiles, [](const auto& a, const auto& b) {
                    return a.overallPercentage < b.overallPercentage;
                });
                profiles = sorted.sorted;
            }
        } else if (sortBy == "name") {
            auto sorted = attendance::algorithms::quickSortWithMetrics(profiles, [sortOrder](const auto& a, const auto& b) {
                return sortOrder == "desc" ? a.name > b.name : a.name < b.name;
            });
            profiles = sorted.sorted;
        }

        json stuArray = json::array();
        for (const auto& p : profiles) {
            json subMap = json::object();
            for (const auto& [k, v] : p.subjectWise) {
                subMap[k] = {
                    {"subjectCode", v.subjectCode},
                    {"subjectName", v.subjectName},
                    {"percentage", v.percentage},
                    {"isBelowThreshold", v.isBelowThreshold},
                    {"presentCount", v.presentCount},
                    {"absentCount", v.absentCount},
                    {"consecutiveAbsences", v.consecutiveAbsences}
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
                {"riskCategory", p.riskCategory},
                {"riskScore", p.riskScore},
                {"contributingFactors", p.contributingFactors},
                {"recommendation", p.recommendation},
                {"subjectWise", subMap}
            });
        }

        return makeJsonResponse(200, {
            {"total", stuArray.size()},
            {"students", stuArray}
        });
    });

    // 9. POST /api/students (Protected: Admin or Faculty only)
    CROW_ROUTE(app, "/api/students")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin or Faculty role required"}});
        }

        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");
            std::string email = body.value("email", "");
            std::string rollNumber = body.value("rollNumber", "");
            std::string department = body.value("department", "CSE");
            int semester = body.value("semester", 5);

            if (name.empty() || email.empty() || rollNumber.empty()) {
                return makeJsonResponse(400, {{"error", "Name, email, and roll number are required"}});
            }

            auto existingUser = repo.findUserByEmail(email);
            if (existingUser) {
                return makeJsonResponse(400, {{"error", "Email already exists"}});
            }

            attendance::db::Database::ScopedTransaction tx(db);

            std::string userId = "usr_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::string studentId = "stu_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

            attendance::models::User u;
            u.id = userId;
            u.email = email;
            u.name = name;
            u.role = attendance::models::UserRole::STUDENT;
            u.passwordHash = attendance::auth::AuthService::hashPassword("student123");

            if (!repo.createUser(u)) {
                return makeJsonResponse(500, {{"error", "Failed to create user record"}});
            }

            attendance::models::Student s;
            s.id = studentId;
            s.userId = userId;
            s.rollNumber = rollNumber;
            s.department = department;
            s.semester = semester;

            if (!repo.createStudent(s)) {
                return makeJsonResponse(500, {{"error", "Failed to create student record"}});
            }

            auto subjects = repo.findAllSubjects();
            for (const auto& sub : subjects) {
                repo.createEnrollment(studentId, sub.id);
            }

            tx.commit();

            return makeJsonResponse(201, {
                {"success", true},
                {"student", {
                    {"id", studentId},
                    {"userId", userId},
                    {"rollNumber", rollNumber},
                    {"name", name},
                    {"email", email}
                }}
            });
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Failed to enroll student"}});
        }
    });

    // 10. GET /api/students/<id> (Protected: Student can only access their own profile; Admin/Faculty can access any)
    CROW_ROUTE(app, "/api/students/<string>")
    ([&](const crow::request& req, std::string studentId) {
        auto claims = authenticateRequest(req);
        if (claims.has_value() && !checkStudentAccess(claims, studentId)) {
            return makeJsonResponse(403, {{"error", "Forbidden: You are only permitted to view your own attendance profile"}});
        }

        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }

        auto profOpt = service.getStudentProfile(studentId, threshold);
        if (!profOpt) {
            return makeJsonResponse(404, {{"error", "Student not found"}});
        }

        const auto& p = *profOpt;
        json subMap = json::object();
        for (const auto& [k, v] : p.subjectWise) {
            subMap[k] = {
                {"subjectCode", v.subjectCode},
                {"subjectName", v.subjectName},
                {"percentage", v.percentage},
                {"isBelowThreshold", v.isBelowThreshold},
                {"presentCount", v.presentCount},
                {"absentCount", v.absentCount},
                {"consecutiveAbsences", v.consecutiveAbsences},
                {"classesNeededToRecover", v.classesNeededToRecover},
                {"classesCanBeMissed", v.classesCanBeMissed}
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
            {"riskCategory", p.riskCategory},
            {"riskScore", p.riskScore},
            {"contributingFactors", p.contributingFactors},
            {"recommendation", p.recommendation},
            {"subjectWise", subMap}
        };

        return makeJsonResponse(200, {{"profile", profileJson}});
    });

    // 11. GET /api/attendance/<studentId> (Protected: Student privacy check)
    CROW_ROUTE(app, "/api/attendance/<string>")
    ([&](const crow::request& req, std::string studentId) {
        auto claims = authenticateRequest(req);

        // Guard against Crow routing /api/attendance/undo to this handler
        if (studentId == "undo") {
            if (!claims) {
                return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
            }
            if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
                return makeJsonResponse(403, {{"error", "Forbidden: Faculty or Admin role required"}});
            }

            auto activeUndos = repo.findActiveUndoActions(20);
            json history = json::array();
            for (const auto& a : activeUndos) {
                history.push_back({
                    {"id", a.id},
                    {"recordId", a.recordId},
                    {"studentName", a.studentName},
                    {"subjectCode", a.subjectCode},
                    {"previousStatus", attendance::models::statusToString(a.previousStatus)},
                    {"newStatus", attendance::models::statusToString(a.newStatus)},
                    {"reason", a.reason},
                    {"facultyId", a.facultyId}
                });
            }

            return makeJsonResponse(200, {
                {"undoStackSize", activeUndos.size()},
                {"stackSize", activeUndos.size()},
                {"history", history}
            });
        }

        if (claims.has_value() && !checkStudentAccess(claims, studentId)) {
            return makeJsonResponse(403, {{"error", "Forbidden: You are only permitted to view your own attendance records"}});
        }

        auto detailedRecords = repo.findRecordsByStudentDetailed(studentId);
        json recArray = json::array();
        for (const auto& r : detailedRecords) {
            recArray.push_back({
                {"id", r.id},
                {"sessionId", r.sessionId},
                {"sessionDate", r.sessionDate},
                {"subjectId", r.subjectId},
                {"subjectCode", r.subjectCode},
                {"subjectName", r.subjectName},
                {"facultyName", r.facultyName},
                {"topic", r.topic},
                {"status", attendance::models::statusToString(r.status)},
                {"remarks", r.remarks}
            });
        }

        return makeJsonResponse(200, {
            {"records", recArray},
            {"count", recArray.size()}
        });
    });

    // 12. POST /api/attendance (Protected: Admin or Faculty only)
    CROW_ROUTE(app, "/api/attendance")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Faculty or Admin role required"}});
        }

        try {
            auto body = json::parse(req.body);
            std::string subjectId = body.value("subjectId", "");
            auto records = body.value("records", json::array());

            if (subjectId.empty() || records.empty() || !records.is_array()) {
                return makeJsonResponse(400, {{"error", "Subject ID and records array required"}});
            }

            auto subOpt = repo.findSubjectById(subjectId);
            std::string subjectCode = subOpt ? subOpt->code : "SUB";
            std::string facultyId = claims->facultyId.empty() ? (subOpt ? subOpt->facultyId : claims->userId) : claims->facultyId;

            attendance::db::Database::ScopedTransaction tx(db);

            std::string sessionId = "ses_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            attendance::models::AttendanceSession session;
            session.id = sessionId;
            session.subjectId = subjectId;
            session.facultyId = facultyId;
            session.sessionDate = "2026-11-18";
            session.topic = body.value("topic", "Lecture Session");
            if (!repo.createSession(session)) {
                return makeJsonResponse(500, {{"error", "Failed to create attendance session"}});
            }

            int count = 0;
            for (const auto& r : records) {
                std::string studentId = r.value("studentId", "");
                std::string statusStr = r.value("status", "PRESENT");
                auto status = attendance::models::stringToStatus(statusStr);

                // Check if an existing record was previously marked for this session/student
                auto existingRec = repo.findRecordBySessionAndStudent(sessionId, studentId);
                attendance::models::AttendanceStatus previousStatus = 
                    existingRec ? existingRec->status : attendance::models::AttendanceStatus::EXCUSED;

                std::string recordId = "rec_" + sessionId + "_" + studentId;
                attendance::models::AttendanceRecord ar;
                ar.id = recordId;
                ar.sessionId = sessionId;
                ar.studentId = studentId;
                ar.status = status;
                ar.remarks = r.value("remarks", "");
                if (!repo.createRecord(ar)) {
                    continue;
                }

                auto stu = repo.findStudentById(studentId);
                std::string stuName = stu ? stu->name : "Student";

                // Push to FIFO Alert Queue
                attendance::models::AlertEvent evt;
                evt.id = "evt_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "_" + studentId;
                evt.type = "RECORD_CREATED";
                evt.studentId = studentId;
                evt.studentName = stuName;
                evt.subjectCode = subjectCode;
                evt.message = "Attendance marked " + statusStr + " for " + stuName + " in " + subjectCode;
                evt.severity = (statusStr == "ABSENT") ? "WARNING" : "INFO";
                attendance::dsa::getGlobalAlertQueue().enqueue(evt);
                repo.saveAlertQueueItem(evt);

                // Push to LIFO Undo Stack with exact previousStatus and newStatus
                attendance::models::UndoAction undo;
                undo.id = "undo_" + recordId;
                undo.recordId = recordId;
                undo.sessionId = sessionId;
                undo.studentId = studentId;
                undo.studentName = stuName;
                undo.subjectCode = subjectCode;
                undo.previousStatus = previousStatus;
                undo.newStatus = status;
                undo.facultyId = claims->userId;
                undo.reason = "Roll call submission";
                attendance::dsa::getGlobalUndoStack().push(undo);
                repo.saveUndoRecord(undo);

                count++;
            }

            tx.commit();

            return makeJsonResponse(200, {
                {"success", true},
                {"sessionId", sessionId},
                {"recordsCount", count},
                {"recordsProcessed", count},
                {"undoStackDepth", attendance::dsa::getGlobalUndoStack().size()}
            });
        } catch (...) {
            return makeJsonResponse(500, {{"error", "Failed to mark attendance"}});
        }
    });

    // 13. PATCH /api/attendance (Protected: Admin or Faculty only - Status Correction)
    CROW_ROUTE(app, "/api/attendance")
        .methods("PATCH"_method)
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Faculty or Admin role required"}});
        }

        try {
            auto body = json::parse(req.body);
            std::string recordId = body.value("recordId", "");
            std::string newStatusStr = body.value("newStatus", "");
            std::string reason = body.value("reason", "Manual correction by faculty");

            if (recordId.empty() || newStatusStr.empty()) {
                return makeJsonResponse(400, {{"error", "Record ID and newStatus are required"}});
            }

            auto existingOpt = repo.findRecordById(recordId);
            if (!existingOpt) {
                return makeJsonResponse(404, {{"error", "Attendance record not found"}});
            }

            const auto& existing = *existingOpt;
            auto previousStatus = existing.status;
            auto newStatus = attendance::models::stringToStatus(newStatusStr);

            attendance::db::Database::ScopedTransaction tx(db);

            if (!repo.updateRecordStatus(recordId, newStatus)) {
                return makeJsonResponse(500, {{"error", "Failed to update record in database"}});
            }

            // Push state change to Undo Stack
            attendance::models::UndoAction undo;
            undo.id = "undo_patch_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            undo.recordId = existing.id;
            undo.sessionId = existing.sessionId;
            undo.studentId = existing.studentId;
            undo.studentName = existing.remarks; // Or student name
            auto stuOpt = repo.findStudentById(existing.studentId);
            if (stuOpt) undo.studentName = stuOpt->name;
            undo.subjectCode = existing.subjectCode;
            undo.previousStatus = previousStatus;
            undo.newStatus = newStatus;
            undo.facultyId = claims->userId;
            undo.reason = reason;

            attendance::dsa::getGlobalUndoStack().push(undo);
            repo.saveUndoRecord(undo);

            // Enqueue event notification
            attendance::models::AlertEvent evt;
            evt.id = "evt_correct_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            evt.type = "ATTENDANCE_CORRECTED";
            evt.studentId = existing.studentId;
            evt.studentName = undo.studentName;
            evt.subjectCode = undo.subjectCode;
            evt.message = "Status corrected from " + attendance::models::statusToString(previousStatus) + 
                          " to " + newStatusStr + " for " + undo.studentName;
            evt.severity = "INFO";
            attendance::dsa::getGlobalAlertQueue().enqueue(evt);
            repo.saveAlertQueueItem(evt);

            tx.commit();

            return makeJsonResponse(200, {
                {"success", true},
                {"recordId", recordId},
                {"previousStatus", attendance::models::statusToString(previousStatus)},
                {"newStatus", newStatusStr},
                {"undoStackDepth", attendance::dsa::getGlobalUndoStack().size()}
            });
        } catch (...) {
            return makeJsonResponse(500, {{"error", "Failed to correct attendance"}});
        }
    });

    // 14. POST /api/attendance/undo (Protected: Admin or Faculty only)
    CROW_ROUTE(app, "/api/attendance/undo")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Faculty or Admin role required"}});
        }

        std::string facultyFilter = (claims->role == attendance::models::UserRole::ADMIN) ? "" : claims->userId;

        attendance::db::Database::ScopedTransaction tx(db);

        // Fetch latest active undo record
        auto actionOpt = repo.popLatestUndoRecord(facultyFilter);
        if (!actionOpt) {
            return makeJsonResponse(200, {
                {"success", false},
                {"message", "No active operations in the AttendanceStack to undo."},
                {"stackSize", 0}
            });
        }

        const auto& act = *actionOpt;

        // Revert the record in the database
        if (!repo.updateRecordStatus(act.recordId, act.previousStatus)) {
            return makeJsonResponse(500, {{"error", "Failed to revert record in database"}});
        }

        // Mark undo action as completed in SQLite
        if (!repo.markUndoActionDone(act.id)) {
            return makeJsonResponse(500, {{"error", "Failed to update undo record state"}});
        }

        // Pop from in-memory stack
        attendance::dsa::getGlobalUndoStack().pop();

        tx.commit();

        return makeJsonResponse(200, {
            {"success", true},
            {"message", "Reverted attendance for " + act.studentName + " in " + act.subjectCode +
                        " back to " + attendance::models::statusToString(act.previousStatus) + 
                        " (was " + attendance::models::statusToString(act.newStatus) + ")"},
            {"undoneAction", {
                {"id", act.id},
                {"recordId", act.recordId},
                {"studentName", act.studentName},
                {"subjectCode", act.subjectCode},
                {"previousStatus", attendance::models::statusToString(act.previousStatus)},
                {"newStatus", attendance::models::statusToString(act.newStatus)}
            }},
            {"remainingStackDepth", attendance::dsa::getGlobalUndoStack().size()}
        });
    });

    // 15. GET /api/attendance/undo (Protected: Admin or Faculty only)
    CROW_ROUTE(app, "/api/attendance/undo")
    ([&](const crow::request& req) {
        auto claims = authenticateRequest(req);
        if (!claims) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Authentication required"}});
        }
        if (!checkRole(claims, {attendance::models::UserRole::ADMIN, attendance::models::UserRole::FACULTY})) {
            return makeJsonResponse(403, {{"error", "Forbidden: Faculty or Admin role required"}});
        }

        auto activeUndos = repo.findActiveUndoActions(20);
        json history = json::array();
        for (const auto& a : activeUndos) {
            history.push_back({
                {"id", a.id},
                {"recordId", a.recordId},
                {"studentName", a.studentName},
                {"subjectCode", a.subjectCode},
                {"previousStatus", attendance::models::statusToString(a.previousStatus)},
                {"newStatus", attendance::models::statusToString(a.newStatus)},
                {"reason", a.reason},
                {"facultyId", a.facultyId}
            });
        }

        return makeJsonResponse(200, {
            {"undoStackSize", activeUndos.size()},
            {"stackSize", activeUndos.size()},
            {"history", history}
        });
    });

    // 16. GET /api/risk/<studentId> (Protected: Student privacy check)
    CROW_ROUTE(app, "/api/risk/<string>")
    ([&](const crow::request& req, std::string studentId) {
        auto claims = authenticateRequest(req);
        if (claims.has_value() && !checkStudentAccess(claims, studentId)) {
            return makeJsonResponse(403, {{"error", "Forbidden: You are only permitted to view your own risk predictions"}});
        }

        double threshold = 75.0;
        if (req.url_params.get("threshold")) {
            try { threshold = std::stod(req.url_params.get("threshold")); } catch (...) {}
        }

        auto profOpt = service.getStudentProfile(studentId, threshold);
        if (!profOpt) {
            return makeJsonResponse(404, {{"error", "Student profile not found"}});
        }

        const auto& p = *profOpt;
        auto studentRecords = repo.findRecordsByStudent(studentId);
        std::vector<attendance::models::AttendanceStatus> statuses;
        for (const auto& r : studentRecords) {
            statuses.push_back(r.status);
        }

        auto prediction = attendance::ml::RiskPredictor::predict(statuses, threshold);

        return makeJsonResponse(200, {
            {"studentId", studentId},
            {"studentName", p.name},
            {"rollNumber", p.rollNumber},
            {"attendancePercentage", p.overallPercentage},
            {"threshold", threshold},
            {"prediction", {
                {"category", attendance::models::riskToString(prediction.riskCategory)},
                {"score", prediction.riskScore},
                {"confidence", prediction.confidence},
                {"recommendation", prediction.recommendation},
                {"contributingFactors", prediction.contributingFactors},
                {"methodology", prediction.methodology}
            }}
        });
    });

    // 17. POST /api/prediction (What-If Simulation; Protected)
    CROW_ROUTE(app, "/api/prediction")
        .methods("POST"_method)
    ([&](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string studentId = body.value("studentId", "");
            int attendNext = body.value("attendNext", 5);
            int missNext = body.value("missNext", 0);
            double threshold = body.value("threshold", 75.0);

            auto claims = authenticateRequest(req);
            if (claims.has_value() && !studentId.empty() && !checkStudentAccess(claims, studentId)) {
                return makeJsonResponse(403, {{"error", "Forbidden: Students can only run simulations for themselves"}});
            }

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
                    {"confidence", projectedRisk.confidence},
                    {"recommendation", projectedRisk.recommendation},
                    {"contributingFactors", projectedRisk.contributingFactors},
                    {"methodology", projectedRisk.methodology}
                }}
            });
        } catch (...) {
            return makeJsonResponse(400, {{"error", "Failed to run prediction simulation"}});
        }
    });

    // 18. GET /api/dsa/benchmark
    CROW_ROUTE(app, "/api/dsa/benchmark")
    ([&](const crow::request& req) {
        auto allStudents = repo.findAllStudents();
        std::vector<attendance::models::StudentAttendanceProfile> profiles;
        for (const auto& s : allStudents) {
            auto profOpt = service.getStudentProfile(s.id, 75.0);
            if (profOpt) profiles.push_back(*profOpt);
        }

        // Sorting Benchmark
        auto mergeSortRes = attendance::algorithms::mergeSortWithMetrics(
            profiles,
            [](const auto& a, const auto& b) { return a.overallPercentage < b.overallPercentage; }
        );

        auto quickSortRes = attendance::algorithms::quickSortWithMetrics(
            profiles,
            [](const auto& a, const auto& b) { return a.overallPercentage < b.overallPercentage; }
        );

        // Searching Benchmark
        auto sortedProfiles = mergeSortRes.sorted;
        std::string targetKey = sortedProfiles.empty() ? "" : sortedProfiles[sortedProfiles.size() / 2].rollNumber;

        auto linearSearchRes = attendance::algorithms::linearSearch(
            profiles,
            [&targetKey](const auto& p) { return p.rollNumber == targetKey; },
            [](const auto& p) { return p.rollNumber; }
        );

        auto binarySearchRes = attendance::algorithms::binarySearch(
            sortedProfiles,
            targetKey,
            [](const auto& p) { return p.rollNumber; }
        );

        // StudentHashMap amortized lookup diagnostics
        attendance::dsa::StudentHashMap<std::string, attendance::models::StudentAttendanceProfile> map;
        for (const auto& p : profiles) {
            map.set(p.rollNumber, p);
        }
        json lookupBench = map.getDiagnostics();
        lookupBench["structure"] = "StudentHashMap";
        lookupBench["hashAlgorithm"] = "djb2 polynomial rolling hash";
        lookupBench["amortizedTimeComplexity"] = "O(1)";

        return makeJsonResponse(200, json{
            {"success", true},
            {"datasetSize", profiles.size()},
            {"sorting", {
                {"mergeSort", {
                    {"algorithm", "Merge Sort"},
                    {"durationMicroseconds", mergeSortRes.durationMicroseconds},
                    {"comparisons", mergeSortRes.comparisons},
                    {"swaps", mergeSortRes.swaps},
                    {"timeComplexity", "O(n log n)"},
                    {"spaceComplexity", "O(n)"}
                }},
                {"quickSort", {
                    {"algorithm", "Quick Sort"},
                    {"durationMicroseconds", quickSortRes.durationMicroseconds},
                    {"comparisons", quickSortRes.comparisons},
                    {"swaps", quickSortRes.swaps},
                    {"timeComplexity", "O(n log n) avg, O(n^2) worst"},
                    {"spaceComplexity", "O(log n)"}
                }}
            }},
            {"searching", {
                {"linearSearch", {
                    {"algorithm", linearSearchRes.algorithm},
                    {"found", linearSearchRes.found},
                    {"stepsCount", linearSearchRes.stepsCount},
                    {"timeMicroseconds", linearSearchRes.timeMicroseconds},
                    {"timeComplexity", linearSearchRes.timeComplexity}
                }},
                {"binarySearch", {
                    {"algorithm", binarySearchRes.algorithm},
                    {"found", binarySearchRes.found},
                    {"stepsCount", binarySearchRes.stepsCount},
                    {"timeMicroseconds", binarySearchRes.timeMicroseconds},
                    {"timeComplexity", binarySearchRes.timeComplexity}
                }}
            }},
            {"lookupBenchmark", lookupBench}
        });
    });

    // 19. GET /api/dsa/graph
    CROW_ROUTE(app, "/api/dsa/graph")
    ([&](const crow::request& req) {
        attendance::dsa::AttendanceGraph graph;

        auto facultyList = repo.findAllFaculty();
        for (const auto& f : facultyList) {
            graph.addNode({
                "FACULTY_" + f.id,
                f.name,
                "FACULTY",
                {{"department", f.department}, {"email", f.email}}
            });
        }

        auto subjectList = repo.findAllSubjects();
        for (const auto& sub : subjectList) {
            graph.addNode({
                "SUBJECT_" + sub.id,
                sub.code + " - " + sub.name,
                "SUBJECT",
                {{"code", sub.code}, {"credits", sub.credits}}
            });

            if (!sub.facultyId.empty()) {
                graph.addEdge({"FACULTY_" + sub.facultyId, "SUBJECT_" + sub.id, "TEACHES"}, true);
            }
        }

        auto studentList = repo.findAllStudents();
        for (const auto& stu : studentList) {
            graph.addNode({
                "STUDENT_" + stu.id,
                stu.name + " (" + stu.rollNumber + ")",
                "STUDENT",
                {{"rollNumber", stu.rollNumber}, {"department", stu.department}}
            });

            auto enrolledSubIds = repo.findEnrolledSubjectIds(stu.id);
            for (const auto& subId : enrolledSubIds) {
                graph.addEdge({"SUBJECT_" + subId, "STUDENT_" + stu.id, "ENROLLED_IN"}, true);
            }
        }

        std::string startNode = req.url_params.get("start") ? req.url_params.get("start") : "";
        if (startNode.empty() && !facultyList.empty()) {
            startNode = "FACULTY_" + facultyList[0].id;
        }

        auto bfsResult = graph.bfs(startNode);
        auto dfsResult = graph.dfs(startNode);

        json levelsJson = json::object();
        for (const auto& [k, v] : bfsResult.levels) {
            levelsJson[k] = v;
        }

        auto graphJson = graph.toJson();

        return makeJsonResponse(200, json{
            {"success", true},
            {"startNode", startNode},
            {"graph", graphJson},
            {"traversals", {
                {"bfs", {
                    {"order", bfsResult.order},
                    {"nodesVisited", bfsResult.nodesVisited},
                    {"levels", levelsJson}
                }},
                {"dfs", {
                    {"order", dfsResult.order},
                    {"nodesVisited", dfsResult.nodesVisited}
                }}
            }}
        });
    });

    std::cout << "========================================================\n";
    std::cout << "🚀 AI Attendance Risk Predictor — C++ Backend (Crow)\n";
    std::cout << "========================================================\n";
    std::cout << "Listening on port " << port << " ...\n";
    std::cout << "Database connected: " << dbPath << "\n";
    std::cout << "Security: Bcrypt Auth + HS256 Signed JWT + RBAC Enforced\n";
    std::cout << "All REST APIs and DSA benchmarks online!\n";
    std::cout << "========================================================\n";

    app.port(port).multithreaded().run();

    return 0;
}
