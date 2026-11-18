#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "models/Models.hpp"
#include "database/Database.hpp"

namespace attendance::repositories {

class AttendanceRepository {
public:
    explicit AttendanceRepository(attendance::db::Database& db) : db_(db) {}

    // Users
    std::optional<attendance::models::User> findUserByEmail(const std::string& email);
    std::optional<attendance::models::User> findUserById(const std::string& id);
    bool createUser(const attendance::models::User& user);

    // Server-Side Opaque Sessions
    bool createUserSession(const attendance::models::UserSession& session);
    std::optional<attendance::models::UserSession> findSessionByTokenHash(const std::string& tokenHash);
    bool revokeSession(const std::string& tokenHash);
    bool revokeUserSessions(const std::string& userId);
    int cleanExpiredSessions();

    // Audit Logging
    bool logAudit(const std::string& userId, const std::string& userEmail, const std::string& action, 
                  const std::string& entityType, const std::string& entityId, const std::string& details, 
                  const std::string& ipAddress);
    std::vector<attendance::models::AuditLog> findRecentAuditLogs(int limit = 50);

    // System Configuration
    double getAttendanceThreshold();
    bool setAttendanceThreshold(double threshold);
    std::string getSystemConfig(const std::string& key, const std::string& defaultValue = "");
    bool setSystemConfig(const std::string& key, const std::string& value);

    // Students
    std::vector<attendance::models::Student> findAllStudents();
    std::optional<attendance::models::Student> findStudentById(const std::string& id);
    std::optional<attendance::models::Student> findStudentByUserId(const std::string& userId);
    bool createStudent(const attendance::models::Student& student);
    std::vector<attendance::models::Student> searchStudents(const std::string& query, const std::string& dept = "", int semester = 0);
    std::vector<attendance::models::Student> findStudentsEnrolledInSubject(const std::string& subjectId);

    // Faculty
    std::vector<attendance::models::Faculty> findAllFaculty();
    std::optional<attendance::models::Faculty> findFacultyById(const std::string& id);
    std::optional<attendance::models::Faculty> findFacultyByUserId(const std::string& userId);
    bool createFaculty(const attendance::models::Faculty& faculty);

    // Subjects
    std::vector<attendance::models::Subject> findAllSubjects();
    std::optional<attendance::models::Subject> findSubjectById(const std::string& id);
    std::optional<attendance::models::Subject> findSubjectByCode(const std::string& code);
    bool createSubject(const attendance::models::Subject& subject);
    std::vector<attendance::models::Subject> findSubjectsByFacultyId(const std::string& facultyId);

    // Enrollments
    std::vector<std::string> findEnrolledSubjectIds(const std::string& studentId);
    bool createEnrollment(const std::string& studentId, const std::string& subjectId);

    // Sessions & Records
    bool createSession(const attendance::models::AttendanceSession& session);
    std::optional<attendance::models::AttendanceSession> findSessionById(const std::string& id);
    std::vector<attendance::models::AttendanceSession> findSessionsByFaculty(const std::string& facultyId, int limit = 50);
    std::vector<attendance::models::AttendanceSession> findSessionsBySubject(const std::string& subjectId, int limit = 50);
    
    std::vector<attendance::models::AttendanceRecord> findRecordsByStudent(const std::string& studentId);
    std::vector<attendance::models::AttendanceRecord> findRecordsByStudentDetailed(const std::string& studentId);
    std::vector<attendance::models::AttendanceRecord> findRecordsBySession(const std::string& sessionId);
    std::optional<attendance::models::AttendanceRecord> findRecordById(const std::string& recordId);
    std::optional<attendance::models::AttendanceRecord> findRecordBySessionAndStudent(const std::string& sessionId, const std::string& studentId);
    bool createRecord(const attendance::models::AttendanceRecord& record);
    bool updateRecordStatus(const std::string& recordId, attendance::models::AttendanceStatus status);
    bool deleteRecord(const std::string& recordId);

    // Persistent Undo & Queue
    bool saveUndoRecord(const attendance::models::UndoAction& action);
    std::optional<attendance::models::UndoAction> popLatestUndoRecord(const std::string& facultyId = "");
    std::vector<attendance::models::UndoAction> findActiveUndoActions(int limit = 20);
    bool markUndoActionDone(const std::string& undoId);
    bool deleteSessionIfEmpty(const std::string& sessionId);

    // Faculty Ownership & Subject Authorization Checks
    bool isFacultyAssignedToSubject(const std::string& facultyUserIdOrId, const std::string& subjectId);
    bool isStudentEnrolledInSubject(const std::string& studentId, const std::string& subjectId);
    bool isStudentEnrolledInFacultySubjects(const std::string& studentId, const std::string& facultyUserIdOrId);
    bool isRecordOwnedByFaculty(const std::string& recordId, const std::string& facultyUserIdOrId);
    std::vector<std::string> findFacultySubjectIds(const std::string& facultyUserIdOrId);
    std::vector<std::string> findFacultyStudentIds(const std::string& facultyUserIdOrId);

    // Secure Password Setup & Reset
    bool updateUserPassword(const std::string& userId, const std::string& newPasswordHash);
    std::optional<attendance::models::User> findUserBySetupToken(const std::string& setupToken);
    
    // Alert Queue
    bool saveAlertQueueItem(const attendance::models::AlertEvent& event);
    std::vector<attendance::models::AlertEvent> findRecentAlerts(int limit = 10);

    attendance::db::Database& getDb() { return db_; }

private:
    attendance::db::Database& db_;
};

} // namespace attendance::repositories
