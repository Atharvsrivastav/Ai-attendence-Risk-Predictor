#include "repositories/AttendanceRepository.hpp"
#include <iostream>
#include <chrono>

namespace attendance::repositories {

std::optional<attendance::models::User> AttendanceRepository::findUserByEmail(const std::string& email) {
    auto stmt = db_.prepare(
        "SELECT u.id, u.email, u.password_hash, u.name, u.role, u.created_at, u.updated_at, "
        "       s.id, f.id "
        "FROM users u "
        "LEFT JOIN students s ON s.user_id = u.id "
        "LEFT JOIN faculty f ON f.user_id = u.id "
        "WHERE LOWER(u.email) = LOWER(?);"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, email);

    if (stmt->step()) {
        attendance::models::User u;
        u.id = stmt->getString(0);
        u.email = stmt->getString(1);
        u.passwordHash = stmt->getString(2);
        u.name = stmt->getString(3);
        u.role = attendance::models::stringToRole(stmt->getString(4));
        u.createdAt = stmt->getString(5);
        u.updatedAt = stmt->getString(6);
        u.studentId = stmt->isNull(7) ? "" : stmt->getString(7);
        u.facultyId = stmt->isNull(8) ? "" : stmt->getString(8);
        return u;
    }
    return std::nullopt;
}

std::optional<attendance::models::User> AttendanceRepository::findUserById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT u.id, u.email, u.password_hash, u.name, u.role, u.created_at, u.updated_at, "
        "       s.id, f.id "
        "FROM users u "
        "LEFT JOIN students s ON s.user_id = u.id "
        "LEFT JOIN faculty f ON f.user_id = u.id "
        "WHERE u.id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, id);

    if (stmt->step()) {
        attendance::models::User u;
        u.id = stmt->getString(0);
        u.email = stmt->getString(1);
        u.passwordHash = stmt->getString(2);
        u.name = stmt->getString(3);
        u.role = attendance::models::stringToRole(stmt->getString(4));
        u.createdAt = stmt->getString(5);
        u.updatedAt = stmt->getString(6);
        u.studentId = stmt->isNull(7) ? "" : stmt->getString(7);
        u.facultyId = stmt->isNull(8) ? "" : stmt->getString(8);
        return u;
    }
    return std::nullopt;
}

bool AttendanceRepository::createUser(const attendance::models::User& u) {
    auto stmt = db_.prepare(
        "INSERT INTO users (id, email, password_hash, name, role) VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, u.id);
    stmt->bind(2, u.email);
    stmt->bind(3, u.passwordHash);
    stmt->bind(4, u.name);
    stmt->bind(5, attendance::models::roleToString(u.role));
    return stmt->execute();
}

std::vector<attendance::models::Student> AttendanceRepository::findAllStudents() {
    std::vector<attendance::models::Student> list;
    auto stmt = db_.prepare(
        "SELECT s.id, s.user_id, s.roll_number, s.department, s.semester, s.created_at, s.updated_at, "
        "       u.name, u.email "
        "FROM students s "
        "JOIN users u ON u.id = s.user_id "
        "ORDER BY s.roll_number ASC;"
    );
    if (!stmt) return list;

    while (stmt->step()) {
        attendance::models::Student s;
        s.id = stmt->getString(0);
        s.userId = stmt->getString(1);
        s.rollNumber = stmt->getString(2);
        s.department = stmt->getString(3);
        s.semester = stmt->getInt(4);
        s.createdAt = stmt->getString(5);
        s.updatedAt = stmt->getString(6);
        s.name = stmt->getString(7);
        s.email = stmt->getString(8);
        list.push_back(s);
    }
    return list;
}

std::optional<attendance::models::Student> AttendanceRepository::findStudentById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT s.id, s.user_id, s.roll_number, s.department, s.semester, s.created_at, s.updated_at, "
        "       u.name, u.email "
        "FROM students s "
        "JOIN users u ON u.id = s.user_id "
        "WHERE s.id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, id);

    if (stmt->step()) {
        attendance::models::Student s;
        s.id = stmt->getString(0);
        s.userId = stmt->getString(1);
        s.rollNumber = stmt->getString(2);
        s.department = stmt->getString(3);
        s.semester = stmt->getInt(4);
        s.createdAt = stmt->getString(5);
        s.updatedAt = stmt->getString(6);
        s.name = stmt->getString(7);
        s.email = stmt->getString(8);
        return s;
    }
    return std::nullopt;
}

std::optional<attendance::models::Student> AttendanceRepository::findStudentByUserId(const std::string& userId) {
    auto stmt = db_.prepare(
        "SELECT s.id, s.user_id, s.roll_number, s.department, s.semester, s.created_at, s.updated_at, "
        "       u.name, u.email "
        "FROM students s "
        "JOIN users u ON u.id = s.user_id "
        "WHERE s.user_id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, userId);

    if (stmt->step()) {
        attendance::models::Student s;
        s.id = stmt->getString(0);
        s.userId = stmt->getString(1);
        s.rollNumber = stmt->getString(2);
        s.department = stmt->getString(3);
        s.semester = stmt->getInt(4);
        s.createdAt = stmt->getString(5);
        s.updatedAt = stmt->getString(6);
        s.name = stmt->getString(7);
        s.email = stmt->getString(8);
        return s;
    }
    return std::nullopt;
}

bool AttendanceRepository::createStudent(const attendance::models::Student& student) {
    auto stmt = db_.prepare(
        "INSERT INTO students (id, user_id, roll_number, department, semester) VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, student.id);
    stmt->bind(2, student.userId);
    stmt->bind(3, student.rollNumber);
    stmt->bind(4, student.department);
    stmt->bind(5, student.semester);
    return stmt->execute();
}

std::vector<attendance::models::Faculty> AttendanceRepository::findAllFaculty() {
    std::vector<attendance::models::Faculty> list;
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.department, f.created_at, f.updated_at, "
        "       u.name, u.email "
        "FROM faculty f "
        "JOIN users u ON u.id = f.user_id "
        "ORDER BY u.name ASC;"
    );
    if (!stmt) return list;

    while (stmt->step()) {
        attendance::models::Faculty f;
        f.id = stmt->getString(0);
        f.userId = stmt->getString(1);
        f.department = stmt->getString(2);
        f.createdAt = stmt->getString(3);
        f.updatedAt = stmt->getString(4);
        f.name = stmt->getString(5);
        f.email = stmt->getString(6);
        list.push_back(f);
    }
    return list;
}

std::optional<attendance::models::Faculty> AttendanceRepository::findFacultyById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.department, f.created_at, f.updated_at, "
        "       u.name, u.email "
        "FROM faculty f "
        "JOIN users u ON u.id = f.user_id "
        "WHERE f.id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, id);

    if (stmt->step()) {
        attendance::models::Faculty f;
        f.id = stmt->getString(0);
        f.userId = stmt->getString(1);
        f.department = stmt->getString(2);
        f.createdAt = stmt->getString(3);
        f.updatedAt = stmt->getString(4);
        f.name = stmt->getString(5);
        f.email = stmt->getString(6);
        return f;
    }
    return std::nullopt;
}

std::optional<attendance::models::Faculty> AttendanceRepository::findFacultyByUserId(const std::string& userId) {
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.department, f.created_at, f.updated_at, "
        "       u.name, u.email "
        "FROM faculty f "
        "JOIN users u ON u.id = f.user_id "
        "WHERE f.user_id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, userId);

    if (stmt->step()) {
        attendance::models::Faculty f;
        f.id = stmt->getString(0);
        f.userId = stmt->getString(1);
        f.department = stmt->getString(2);
        f.createdAt = stmt->getString(3);
        f.updatedAt = stmt->getString(4);
        f.name = stmt->getString(5);
        f.email = stmt->getString(6);
        return f;
    }
    return std::nullopt;
}

std::vector<attendance::models::Subject> AttendanceRepository::findAllSubjects() {
    std::vector<attendance::models::Subject> list;
    auto stmt = db_.prepare(
        "SELECT s.id, s.code, s.name, s.faculty_id, s.credits, s.created_at, s.updated_at, "
        "       u.name, u.email, "
        "       (SELECT COUNT(*) FROM attendance_sessions ses WHERE ses.subject_id = s.id) as total_sessions, "
        "       (SELECT COUNT(*) FROM enrollments en WHERE en.subject_id = s.id) as enrolled_count "
        "FROM subjects s "
        "LEFT JOIN faculty f ON f.id = s.faculty_id "
        "LEFT JOIN users u ON u.id = f.user_id "
        "ORDER BY s.code ASC;"
    );
    if (!stmt) return list;

    while (stmt->step()) {
        attendance::models::Subject sub;
        sub.id = stmt->getString(0);
        sub.code = stmt->getString(1);
        sub.name = stmt->getString(2);
        sub.facultyId = stmt->getString(3);
        sub.credits = stmt->getInt(4);
        sub.createdAt = stmt->getString(5);
        sub.updatedAt = stmt->getString(6);
        sub.facultyName = stmt->isNull(7) ? "Unassigned" : stmt->getString(7);
        sub.facultyEmail = stmt->isNull(8) ? "" : stmt->getString(8);
        sub.totalSessions = stmt->getInt(9);
        sub.enrolledStudents = stmt->getInt(10);
        list.push_back(sub);
    }
    return list;
}

std::optional<attendance::models::Subject> AttendanceRepository::findSubjectById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT s.id, s.code, s.name, s.faculty_id, s.credits, s.created_at, s.updated_at, "
        "       u.name, u.email "
        "FROM subjects s "
        "LEFT JOIN faculty f ON f.id = s.faculty_id "
        "LEFT JOIN users u ON u.id = f.user_id "
        "WHERE s.id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, id);

    if (stmt->step()) {
        attendance::models::Subject sub;
        sub.id = stmt->getString(0);
        sub.code = stmt->getString(1);
        sub.name = stmt->getString(2);
        sub.facultyId = stmt->getString(3);
        sub.credits = stmt->getInt(4);
        sub.createdAt = stmt->getString(5);
        sub.updatedAt = stmt->getString(6);
        sub.facultyName = stmt->isNull(7) ? "Unassigned" : stmt->getString(7);
        sub.facultyEmail = stmt->isNull(8) ? "" : stmt->getString(8);
        return sub;
    }
    return std::nullopt;
}

std::optional<attendance::models::Subject> AttendanceRepository::findSubjectByCode(const std::string& code) {
    auto stmt = db_.prepare(
        "SELECT s.id, s.code, s.name, s.faculty_id, s.credits, s.created_at, s.updated_at, "
        "       u.name, u.email "
        "FROM subjects s "
        "LEFT JOIN faculty f ON f.id = s.faculty_id "
        "LEFT JOIN users u ON u.id = f.user_id "
        "WHERE UPPER(s.code) = UPPER(?);"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, code);

    if (stmt->step()) {
        attendance::models::Subject sub;
        sub.id = stmt->getString(0);
        sub.code = stmt->getString(1);
        sub.name = stmt->getString(2);
        sub.facultyId = stmt->getString(3);
        sub.credits = stmt->getInt(4);
        sub.createdAt = stmt->getString(5);
        sub.updatedAt = stmt->getString(6);
        sub.facultyName = stmt->isNull(7) ? "Unassigned" : stmt->getString(7);
        sub.facultyEmail = stmt->isNull(8) ? "" : stmt->getString(8);
        return sub;
    }
    return std::nullopt;
}

bool AttendanceRepository::createSubject(const attendance::models::Subject& subject) {
    auto stmt = db_.prepare(
        "INSERT INTO subjects (id, code, name, faculty_id, credits) VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, subject.id);
    stmt->bind(2, subject.code);
    stmt->bind(3, subject.name);
    stmt->bind(4, subject.facultyId);
    stmt->bind(5, subject.credits);
    return stmt->execute();
}

std::vector<std::string> AttendanceRepository::findEnrolledSubjectIds(const std::string& studentId) {
    std::vector<std::string> subIds;
    auto stmt = db_.prepare("SELECT subject_id FROM enrollments WHERE student_id = ?;");
    if (!stmt) return subIds;
    stmt->bind(1, studentId);

    while (stmt->step()) {
        subIds.push_back(stmt->getString(0));
    }
    return subIds;
}

bool AttendanceRepository::createEnrollment(const std::string& studentId, const std::string& subjectId) {
    std::string id = "enr_" + studentId + "_" + subjectId;
    auto stmt = db_.prepare(
        "INSERT OR IGNORE INTO enrollments (id, student_id, subject_id) VALUES (?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, id);
    stmt->bind(2, studentId);
    stmt->bind(3, subjectId);
    return stmt->execute();
}

bool AttendanceRepository::createSession(const attendance::models::AttendanceSession& session) {
    auto stmt = db_.prepare(
        "INSERT INTO attendance_sessions (id, subject_id, faculty_id, session_date, topic) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, session.id);
    stmt->bind(2, session.subjectId);
    stmt->bind(3, session.facultyId);
    stmt->bind(4, session.sessionDate);
    stmt->bind(5, session.topic);
    return stmt->execute();
}

std::optional<attendance::models::AttendanceSession> AttendanceRepository::findSessionById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT id, subject_id, faculty_id, session_date, topic, created_at "
        "FROM attendance_sessions WHERE id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, id);

    if (stmt->step()) {
        attendance::models::AttendanceSession s;
        s.id = stmt->getString(0);
        s.subjectId = stmt->getString(1);
        s.facultyId = stmt->getString(2);
        s.sessionDate = stmt->getString(3);
        s.topic = stmt->getString(4);
        s.createdAt = stmt->getString(5);
        return s;
    }
    return std::nullopt;
}

std::vector<attendance::models::AttendanceRecord> AttendanceRepository::findRecordsByStudent(const std::string& studentId) {
    std::vector<attendance::models::AttendanceRecord> records;
    auto stmt = db_.prepare(
        "SELECT r.id, r.session_id, r.student_id, r.status, r.remarks, r.created_at, "
        "       s.subject_id, s.session_date, s.topic "
        "FROM attendance_records r "
        "JOIN attendance_sessions s ON s.id = r.session_id "
        "WHERE r.student_id = ? "
        "ORDER BY s.session_date ASC;"
    );
    if (!stmt) return records;
    stmt->bind(1, studentId);

    while (stmt->step()) {
        attendance::models::AttendanceRecord r;
        r.id = stmt->getString(0);
        r.sessionId = stmt->getString(1);
        r.studentId = stmt->getString(2);
        r.status = attendance::models::stringToStatus(stmt->getString(3));
        r.remarks = stmt->isNull(4) ? "" : stmt->getString(4);
        r.createdAt = stmt->getString(5);
        r.subjectId = stmt->getString(6);
        r.sessionDate = stmt->getString(7);
        r.topic = stmt->getString(8);
        records.push_back(r);
    }
    return records;
}

std::vector<attendance::models::AttendanceRecord> AttendanceRepository::findRecordsByStudentDetailed(const std::string& studentId) {
    std::vector<attendance::models::AttendanceRecord> records;
    auto stmt = db_.prepare(
        "SELECT r.id, r.session_id, r.student_id, r.status, r.remarks, r.created_at, "
        "       s.subject_id, s.session_date, s.topic, "
        "       sub.code, sub.name, "
        "       u.name "
        "FROM attendance_records r "
        "JOIN attendance_sessions s ON s.id = r.session_id "
        "JOIN subjects sub ON sub.id = s.subject_id "
        "LEFT JOIN faculty f ON f.id = s.faculty_id "
        "LEFT JOIN users u ON u.id = f.user_id "
        "WHERE r.student_id = ? "
        "ORDER BY s.session_date ASC;"
    );
    if (!stmt) return records;
    stmt->bind(1, studentId);

    while (stmt->step()) {
        attendance::models::AttendanceRecord r;
        r.id = stmt->getString(0);
        r.sessionId = stmt->getString(1);
        r.studentId = stmt->getString(2);
        r.status = attendance::models::stringToStatus(stmt->getString(3));
        r.remarks = stmt->isNull(4) ? "" : stmt->getString(4);
        r.createdAt = stmt->getString(5);
        r.subjectId = stmt->getString(6);
        r.sessionDate = stmt->getString(7);
        r.topic = stmt->getString(8);
        r.subjectCode = stmt->getString(9);
        r.subjectName = stmt->getString(10);
        r.facultyName = stmt->isNull(11) ? "Instructor" : stmt->getString(11);
        records.push_back(r);
    }
    return records;
}

std::optional<attendance::models::AttendanceRecord> AttendanceRepository::findRecordById(const std::string& recordId) {
    auto stmt = db_.prepare(
        "SELECT r.id, r.session_id, r.student_id, r.status, r.remarks, r.created_at, "
        "       s.subject_id, s.session_date, s.topic, "
        "       sub.code, sub.name, "
        "       u.name, s.faculty_id "
        "FROM attendance_records r "
        "JOIN attendance_sessions s ON s.id = r.session_id "
        "JOIN subjects sub ON sub.id = s.subject_id "
        "LEFT JOIN faculty f ON f.id = s.faculty_id "
        "LEFT JOIN users u ON u.id = f.user_id "
        "WHERE r.id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, recordId);

    if (stmt->step()) {
        attendance::models::AttendanceRecord r;
        r.id = stmt->getString(0);
        r.sessionId = stmt->getString(1);
        r.studentId = stmt->getString(2);
        r.status = attendance::models::stringToStatus(stmt->getString(3));
        r.remarks = stmt->isNull(4) ? "" : stmt->getString(4);
        r.createdAt = stmt->getString(5);
        r.subjectId = stmt->getString(6);
        r.sessionDate = stmt->getString(7);
        r.topic = stmt->getString(8);
        r.subjectCode = stmt->getString(9);
        r.subjectName = stmt->getString(10);
        r.facultyName = stmt->isNull(11) ? "" : stmt->getString(11);
        return r;
    }
    return std::nullopt;
}

std::optional<attendance::models::AttendanceRecord> AttendanceRepository::findRecordBySessionAndStudent(
    const std::string& sessionId, const std::string& studentId) 
{
    auto stmt = db_.prepare(
        "SELECT id, session_id, student_id, status, remarks, created_at "
        "FROM attendance_records "
        "WHERE session_id = ? AND student_id = ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, sessionId);
    stmt->bind(2, studentId);

    if (stmt->step()) {
        attendance::models::AttendanceRecord r;
        r.id = stmt->getString(0);
        r.sessionId = stmt->getString(1);
        r.studentId = stmt->getString(2);
        r.status = attendance::models::stringToStatus(stmt->getString(3));
        r.remarks = stmt->isNull(4) ? "" : stmt->getString(4);
        r.createdAt = stmt->getString(5);
        return r;
    }
    return std::nullopt;
}

bool AttendanceRepository::createRecord(const attendance::models::AttendanceRecord& record) {
    auto stmt = db_.prepare(
        "INSERT INTO attendance_records (id, session_id, student_id, status, remarks) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, record.id);
    stmt->bind(2, record.sessionId);
    stmt->bind(3, record.studentId);
    stmt->bind(4, attendance::models::statusToString(record.status));
    stmt->bind(5, record.remarks);
    return stmt->execute();
}

bool AttendanceRepository::updateRecordStatus(const std::string& recordId, attendance::models::AttendanceStatus status) {
    auto stmt = db_.prepare(
        "UPDATE attendance_records SET status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;"
    );
    if (!stmt) return false;
    stmt->bind(1, attendance::models::statusToString(status));
    stmt->bind(2, recordId);
    return stmt->execute();
}

bool AttendanceRepository::deleteRecord(const std::string& recordId) {
    auto stmt = db_.prepare("DELETE FROM attendance_records WHERE id = ?;");
    if (!stmt) return false;
    stmt->bind(1, recordId);
    return stmt->execute();
}

bool AttendanceRepository::saveUndoRecord(const attendance::models::UndoAction& action) {
    auto stmt = db_.prepare(
        "INSERT OR REPLACE INTO undo_records (id, record_id, session_id, student_id, student_name, subject_code, "
        "                                     previous_status, new_status, faculty_id, reason, is_undone) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0);"
    );
    if (!stmt) return false;
    stmt->bind(1, action.id);
    stmt->bind(2, action.recordId);
    stmt->bind(3, action.sessionId);
    stmt->bind(4, action.studentId);
    stmt->bind(5, action.studentName);
    stmt->bind(6, action.subjectCode);
    stmt->bind(7, attendance::models::statusToString(action.previousStatus));
    stmt->bind(8, attendance::models::statusToString(action.newStatus));
    stmt->bind(9, action.facultyId);
    stmt->bind(10, action.reason);
    return stmt->execute();
}

std::optional<attendance::models::UndoAction> AttendanceRepository::popLatestUndoRecord(const std::string& facultyId) {
    std::string sql = 
        "SELECT id, record_id, session_id, student_id, student_name, subject_code, "
        "       previous_status, new_status, timestamp, faculty_id, reason "
        "FROM undo_records "
        "WHERE is_undone = 0 ";
    if (!facultyId.empty()) {
        sql += "AND faculty_id = ? ";
    }
    sql += "ORDER BY rowid DESC LIMIT 1;";

    auto stmt = db_.prepare(sql);
    if (!stmt) return std::nullopt;
    if (!facultyId.empty()) {
        stmt->bind(1, facultyId);
    }

    if (stmt->step()) {
        attendance::models::UndoAction action;
        action.id = stmt->getString(0);
        action.recordId = stmt->getString(1);
        action.sessionId = stmt->getString(2);
        action.studentId = stmt->getString(3);
        action.studentName = stmt->getString(4);
        action.subjectCode = stmt->getString(5);
        action.previousStatus = attendance::models::stringToStatus(stmt->getString(6));
        action.newStatus = attendance::models::stringToStatus(stmt->getString(7));
        action.timestamp = stmt->getString(8);
        action.facultyId = stmt->getString(9);
        action.reason = stmt->getString(10);
        return action;
    }
    return std::nullopt;
}

std::vector<attendance::models::UndoAction> AttendanceRepository::findActiveUndoActions(int limit) {
    std::vector<attendance::models::UndoAction> list;
    auto stmt = db_.prepare(
        "SELECT id, record_id, session_id, student_id, student_name, subject_code, "
        "       previous_status, new_status, timestamp, faculty_id, reason "
        "FROM undo_records "
        "WHERE is_undone = 0 "
        "ORDER BY rowid DESC LIMIT " + std::to_string(limit) + ";"
    );
    if (!stmt) return list;

    while (stmt->step()) {
        attendance::models::UndoAction action;
        action.id = stmt->getString(0);
        action.recordId = stmt->getString(1);
        action.sessionId = stmt->getString(2);
        action.studentId = stmt->getString(3);
        action.studentName = stmt->getString(4);
        action.subjectCode = stmt->getString(5);
        action.previousStatus = attendance::models::stringToStatus(stmt->getString(6));
        action.newStatus = attendance::models::stringToStatus(stmt->getString(7));
        action.timestamp = stmt->getString(8);
        action.facultyId = stmt->getString(9);
        action.reason = stmt->getString(10);
        list.push_back(action);
    }
    return list;
}

bool AttendanceRepository::markUndoActionDone(const std::string& undoId) {
    auto stmt = db_.prepare("UPDATE undo_records SET is_undone = 1 WHERE id = ?;");
    if (!stmt) return false;
    stmt->bind(1, undoId);
    return stmt->execute();
}

bool AttendanceRepository::deleteSessionIfEmpty(const std::string& sessionId) {
    auto stmt = db_.prepare("SELECT COUNT(*) FROM attendance_records WHERE session_id = ?;");
    if (!stmt) return false;
    stmt->bind(1, sessionId);
    if (stmt->step() && stmt->getInt(0) == 0) {
        auto delStmt = db_.prepare("DELETE FROM attendance_sessions WHERE id = ?;");
        if (delStmt) {
            delStmt->bind(1, sessionId);
            return delStmt->execute();
        }
    }
    return false;
}

bool AttendanceRepository::isFacultyAssignedToSubject(const std::string& facultyUserIdOrId, const std::string& subjectId) {
    if (facultyUserIdOrId.empty() || subjectId.empty()) return false;
    auto stmt = db_.prepare(
        "SELECT 1 FROM subjects s "
        "LEFT JOIN faculty f ON s.faculty_id = f.id "
        "WHERE s.id = ? AND (s.faculty_id = ? OR f.user_id = ? OR f.id = ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, subjectId);
    stmt->bind(2, facultyUserIdOrId);
    stmt->bind(3, facultyUserIdOrId);
    stmt->bind(4, facultyUserIdOrId);
    return stmt->step();
}

bool AttendanceRepository::isStudentEnrolledInSubject(const std::string& studentId, const std::string& subjectId) {
    if (studentId.empty() || subjectId.empty()) return false;
    auto stmt = db_.prepare("SELECT 1 FROM enrollments WHERE student_id = ? AND subject_id = ?;");
    if (!stmt) return false;
    stmt->bind(1, studentId);
    stmt->bind(2, subjectId);
    return stmt->step();
}

bool AttendanceRepository::isStudentEnrolledInFacultySubjects(const std::string& studentId, const std::string& facultyUserIdOrId) {
    if (studentId.empty() || facultyUserIdOrId.empty()) return false;
    auto stmt = db_.prepare(
        "SELECT 1 FROM enrollments e "
        "JOIN subjects s ON e.subject_id = s.id "
        "LEFT JOIN faculty f ON s.faculty_id = f.id "
        "WHERE e.student_id = ? AND (s.faculty_id = ? OR f.user_id = ? OR f.id = ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, studentId);
    stmt->bind(2, facultyUserIdOrId);
    stmt->bind(3, facultyUserIdOrId);
    stmt->bind(4, facultyUserIdOrId);
    return stmt->step();
}

bool AttendanceRepository::isRecordOwnedByFaculty(const std::string& recordId, const std::string& facultyUserIdOrId) {
    if (recordId.empty() || facultyUserIdOrId.empty()) return false;
    auto stmt = db_.prepare(
        "SELECT 1 FROM attendance_records rec "
        "JOIN attendance_sessions ses ON rec.session_id = ses.id "
        "JOIN subjects sub ON ses.subject_id = sub.id "
        "LEFT JOIN faculty f ON sub.faculty_id = f.id "
        "WHERE rec.id = ? AND (ses.faculty_id = ? OR sub.faculty_id = ? OR f.user_id = ? OR f.id = ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, recordId);
    stmt->bind(2, facultyUserIdOrId);
    stmt->bind(3, facultyUserIdOrId);
    stmt->bind(4, facultyUserIdOrId);
    stmt->bind(5, facultyUserIdOrId);
    return stmt->step();
}

std::vector<std::string> AttendanceRepository::findFacultySubjectIds(const std::string& facultyUserIdOrId) {
    std::vector<std::string> ids;
    if (facultyUserIdOrId.empty()) return ids;
    auto stmt = db_.prepare(
        "SELECT s.id FROM subjects s "
        "LEFT JOIN faculty f ON s.faculty_id = f.id "
        "WHERE s.faculty_id = ? OR f.user_id = ? OR f.id = ?;"
    );
    if (!stmt) return ids;
    stmt->bind(1, facultyUserIdOrId);
    stmt->bind(2, facultyUserIdOrId);
    stmt->bind(3, facultyUserIdOrId);
    while (stmt->step()) {
        ids.push_back(stmt->getString(0));
    }
    return ids;
}

std::vector<std::string> AttendanceRepository::findFacultyStudentIds(const std::string& facultyUserIdOrId) {
    std::vector<std::string> ids;
    if (facultyUserIdOrId.empty()) return ids;
    auto stmt = db_.prepare(
        "SELECT DISTINCT e.student_id FROM enrollments e "
        "JOIN subjects s ON e.subject_id = s.id "
        "LEFT JOIN faculty f ON s.faculty_id = f.id "
        "WHERE s.faculty_id = ? OR f.user_id = ? OR f.id = ?;"
    );
    if (!stmt) return ids;
    stmt->bind(1, facultyUserIdOrId);
    stmt->bind(2, facultyUserIdOrId);
    stmt->bind(3, facultyUserIdOrId);
    while (stmt->step()) {
        ids.push_back(stmt->getString(0));
    }
    return ids;
}

bool AttendanceRepository::updateUserPassword(const std::string& userId, const std::string& newPasswordHash) {
    auto stmt = db_.prepare("UPDATE users SET password_hash = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;");
    if (!stmt) return false;
    stmt->bind(1, newPasswordHash);
    stmt->bind(2, userId);
    return stmt->execute();
}

std::optional<attendance::models::User> AttendanceRepository::findUserBySetupToken(const std::string& setupToken) {
    if (setupToken.empty()) return std::nullopt;
    std::string pattern = "%!SETUP_REQUIRED:" + setupToken + ":%";
    auto stmt = db_.prepare(
        "SELECT u.id, u.email, u.password_hash, u.name, u.role, u.created_at, u.updated_at, "
        "       s.id AS studentId, f.id AS facultyId "
        "FROM users u "
        "LEFT JOIN students s ON u.id = s.user_id "
        "LEFT JOIN faculty f ON u.id = f.user_id "
        "WHERE u.password_hash LIKE ?;"
    );
    if (!stmt) return std::nullopt;
    stmt->bind(1, pattern);
    if (stmt->step()) {
        attendance::models::User u;
        u.id = stmt->getString(0);
        u.email = stmt->getString(1);
        u.passwordHash = stmt->getString(2);
        u.name = stmt->getString(3);
        u.role = attendance::models::stringToRole(stmt->getString(4));
        u.createdAt = stmt->getString(5);
        u.updatedAt = stmt->getString(6);
        u.studentId = stmt->isNull(7) ? "" : stmt->getString(7);
        u.facultyId = stmt->isNull(8) ? "" : stmt->getString(8);
        return u;
    }
    return std::nullopt;
}

bool AttendanceRepository::saveAlertQueueItem(const attendance::models::AlertEvent& event) {
    auto stmt = db_.prepare(
        "INSERT INTO alert_queue_items (id, event_type, student_id, student_name, subject_code, message, severity, is_processed) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 0);"
    );
    if (!stmt) return false;
    stmt->bind(1, event.id);
    stmt->bind(2, event.type);
    stmt->bind(3, event.studentId);
    stmt->bind(4, event.studentName);
    stmt->bind(5, event.subjectCode);
    stmt->bind(6, event.message);
    stmt->bind(7, event.severity);
    return stmt->execute();
}

std::vector<attendance::models::AlertEvent> AttendanceRepository::findRecentAlerts(int limit) {
    std::vector<attendance::models::AlertEvent> list;
    auto stmt = db_.prepare(
        "SELECT id, event_type, student_id, student_name, subject_code, message, timestamp, severity "
        "FROM alert_queue_items "
        "ORDER BY timestamp DESC LIMIT " + std::to_string(limit) + ";"
    );
    if (!stmt) return list;

    while (stmt->step()) {
        attendance::models::AlertEvent ev;
        ev.id = stmt->getString(0);
        ev.type = stmt->getString(1);
        ev.studentId = stmt->getString(2);
        ev.studentName = stmt->getString(3);
        ev.subjectCode = stmt->getString(4);
        ev.message = stmt->getString(5);
        ev.timestamp = stmt->getString(6);
        ev.severity = stmt->getString(7);
        list.push_back(ev);
    }
    return list;
}

} // namespace attendance::repositories
