#include "repositories/AttendanceRepository.hpp"
#include <iostream>

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
    return stmt->step() || true; // INSERT returns done
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

bool AttendanceRepository::createStudent(const attendance::models::Student& s) {
    auto stmt = db_.prepare(
        "INSERT INTO students (id, user_id, roll_number, department, semester) VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, s.id);
    stmt->bind(2, s.userId);
    stmt->bind(3, s.rollNumber);
    stmt->bind(4, s.department);
    stmt->bind(5, s.semester);
    return stmt->step() || true;
}

std::vector<attendance::models::Faculty> AttendanceRepository::findAllFaculty() {
    std::vector<attendance::models::Faculty> list;
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.employee_id, f.department, f.created_at, f.updated_at, "
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
        f.employeeId = stmt->getString(2);
        f.department = stmt->getString(3);
        f.createdAt = stmt->getString(4);
        f.updatedAt = stmt->getString(5);
        f.name = stmt->getString(6);
        f.email = stmt->getString(7);
        list.push_back(f);
    }
    return list;
}

std::optional<attendance::models::Faculty> AttendanceRepository::findFacultyById(const std::string& id) {
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.employee_id, f.department, f.created_at, f.updated_at, "
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
        f.employeeId = stmt->getString(2);
        f.department = stmt->getString(3);
        f.createdAt = stmt->getString(4);
        f.updatedAt = stmt->getString(5);
        f.name = stmt->getString(6);
        f.email = stmt->getString(7);
        return f;
    }
    return std::nullopt;
}

std::optional<attendance::models::Faculty> AttendanceRepository::findFacultyByUserId(const std::string& userId) {
    auto stmt = db_.prepare(
        "SELECT f.id, f.user_id, f.employee_id, f.department, f.created_at, f.updated_at, "
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
        f.employeeId = stmt->getString(2);
        f.department = stmt->getString(3);
        f.createdAt = stmt->getString(4);
        f.updatedAt = stmt->getString(5);
        f.name = stmt->getString(6);
        f.email = stmt->getString(7);
        return f;
    }
    return std::nullopt;
}

std::vector<attendance::models::Subject> AttendanceRepository::findAllSubjects() {
    std::vector<attendance::models::Subject> list;
    auto stmt = db_.prepare(
        "SELECT s.id, s.code, s.name, s.faculty_id, s.credits, s.created_at, s.updated_at, "
        "       u.name, u.email, "
        "       (SELECT COUNT(*) FROM attendance_sessions WHERE subject_id = s.id) as session_count, "
        "       (SELECT COUNT(*) FROM enrollments WHERE subject_id = s.id) as student_count "
        "FROM subjects s "
        "JOIN faculty f ON f.id = s.faculty_id "
        "JOIN users u ON u.id = f.user_id "
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
        sub.facultyName = stmt->getString(7);
        sub.facultyEmail = stmt->getString(8);
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
        "JOIN faculty f ON f.id = s.faculty_id "
        "JOIN users u ON u.id = f.user_id "
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
        sub.facultyName = stmt->getString(7);
        sub.facultyEmail = stmt->getString(8);
        return sub;
    }
    return std::nullopt;
}

std::vector<std::string> AttendanceRepository::findEnrolledSubjectIds(const std::string& studentId) {
    std::vector<std::string> ids;
    auto stmt = db_.prepare("SELECT subject_id FROM enrollments WHERE student_id = ?;");
    if (!stmt) return ids;
    stmt->bind(1, studentId);
    while (stmt->step()) {
        ids.push_back(stmt->getString(0));
    }
    return ids;
}

bool AttendanceRepository::createEnrollment(const std::string& studentId, const std::string& subjectId) {
    auto stmt = db_.prepare("INSERT OR IGNORE INTO enrollments (id, student_id, subject_id) VALUES (?, ?, ?);");
    if (!stmt) return false;
    std::string id = "enr_" + studentId + "_" + subjectId;
    stmt->bind(1, id);
    stmt->bind(2, studentId);
    stmt->bind(3, subjectId);
    return stmt->step() || true;
}

bool AttendanceRepository::createSession(const attendance::models::AttendanceSession& session) {
    auto stmt = db_.prepare(
        "INSERT INTO attendance_sessions (id, subject_id, faculty_id, session_date, topic) VALUES (?, ?, ?, ?, ?);"
    );
    if (!stmt) return false;
    stmt->bind(1, session.id);
    stmt->bind(2, session.subjectId);
    stmt->bind(3, session.facultyId);
    stmt->bind(4, session.sessionDate);
    stmt->bind(5, session.topic);
    return stmt->step() || true;
}

std::vector<attendance::models::AttendanceRecord> AttendanceRepository::findRecordsByStudent(const std::string& studentId) {
    std::vector<attendance::models::AttendanceRecord> list;
    auto stmt = db_.prepare(
        "SELECT r.id, r.session_id, r.student_id, r.status, r.remarks, r.created_at, r.updated_at, "
        "       s.session_date, s.subject_id "
        "FROM attendance_records r "
        "JOIN attendance_sessions s ON s.id = r.session_id "
        "WHERE r.student_id = ? "
        "ORDER BY s.session_date ASC, r.created_at ASC;"
    );
    if (!stmt) return list;
    stmt->bind(1, studentId);

    while (stmt->step()) {
        attendance::models::AttendanceRecord rec;
        rec.id = stmt->getString(0);
        rec.sessionId = stmt->getString(1);
        rec.studentId = stmt->getString(2);
        rec.status = attendance::models::stringToStatus(stmt->getString(3));
        rec.remarks = stmt->getString(4);
        rec.createdAt = stmt->getString(5);
        rec.updatedAt = stmt->getString(6);
        rec.sessionDate = stmt->getString(7);
        rec.subjectId = stmt->getString(8);
        list.push_back(rec);
    }
    return list;
}

bool AttendanceRepository::createRecord(const attendance::models::AttendanceRecord& rec) {
    auto stmt = db_.prepare(
        "INSERT INTO attendance_records (id, session_id, student_id, status, remarks) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(session_id, student_id) DO UPDATE SET status=excluded.status, updated_at=CURRENT_TIMESTAMP;"
    );
    if (!stmt) return false;
    stmt->bind(1, rec.id);
    stmt->bind(2, rec.sessionId);
    stmt->bind(3, rec.studentId);
    stmt->bind(4, attendance::models::statusToString(rec.status));
    stmt->bind(5, rec.remarks);
    return stmt->step() || true;
}

bool AttendanceRepository::updateRecordStatus(const std::string& recordId, attendance::models::AttendanceStatus status) {
    auto stmt = db_.prepare(
        "UPDATE attendance_records SET status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;"
    );
    if (!stmt) return false;
    stmt->bind(1, attendance::models::statusToString(status));
    stmt->bind(2, recordId);
    return stmt->step() || true;
}

bool AttendanceRepository::saveUndoRecord(const attendance::models::UndoAction& action) {
    auto stmt = db_.prepare(
        "INSERT INTO undo_records (id, record_id, session_id, student_id, student_name, subject_code, "
        "                         previous_status, new_status, faculty_id, reason, is_undone) "
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
    return stmt->step() || true;
}

std::optional<attendance::models::UndoAction> AttendanceRepository::popLatestUndoRecord() {
    auto stmt = db_.prepare(
        "SELECT id, record_id, session_id, student_id, student_name, subject_code, "
        "       previous_status, new_status, timestamp, faculty_id, reason "
        "FROM undo_records "
        "WHERE is_undone = 0 "
        "ORDER BY timestamp DESC, rowid DESC LIMIT 1;"
    );
    if (!stmt) return std::nullopt;

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

        // Mark as undone in persistent database
        db_.execute("UPDATE undo_records SET is_undone = 1 WHERE id = '" + action.id + "';");
        return action;
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
    return stmt->step() || true;
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
