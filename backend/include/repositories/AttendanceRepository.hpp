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

    // Students
    std::vector<attendance::models::Student> findAllStudents();
    std::optional<attendance::models::Student> findStudentById(const std::string& id);
    std::optional<attendance::models::Student> findStudentByUserId(const std::string& userId);
    bool createStudent(const attendance::models::Student& student);

    // Faculty
    std::vector<attendance::models::Faculty> findAllFaculty();
    std::optional<attendance::models::Faculty> findFacultyById(const std::string& id);
    std::optional<attendance::models::Faculty> findFacultyByUserId(const std::string& userId);

    // Subjects
    std::vector<attendance::models::Subject> findAllSubjects();
    std::optional<attendance::models::Subject> findSubjectById(const std::string& id);
    std::optional<attendance::models::Subject> findSubjectByCode(const std::string& code);
    bool createSubject(const attendance::models::Subject& subject);

    // Enrollments
    std::vector<std::string> findEnrolledSubjectIds(const std::string& studentId);
    bool createEnrollment(const std::string& studentId, const std::string& subjectId);

    // Sessions & Records
    bool createSession(const attendance::models::AttendanceSession& session);
    std::optional<attendance::models::AttendanceSession> findSessionById(const std::string& id);
    std::vector<attendance::models::AttendanceRecord> findRecordsByStudent(const std::string& studentId);
    std::vector<attendance::models::AttendanceRecord> findRecordsByStudentDetailed(const std::string& studentId);
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
    
    // Alert Queue
    bool saveAlertQueueItem(const attendance::models::AlertEvent& event);
    std::vector<attendance::models::AlertEvent> findRecentAlerts(int limit = 10);

    attendance::db::Database& getDb() { return db_; }

private:
    attendance::db::Database& db_;
};

} // namespace attendance::repositories
