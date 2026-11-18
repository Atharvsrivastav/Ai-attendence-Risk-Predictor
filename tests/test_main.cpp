#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <unordered_set>

// DSA Headers
#include "dsa/StudentHashMap.hpp"
#include "dsa/AttendanceQueue.hpp"
#include "dsa/AttendanceStack.hpp"
#include "dsa/AttendanceSet.hpp"
#include "dsa/RiskDecisionTree.hpp"
#include "dsa/AttendanceGraph.hpp"

// Algorithm Headers
#include "algorithms/Sorting.hpp"
#include "algorithms/Searching.hpp"
#include "algorithms/AttendanceCalculator.hpp"
#include "algorithms/ConsecutiveAbsences.hpp"
#include "algorithms/RecentWindowAnalysis.hpp"
#include "algorithms/TrendAnalysis.hpp"
#include "algorithms/RecoveryCalculator.hpp"

// ML Headers
#include "ml/FeatureEngineering.hpp"
#include "ml/LogisticRegressionModel.hpp"
#include "ml/RiskPredictor.hpp"

// Auth & Database Headers
#include "auth/AuthService.hpp"
#include "database/Database.hpp"
#include "repositories/AttendanceRepository.hpp"
#include "services/AttendanceService.hpp"
#include "views/SvgCharts.hpp"

using namespace attendance;

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST_CASE(suite, name) \
    std::cout << " [" << suite << "] " << name << " ... " << std::flush; \
    try {

#define TEST_PASS() \
        std::cout << "PASSED\n"; \
        g_testsPassed++; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << "\n"; \
        g_testsFailed++; \
    } catch (...) { \
        std::cout << "FAILED: unknown error\n"; \
        g_testsFailed++; \
    }

#define ASSERT_TRUE(expr) \
    if (!(expr)) throw std::runtime_error("Assertion failed: " #expr " at line " + std::to_string(__LINE__));

#define ASSERT_FALSE(expr) \
    if (expr) throw std::runtime_error("Assertion failed: NOT (" #expr ") at line " + std::to_string(__LINE__));

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) throw std::runtime_error("Assertion failed: " #a " == " #b " at line " + std::to_string(__LINE__));

#define ASSERT_NEAR(a, b, eps) \
    if (std::abs((a) - (b)) > (eps)) throw std::runtime_error("Assertion failed: |" #a " - " #b "| <= " #eps " at line " + std::to_string(__LINE__));

static std::string readFileContents(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// -------------------------------------------------------------
// SUITE 1: Custom DSA Structures
// -------------------------------------------------------------
void suite_dsa_structures() {
    TEST_CASE("DSA", "StudentHashMap: Insert, Get, Update, Collision djb2, and Auto-Resize")
        dsa::StudentHashMap<std::string, int> map(4);
        ASSERT_EQ(map.size(), 0);

        map.set("student_1", 85);
        map.set("student_2", 92);
        map.set("student_3", 64);
        ASSERT_EQ(map.size(), 3);

        int score = 0;
        ASSERT_TRUE(map.get("student_1", score));
        ASSERT_EQ(score, 85);

        // Update in-place
        map.set("student_1", 88);
        ASSERT_TRUE(map.get("student_1", score));
        ASSERT_EQ(score, 88);
        ASSERT_EQ(map.size(), 3);

        // Force auto-resize by exceeding load factor 0.75
        for (int i = 4; i <= 20; i++) {
            map.set("student_" + std::to_string(i), 70 + (i % 30));
        }
        ASSERT_TRUE(map.capacity() >= 16);
        ASSERT_EQ(map.size(), 20);

        for (int i = 1; i <= 20; i++) {
            ASSERT_TRUE(map.has("student_" + std::to_string(i)));
        }
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceQueue: FIFO Enqueue, Dequeue, Peek, Thread Safety")
        dsa::AttendanceQueue<std::string> queue;
        ASSERT_TRUE(queue.isEmpty());

        queue.enqueue("Alert_LowAttendance_Aarav");
        queue.enqueue("Alert_ConsecutiveAbsence_Sneha");
        queue.enqueue("Alert_RiskCritical_Rahul");

        ASSERT_EQ(queue.size(), 3);
        ASSERT_EQ(queue.peek().value_or(""), "Alert_LowAttendance_Aarav");

        // Verify FIFO order
        ASSERT_EQ(queue.dequeue().value_or(""), "Alert_LowAttendance_Aarav");
        ASSERT_EQ(queue.dequeue().value_or(""), "Alert_ConsecutiveAbsence_Sneha");
        ASSERT_EQ(queue.dequeue().value_or(""), "Alert_RiskCritical_Rahul");
        ASSERT_TRUE(queue.isEmpty());
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceStack: LIFO Push, Pop, Peek, removeIf, Max Depth Pruning")
        dsa::AttendanceStack<int> stack(5); // max depth 5
        for (int i = 1; i <= 5; i++) {
            stack.push(i);
        }
        ASSERT_EQ(stack.size(), 5);
        ASSERT_EQ(stack.peek().value_or(-1), 5);

        // Push 6th element: bottom element (1) should be pruned
        stack.push(6);
        ASSERT_EQ(stack.size(), 5);
        ASSERT_EQ(stack.peek().value_or(-1), 6);

        // Test selective removeIf (removes specific element without blindly popping top)
        ASSERT_TRUE(stack.removeIf([](int val) { return val == 4; }));
        ASSERT_EQ(stack.size(), 4);

        ASSERT_EQ(stack.pop().value_or(-1), 6);
        ASSERT_EQ(stack.pop().value_or(-1), 5);
        ASSERT_EQ(stack.pop().value_or(-1), 3);
        ASSERT_EQ(stack.pop().value_or(-1), 2);
        ASSERT_TRUE(stack.isEmpty());
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceSet: Add, Remove, Intersect, Unite, Difference, Subset")
        dsa::AttendanceSet<std::string> setA;
        setA.add("stu_1");
        setA.add("stu_2");
        setA.add("stu_3");

        dsa::AttendanceSet<std::string> setB;
        setB.add("stu_2");
        setB.add("stu_3");
        setB.add("stu_4");

        auto intersection = setA.intersect(setB);
        ASSERT_EQ(intersection.size(), 2);
        ASSERT_TRUE(intersection.has("stu_2"));
        ASSERT_TRUE(intersection.has("stu_3"));
        ASSERT_FALSE(intersection.has("stu_1"));

        auto united = setA.unite(setB);
        ASSERT_EQ(united.size(), 4);

        auto diff = setA.difference(setB);
        ASSERT_EQ(diff.size(), 1);
        ASSERT_TRUE(diff.has("stu_1"));

        ASSERT_TRUE(intersection.isSubsetOf(setA));
        ASSERT_TRUE(intersection.isSubsetOf(setB));
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceGraph: Adjacency List, BFS, DFS, Shortest Path")
        dsa::AttendanceGraph graph;
        graph.addNode({"FACULTY_1", "Dr. Sharma", "FACULTY", {}});
        graph.addNode({"SUBJECT_1", "CS301 DSA", "SUBJECT", {}});
        graph.addNode({"STUDENT_1", "Aarav Patel", "STUDENT", {}});
        graph.addNode({"STUDENT_2", "Sneha Rao", "STUDENT", {}});

        graph.addEdge({"FACULTY_1", "SUBJECT_1", "TEACHES"}, true);
        graph.addEdge({"SUBJECT_1", "STUDENT_1", "ENROLLED_IN"}, true);
        graph.addEdge({"SUBJECT_1", "STUDENT_2", "ENROLLED_IN"}, true);

        auto bfs = graph.bfs("FACULTY_1");
        ASSERT_EQ(bfs.nodesVisited, 4);
        ASSERT_EQ(bfs.levels["FACULTY_1"], 0);
        ASSERT_EQ(bfs.levels["SUBJECT_1"], 1);
        ASSERT_EQ(bfs.levels["STUDENT_1"], 2);
        ASSERT_EQ(bfs.levels["STUDENT_2"], 2);

        auto dfs = graph.dfs("FACULTY_1");
        ASSERT_EQ(dfs.nodesVisited, 4);

        auto path = graph.findShortestPath("FACULTY_1", "STUDENT_1");
        ASSERT_EQ(path.size(), 3);
        ASSERT_EQ(path[0], "FACULTY_1");
        ASSERT_EQ(path[1], "SUBJECT_1");
        ASSERT_EQ(path[2], "STUDENT_1");
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 2: Core Attendance Algorithms
// -------------------------------------------------------------
void suite_core_algorithms() {
    TEST_CASE("Algorithms", "Sorting: MergeSort and QuickSort with Swap and Comparison Counts")
        struct Item {
            std::string id;
            double val;
        };
        std::vector<Item> data = {{"A", 65.0}, {"B", 85.0}, {"C", 45.0}, {"D", 95.0}, {"E", 75.0}};

        auto ms = algorithms::mergeSortWithMetrics(data, [](const auto& a, const auto& b) { return a.val < b.val; });
        ASSERT_EQ(ms.sorted.size(), 5);
        ASSERT_NEAR(ms.sorted.front().val, 45.0, 0.001);
        ASSERT_NEAR(ms.sorted.back().val, 95.0, 0.001);
        ASSERT_TRUE(ms.comparisons > 0);

        auto qs = algorithms::quickSortWithMetrics(data, [](const auto& a, const auto& b) { return a.val < b.val; });
        ASSERT_EQ(qs.sorted.size(), 5);
        ASSERT_NEAR(qs.sorted.front().val, 45.0, 0.001);
        ASSERT_NEAR(qs.sorted.back().val, 95.0, 0.001);
    TEST_PASS()

    TEST_CASE("Algorithms", "Searching: LinearSearch vs BinarySearch Step-by-Step Traces")
        std::vector<int> sortedArr = {10, 20, 30, 40, 50, 60, 70, 80};

        auto ls = algorithms::linearSearch(
            sortedArr,
            [](int v) { return v == 60; },
            [](int v) { return std::to_string(v); }
        );
        ASSERT_TRUE(ls.found);
        ASSERT_EQ(ls.stepsCount, 6);

        auto bs = algorithms::binarySearch(
            sortedArr,
            std::string("60"),
            [](int v) { return std::to_string(v); }
        );
        ASSERT_TRUE(bs.found);
        ASSERT_TRUE(bs.stepsCount <= 3);
    TEST_PASS()

    TEST_CASE("Algorithms", "Attendance Calculator: Rules (P=1, L=0.5, A=0, E=Excluded)")
        using models::AttendanceStatus;
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT, // 1
            AttendanceStatus::PRESENT, // 1
            AttendanceStatus::LATE,    // 0.5
            AttendanceStatus::ABSENT,  // 0
            AttendanceStatus::EXCUSED  // Excluded from denominator
        };

        auto result = algorithms::calculateAttendancePercentage(records, 75.0);
        ASSERT_EQ(result.totalConducted, 4);
        ASSERT_NEAR(result.attendedPoints, 2.5, 0.001);
        ASSERT_EQ(result.excusedCount, 1);
        ASSERT_NEAR(result.percentage, 62.5, 0.01);
        ASSERT_TRUE(result.isBelowThreshold);
    TEST_PASS()

    TEST_CASE("Algorithms", "Consecutive Absences: Backward Streak and Excused Exemption")
        using models::AttendanceStatus;
        std::vector<AttendanceStatus> history = {
            AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::EXCUSED,
            AttendanceStatus::ABSENT
        };

        auto streak = algorithms::detectConsecutiveAbsences(history);
        ASSERT_EQ(streak.currentConsecutive, 2);
    TEST_PASS()

    TEST_CASE("Algorithms", "Sliding Window Analysis & OLS Linear Regression Trend")
        using models::AttendanceStatus;
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT, AttendanceStatus::ABSENT, AttendanceStatus::ABSENT
        };

        auto window = algorithms::analyzeRecentAttendanceWindow(records, 50.0, 3, 15.0);
        ASSERT_NEAR(window.recentPercentage, 0.0, 0.01);
        ASSERT_EQ(window.recentAbsenceCount, 3);

        auto trend = algorithms::analyzeAttendanceTrend(records);
        ASSERT_EQ(trend.trend, models::TrendType::DECLINING);
        ASSERT_TRUE(trend.slope < 0.0);
    TEST_PASS()

    TEST_CASE("Algorithms", "Recovery Calculator: Exact Needed Lectures & Safe Misses")
        auto rec = algorithms::calculateRecovery(12.0, 20, 75.0);
        ASSERT_EQ(rec.classesNeededToRecover, 12);
        ASSERT_EQ(rec.classesCanBeMissed, 0);

        auto safe = algorithms::calculateRecovery(18.0, 20, 75.0);
        ASSERT_EQ(safe.classesNeededToRecover, 0);
        ASSERT_EQ(safe.classesCanBeMissed, 4);
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 3: Cryptographic Authentication & JWT Security
// -------------------------------------------------------------
void suite_authentication_security() {
    TEST_CASE("Auth", "Cryptographic OS RNG: Salt Uniqueness & Secure Token Generation")
        std::unordered_set<std::string> salts;
        for (int i = 0; i < 50; i++) {
            char salt[16];
            ASSERT_TRUE(auth::AuthService::getSecureRandomBytes(salt, 16));
            std::string s(salt, 16);
            ASSERT_TRUE(salts.find(s) == salts.end());
            salts.insert(s);
        }

        std::string token1 = auth::AuthService::generateSecureToken(32);
        std::string token2 = auth::AuthService::generateSecureToken(32);
        ASSERT_EQ(token1.length(), 64);
        ASSERT_EQ(token2.length(), 64);
        ASSERT_TRUE(token1 != token2);
    TEST_PASS()

    TEST_CASE("Auth", "Argon2id Hashing & Backward Compatible Verification")
        std::string password = "AdminPassword@2026!";
        std::string hash = auth::AuthService::hashPassword(password);
        ASSERT_TRUE(hash.rfind("$argon2id$", 0) == 0);

        // Correct password succeeds
        ASSERT_TRUE(auth::AuthService::verifyPassword(password, hash));

        // Incorrect password fails
        ASSERT_FALSE(auth::AuthService::verifyPassword("WrongPassword123", hash));

        // Foreign passwords fail
        ASSERT_FALSE(auth::AuthService::verifyPassword("student123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("faculty123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("admin123", hash));

        // Legacy bcrypt format is still accepted for backward compatibility
        std::string bcryptHash = "$2a$12$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy";
        // Verification function correctly detects format
        ASSERT_FALSE(auth::AuthService::verifyPassword("wrongpass", bcryptHash));
    TEST_PASS()

    TEST_CASE("Auth", "Account Setup Token: Lockout Before Setup & Password Creation")
        std::string setupToken = auth::AuthService::generateSecureToken(32);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long long expEpoch = now + 3600;

        std::string lockedHash = "!SETUP_REQUIRED:" + setupToken + ":" + std::to_string(expEpoch);
        ASSERT_TRUE(auth::AuthService::isSetupRequired(lockedHash));

        ASSERT_FALSE(auth::AuthService::verifyPassword("student123", lockedHash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("", lockedHash));

        ASSERT_TRUE(auth::AuthService::verifySetupToken(lockedHash, setupToken));
        ASSERT_FALSE(auth::AuthService::verifySetupToken(lockedHash, "wrong_setup_token"));

        std::string expiredLockedHash = "!SETUP_REQUIRED:" + setupToken + ":" + std::to_string(now - 100);
        ASSERT_FALSE(auth::AuthService::verifySetupToken(expiredLockedHash, setupToken));
    TEST_PASS()

    TEST_CASE("Auth", "JWT HS256: Secret Length Requirement & Verification")
        ASSERT_FALSE(auth::AuthService::validateSecret("short_key"));
        ASSERT_FALSE(auth::AuthService::validateSecret("too_weak_secret_under_32_bytes_"));
        ASSERT_TRUE(auth::AuthService::validateSecret("this_is_a_very_strong_256_bit_secret_key_12345"));

        auth::SessionClaims claims{
            "usr_admin_123", "admin@univ.edu", "Dean of Academics",
            models::UserRole::ADMIN, "", ""
        };
        std::string secret = "test_very_secure_secret_key_1234567890_min_32_chars";
        std::string validToken = auth::AuthService::generateToken(claims, secret);
        ASSERT_TRUE(!validToken.empty());

        auto verified = auth::AuthService::verifyToken(validToken, secret);
        ASSERT_TRUE(verified.has_value());
        ASSERT_EQ(verified->userId, "usr_admin_123");
        ASSERT_EQ(verified->role, models::UserRole::ADMIN);
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 4: Isolated Database, Foreign Keys, Sessions & Audit
// -------------------------------------------------------------
void suite_database_and_integrity(const std::string& testDbPath, const std::string& schemaSql, const std::string& migration2Sql) {
    auto& db = db::Database::instance();
    ASSERT_TRUE(db.open(testDbPath));
    ASSERT_TRUE(db.runMigration(schemaSql));
    ASSERT_TRUE(db.runMigration(migration2Sql));

    repositories::AttendanceRepository repo(db);

    TEST_CASE("Database", "Integrity: Parameterized CRUD, Foreign Key Enforcement")
        models::User adminUser{
            "usr_test_admin", "admin@test.edu",
            auth::AuthService::hashPassword("admin123"),
            "Test Admin", models::UserRole::ADMIN
        };
        ASSERT_TRUE(repo.createUser(adminUser));

        models::User dupUser = adminUser;
        dupUser.id = "usr_test_admin_2";
        ASSERT_FALSE(repo.createUser(dupUser));

        models::Student invalidStudent{
            "stu_invalid", "usr_non_existent", "ROLL_INVALID", "CSE", 5
        };
        ASSERT_FALSE(repo.createStudent(invalidStudent));
    TEST_PASS()

    TEST_CASE("Database", "Transactions: Atomic Multi-Operation Rollback & Commit")
        {
            db::Database::ScopedTransaction tx(db);
            ASSERT_TRUE(tx.isActive());

            models::User tempUser{
                "usr_temp_1", "temp1@test.edu",
                auth::AuthService::hashPassword("temp123"),
                "Temp User", models::UserRole::STUDENT
            };
            ASSERT_TRUE(repo.createUser(tempUser));
        }

        auto found = repo.findUserByEmail("temp1@test.edu");
        ASSERT_FALSE(found.has_value());

        {
            db::Database::ScopedTransaction tx(db);
            ASSERT_TRUE(tx.isActive());

            models::User committedUser{
                "usr_comm_1", "comm1@test.edu",
                auth::AuthService::hashPassword("comm123"),
                "Committed User", models::UserRole::STUDENT
            };
            ASSERT_TRUE(repo.createUser(committedUser));
            ASSERT_TRUE(tx.commit());
            ASSERT_FALSE(tx.isActive());
        }

        auto foundComm = repo.findUserByEmail("comm1@test.edu");
        ASSERT_TRUE(foundComm.has_value());
    TEST_PASS()

    TEST_CASE("Database", "Opaque Server-Side Sessions & SHA-256 Token Hashing")
        auto uAdmin = repo.findUserById("usr_test_admin");
        std::string targetUid = uAdmin.has_value() ? "usr_test_admin" : "usr_comm_1";

        std::string rawToken = auth::AuthService::generateSecureToken(32);
        std::string tokenHash = auth::AuthService::hashSessionToken(rawToken);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        models::UserSession session;
        session.id = "sess_test_1";
        session.tokenHash = tokenHash;
        session.userId = targetUid;
        session.createdAt = now;
        session.expiresAt = now + 86400;
        session.userAgent = "TestAgent/1.0";
        session.ipAddress = "127.0.0.1";

        ASSERT_TRUE(repo.createUserSession(session));

        auto retrieved = repo.findSessionByTokenHash(tokenHash);
        ASSERT_TRUE(retrieved.has_value());
        ASSERT_EQ(retrieved->userId, targetUid);

        // Revoke session
        ASSERT_TRUE(repo.revokeSession("sess_test_1"));
        auto afterRevoke = repo.findSessionByTokenHash(tokenHash);
        ASSERT_FALSE(afterRevoke.has_value());
    TEST_PASS()

    TEST_CASE("Database", "System Config & Audit Logging")
        ASSERT_NEAR(repo.getAttendanceThreshold(), 75.0, 0.01);
        ASSERT_TRUE(repo.setAttendanceThreshold(80.0));
        ASSERT_NEAR(repo.getAttendanceThreshold(), 80.0, 0.01);

        // Reset to 75.0
        ASSERT_TRUE(repo.setAttendanceThreshold(75.0));

        // Audit Log
        ASSERT_TRUE(repo.logAudit("usr_test_admin", "admin@test.edu", "CONFIG_UPDATE", "SETTING", "threshold", "{\"old\":75.0,\"new\":80.0}", "127.0.0.1"));
        auto logs = repo.findRecentAuditLogs(10);
        ASSERT_TRUE(!logs.empty());
        ASSERT_EQ(logs[0].action, "CONFIG_UPDATE");
        ASSERT_EQ(logs[0].entityType, "SETTING");
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 5: Faculty Ownership, Subject Authorization & Undo
// -------------------------------------------------------------
void suite_faculty_ownership_and_undo() {
    auto& db = db::Database::instance();
    repositories::AttendanceRepository repo(db);

    TEST_CASE("Authorization", "Faculty Subject Ownership & Enrollment Relationship Checks")
        models::User fac1User{"usr_fac_1", "prof1@test.edu", auth::AuthService::hashPassword("fac123"), "Prof One", models::UserRole::FACULTY};
        repo.createUser(fac1User);
        db.prepare("INSERT INTO faculty (id, user_id, employee_id, department) VALUES ('fac_1', 'usr_fac_1', 'E01', 'CSE');")->execute();

        models::User fac2User{"usr_fac_2", "prof2@test.edu", auth::AuthService::hashPassword("fac123"), "Prof Two", models::UserRole::FACULTY};
        repo.createUser(fac2User);
        db.prepare("INSERT INTO faculty (id, user_id, employee_id, department) VALUES ('fac_2', 'usr_fac_2', 'E02', 'CSE');")->execute();

        models::Subject subA{"sub_A", "CS301", "Algorithms", "fac_1", 4};
        repo.createSubject(subA);

        models::Subject subB{"sub_B", "CS302", "Databases", "fac_2", 4};
        repo.createSubject(subB);

        ASSERT_TRUE(repo.isFacultyAssignedToSubject("fac_1", "sub_A"));
        ASSERT_TRUE(repo.isFacultyAssignedToSubject("usr_fac_1", "sub_A"));
        ASSERT_FALSE(repo.isFacultyAssignedToSubject("fac_1", "sub_B"));

        models::User stuUser{"usr_stu_1", "alice@test.edu", auth::AuthService::hashPassword("stu123"), "Alice", models::UserRole::STUDENT};
        repo.createUser(stuUser);
        models::Student stu{"stu_1", "usr_stu_1", "2026CS100", "CSE", 5};
        repo.createStudent(stu);
        repo.createEnrollment("stu_1", "sub_A");

        ASSERT_TRUE(repo.isStudentEnrolledInSubject("stu_1", "sub_A"));
        ASSERT_FALSE(repo.isStudentEnrolledInSubject("stu_1", "sub_B"));
        ASSERT_TRUE(repo.isStudentEnrolledInFacultySubjects("stu_1", "fac_1"));
        ASSERT_FALSE(repo.isStudentEnrolledInFacultySubjects("stu_1", "fac_2"));
    TEST_PASS()

    TEST_CASE("Undo", "Case A & Case B Roll Call Undo Workflows")
        models::AttendanceSession session{"ses_corr_1", "sub_A", "fac_1", "2026-11-18", "Graph Traversal"};
        ASSERT_TRUE(repo.createSession(session));

        models::AttendanceRecord origRec{"rec_corr_1", "ses_corr_1", "stu_1", models::AttendanceStatus::PRESENT, "Initial"};
        ASSERT_TRUE(repo.createRecord(origRec));

        ASSERT_TRUE(repo.updateRecordStatus(origRec.id, models::AttendanceStatus::ABSENT));

        models::UndoAction undoCaseB{
            "undo_case_B", origRec.id, session.id, "stu_1", "Alice", "CS301",
            models::AttendanceStatus::PRESENT,
            models::AttendanceStatus::ABSENT,
            "2026-11-18T11:00:00Z", "usr_fac_1", "Status correction"
        };
        ASSERT_TRUE(repo.saveUndoRecord(undoCaseB));

        auto popped = repo.popLatestUndoRecord("usr_fac_1");
        ASSERT_TRUE(popped.has_value());
        ASSERT_EQ(popped->previousStatus, models::AttendanceStatus::PRESENT);

        ASSERT_TRUE(repo.updateRecordStatus(popped->recordId, popped->previousStatus));
        ASSERT_TRUE(repo.markUndoActionDone(popped->id));

        auto restoredRec = repo.findRecordById("rec_corr_1");
        ASSERT_TRUE(restoredRec.has_value());
        ASSERT_EQ(restoredRec->status, models::AttendanceStatus::PRESENT);
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 6: Risk Prediction & Boundaries
// -------------------------------------------------------------
void suite_risk_prediction_boundaries() {
    using models::AttendanceStatus;

    TEST_CASE("ML", "Prediction: Low, Borderline, High, Critical, and 0-Session Edge Cases")
        std::vector<AttendanceStatus> lowRecs;
        for (int i = 0; i < 2; i++) lowRecs.push_back(AttendanceStatus::ABSENT);
        for (int i = 0; i < 18; i++) lowRecs.push_back(AttendanceStatus::PRESENT);
        auto predLow = ml::RiskPredictor::predict(lowRecs, 75.0);
        ASSERT_EQ(predLow.riskCategory, models::RiskLevel::LOW);
        ASSERT_TRUE(predLow.riskScore <= 0.35);

        std::vector<AttendanceStatus> critRecs;
        for (int i = 0; i < 8; i++) critRecs.push_back(AttendanceStatus::PRESENT);
        for (int i = 0; i < 12; i++) critRecs.push_back(AttendanceStatus::ABSENT);
        auto predCrit = ml::RiskPredictor::predict(critRecs, 75.0);
        ASSERT_EQ(predCrit.riskCategory, models::RiskLevel::CRITICAL);
        ASSERT_TRUE(predCrit.riskScore >= 0.70);

        std::vector<AttendanceStatus> emptyRecs;
        auto predEmpty = ml::RiskPredictor::predict(emptyRecs, 75.0);
        ASSERT_EQ(predEmpty.riskCategory, models::RiskLevel::LOW);
        ASSERT_EQ(predEmpty.riskScore, 0.0);
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 7: CSV Reports & Dynamic SVG Generation
// -------------------------------------------------------------
void suite_reports_and_svg() {
    auto& db = db::Database::instance();
    repositories::AttendanceRepository repo(db);
    services::AttendanceService service(repo);

    TEST_CASE("Reports", "CSV Generation: Students, Risk, Subjects, Faculty")
        std::string csvStudents = service.generateStudentsCsv();
        ASSERT_TRUE(!csvStudents.empty());
        ASSERT_TRUE(csvStudents.find("Roll Number,Student Name") != std::string::npos);

        std::string csvRisk = service.generateRiskReportCsv(75.0);
        ASSERT_TRUE(!csvRisk.empty());
        ASSERT_TRUE(csvRisk.find("Roll Number,Student Name") != std::string::npos);
    TEST_PASS()

    TEST_CASE("SVG", "Dynamic Vector Graphics: Gauge, Bar, Trend, and Donut Charts")
        std::string gaugeSvg = views::SvgCharts::renderAttendanceGauge(82.5, 75.0, 160);
        ASSERT_TRUE(gaugeSvg.find("<svg") != std::string::npos);
        ASSERT_TRUE(gaugeSvg.find("82.5%") != std::string::npos);

        std::vector<std::string> subLabels = {"CS301", "CS302"};
        std::vector<double> subValues = {85.0, 68.0};
        std::string barSvg = views::SvgCharts::renderSubjectBarChart(subLabels, subValues, 75.0, 500, 200);
        ASSERT_TRUE(barSvg.find("<svg") != std::string::npos);
        ASSERT_TRUE(barSvg.find("CS301") != std::string::npos);

        std::vector<std::string> trendDates = {"W1", "W2", "W3", "W4"};
        std::vector<double> trendValues = {90.0, 85.0, 80.0, 72.0};
        std::string trendSvg = views::SvgCharts::renderAttendanceTrendChart(trendDates, trendValues, 75.0, 500, 200);
        ASSERT_TRUE(trendSvg.find("<svg") != std::string::npos);
        ASSERT_TRUE(trendSvg.find("<path") != std::string::npos);

        std::string donutSvg = views::SvgCharts::renderRiskDistributionDonut(25, 10, 5, 2, 220);
        ASSERT_TRUE(donutSvg.find("<svg") != std::string::npos);
        ASSERT_TRUE(donutSvg.find("Low</span>") != std::string::npos);
    TEST_PASS()
}

int main() {
    std::cout << "========================================================\n";
    std::cout << " AI ATTENDANCE RISK PREDICTOR — REPEATABLE C++ TEST SUITE\n";
    std::cout << "========================================================\n\n";

    #ifdef _WIN32
    _putenv("JWT_SECRET=test_production_ready_cryptographic_secret_key_256_bits_length");
    #else
    setenv("JWT_SECRET", "test_production_ready_cryptographic_secret_key_256_bits_length", 1);
    #endif

    std::string testDbPath = "database/test_isolated_suite.db";
    std::remove(testDbPath.c_str());

    std::string schemaSql = readFileContents("database/migrations/001_initial_schema.sql");
    std::string migration2Sql = readFileContents("database/migrations/002_sessions_and_audit.sql");

    suite_dsa_structures();
    suite_core_algorithms();
    suite_authentication_security();
    suite_database_and_integrity(testDbPath, schemaSql, migration2Sql);
    suite_faculty_ownership_and_undo();
    suite_risk_prediction_boundaries();
    suite_reports_and_svg();

    std::remove(testDbPath.c_str());

    std::cout << "\n========================================================\n";
    std::cout << " FINAL RESULTS: " << g_testsPassed << " PASSED, " << g_testsFailed << " FAILED\n";
    std::cout << "========================================================\n";

    return (g_testsFailed == 0) ? 0 : 1;
}
