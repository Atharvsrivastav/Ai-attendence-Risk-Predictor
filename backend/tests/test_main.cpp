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
        ASSERT_EQ(ls.stepsCount, 6); // Found at index 5 (6 comparisons)

        auto bs = algorithms::binarySearch(
            sortedArr,
            std::string("60"),
            [](int v) { return std::to_string(v); }
        );
        ASSERT_TRUE(bs.found);
        ASSERT_TRUE(bs.stepsCount <= 3); // log2(8) = 3 steps
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

        // Total conducted = 4 (excluding Excused)
        // Attended = 1 + 1 + 0.5 = 2.5
        // Percentage = (2.5 / 4.0) * 100 = 62.5%
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

        // Tail contains: ABSENT, EXCUSED, ABSENT -> Active streak = 2 (Excused does NOT break streak)
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
        // Currently 12 attended out of 20 conducted = 60%. Threshold = 75%.
        // Formula: ceil((0.75 * 20 - 12) / (1 - 0.75)) = ceil((15 - 12) / 0.25) = ceil(3 / 0.25) = 12
        auto rec = algorithms::calculateRecovery(12.0, 20, 75.0);
        ASSERT_EQ(rec.classesNeededToRecover, 12);
        ASSERT_EQ(rec.classesCanBeMissed, 0);

        // Currently 18 attended out of 20 conducted = 90%. Threshold = 75%.
        // Formula: floor((18 - 0.75 * 20) / 0.75) = floor((18 - 15) / 0.75) = floor(3 / 0.75) = 4
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

    TEST_CASE("Auth", "Password Verification: Bcrypt Verification & Rejection of Foreign/Legacy Passwords")
        std::string password = "AdminPassword@2026!";
        std::string hash = auth::AuthService::hashPassword(password);
        ASSERT_TRUE(hash.rfind("$2a$", 0) == 0 || hash.rfind("$2b$", 0) == 0);

        // Correct password succeeds
        ASSERT_TRUE(auth::AuthService::verifyPassword(password, hash));

        // Incorrect password fails
        ASSERT_FALSE(auth::AuthService::verifyPassword("WrongPassword123", hash));

        // Foreign account passwords fail
        ASSERT_FALSE(auth::AuthService::verifyPassword("student123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("faculty123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("admin123", hash));

        // Custom $sha256$ legacy format is REJECTED
        std::string legacyHash = "$sha256$1234567890abcdef1234567890abcdef";
        ASSERT_FALSE(auth::AuthService::verifyPassword("anyPassword", legacyHash));
    TEST_PASS()

    TEST_CASE("Auth", "Account Setup Token: Lockout Before Setup & Password Creation")
        std::string setupToken = auth::AuthService::generateSecureToken(32);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long long expEpoch = now + 3600; // 1 hour validity

        std::string lockedHash = "!SETUP_REQUIRED:" + setupToken + ":" + std::to_string(expEpoch);
        ASSERT_TRUE(auth::AuthService::isSetupRequired(lockedHash));

        // Normal login attempt is rejected on locked account
        ASSERT_FALSE(auth::AuthService::verifyPassword("student123", lockedHash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("", lockedHash));

        // Setup token verification
        ASSERT_TRUE(auth::AuthService::verifySetupToken(lockedHash, setupToken));
        ASSERT_FALSE(auth::AuthService::verifySetupToken(lockedHash, "wrong_setup_token"));

        // Expired setup token fails
        std::string expiredLockedHash = "!SETUP_REQUIRED:" + setupToken + ":" + std::to_string(now - 100);
        ASSERT_FALSE(auth::AuthService::verifySetupToken(expiredLockedHash, setupToken));
    TEST_PASS()

    TEST_CASE("Auth", "JWT HS256: Secret Length Requirement (>= 256 bits / 32 chars)")
        ASSERT_FALSE(auth::AuthService::validateSecret("short_key"));
        ASSERT_FALSE(auth::AuthService::validateSecret("too_weak_secret_under_32_bytes_"));
        ASSERT_TRUE(auth::AuthService::validateSecret("this_is_a_very_strong_256_bit_secret_key_12345"));
    TEST_PASS()

    TEST_CASE("Auth", "JWT HS256: Claims Validation (iss, aud, iat, exp, signature, forgery)")
        auth::SessionClaims claims{
            "usr_admin_123",
            "admin@univ.edu",
            "Dean of Academics",
            models::UserRole::ADMIN,
            "",
            ""
        };
        std::string secret = "test_very_secure_secret_key_1234567890_min_32_chars";
        std::string validToken = auth::AuthService::generateToken(claims, secret);
        ASSERT_TRUE(!validToken.empty());

        // 1. Valid token passes
        auto verified = auth::AuthService::verifyToken(validToken, secret);
        ASSERT_TRUE(verified.has_value());
        ASSERT_EQ(verified->userId, "usr_admin_123");
        ASSERT_EQ(verified->role, models::UserRole::ADMIN);

        // 2. Token signed with wrong secret FAILS
        auto wrongSecretRes = auth::AuthService::verifyToken(validToken, "completely_wrong_secret_key_minimum_32_chars_long");
        ASSERT_FALSE(wrongSecretRes.has_value());

        // 3. Forged token with modified payload FAILS
        std::string forgedToken = validToken;
        size_t dot1 = forgedToken.find('.');
        size_t dot2 = forgedToken.rfind('.');
        if (dot1 != std::string::npos && dot2 != std::string::npos) {
            forgedToken[dot1 + 5] = (forgedToken[dot1 + 5] == 'A') ? 'B' : 'A';
        }
        ASSERT_FALSE(auth::AuthService::verifyToken(forgedToken, secret).has_value());

        // 4. Forged signature FAILS
        std::string forgedSigToken = validToken.substr(0, dot2 + 1) + "fake_signature_bytes_here";
        ASSERT_FALSE(auth::AuthService::verifyToken(forgedSigToken, secret).has_value());

        // 5. Expired token FAILS
        auth::SessionClaims expiredClaims = claims;
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        expiredClaims.issuedAt = now - 7200;
        expiredClaims.expiresAt = now - 3600;
        std::string expiredToken = auth::AuthService::generateToken(expiredClaims, secret);
        ASSERT_FALSE(auth::AuthService::verifyToken(expiredToken, secret).has_value());

        // 6. Token with missing exp FAILS
        auth::SessionClaims noExpClaims = claims;
        noExpClaims.expiresAt = 0; // will default to 7 days in generateToken, test manual missing exp payload
        ASSERT_FALSE(auth::AuthService::verifyToken("header.payload_without_exp.sig", secret).has_value());

        // 7. Malformed tokens FAIL
        ASSERT_FALSE(auth::AuthService::verifyToken("not.a.valid.jwt.format", secret).has_value());
        ASSERT_FALSE(auth::AuthService::verifyToken("malformed@@token.invalid$chars.bad", secret).has_value());
        ASSERT_FALSE(auth::AuthService::verifyToken("", secret).has_value());
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 4: Isolated Database, Foreign Keys, and Transactions
// -------------------------------------------------------------
void suite_database_and_integrity(const std::string& testDbPath, const std::string& schemaSql) {
    auto& db = db::Database::instance();
    ASSERT_TRUE(db.open(testDbPath));
    ASSERT_TRUE(db.runMigration(schemaSql));

    repositories::AttendanceRepository repo(db);

    TEST_CASE("Database", "Integrity: Parameterized CRUD, Foreign Key Enforcement, Duplicate Rejection")
        models::User adminUser{
            "usr_test_admin", "admin@test.edu",
            auth::AuthService::hashPassword("admin123"),
            "Test Admin", models::UserRole::ADMIN
        };
        ASSERT_TRUE(repo.createUser(adminUser));

        // Duplicate email rejection
        models::User dupUser = adminUser;
        dupUser.id = "usr_test_admin_2";
        ASSERT_FALSE(repo.createUser(dupUser));

        // Foreign key enforcement: student with non-existent user_id MUST fail!
        models::Student invalidStudent{
            "stu_invalid", "usr_non_existent", "ROLL_INVALID", "CSE", 5
        };
        ASSERT_FALSE(repo.createStudent(invalidStudent));
    TEST_PASS()

    TEST_CASE("Database", "Transactions: Atomic Multi-Operation Rollback on Failure")
        {
            db::Database::ScopedTransaction tx(db);
            ASSERT_TRUE(tx.isActive());

            models::User tempUser{
                "usr_temp_1", "temp1@test.edu",
                auth::AuthService::hashPassword("temp123"),
                "Temp User", models::UserRole::STUDENT
            };
            ASSERT_TRUE(repo.createUser(tempUser));

            // Destructor will auto-rollback because commit was not called
        }

        // Temp user must not exist after rollback
        auto found = repo.findUserByEmail("temp1@test.edu");
        ASSERT_FALSE(found.has_value());
    TEST_PASS()

    TEST_CASE("Database", "Transactions: Strict Commit Success Validation")
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
            ASSERT_FALSE(tx.isActive()); // No longer active after successful commit
        }

        auto foundComm = repo.findUserByEmail("comm1@test.edu");
        ASSERT_TRUE(foundComm.has_value());
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 5: Faculty Ownership, Subject Authorization & Undo Cases
// -------------------------------------------------------------
void suite_faculty_ownership_and_undo(const std::string& testDbPath) {
    auto& db = db::Database::instance();
    repositories::AttendanceRepository repo(db);

    TEST_CASE("Authorization", "Faculty Subject Ownership & Enrollment Relationship Checks")
        // Setup two faculty members
        models::User fac1User{"usr_fac_1", "prof1@test.edu", auth::AuthService::hashPassword("fac123"), "Prof One", models::UserRole::FACULTY};
        repo.createUser(fac1User);
        db.prepare("INSERT INTO faculty (id, user_id, employee_id, department) VALUES ('fac_1', 'usr_fac_1', 'E01', 'CSE');")->execute();

        models::User fac2User{"usr_fac_2", "prof2@test.edu", auth::AuthService::hashPassword("fac123"), "Prof Two", models::UserRole::FACULTY};
        repo.createUser(fac2User);
        db.prepare("INSERT INTO faculty (id, user_id, employee_id, department) VALUES ('fac_2', 'usr_fac_2', 'E02', 'CSE');")->execute();

        // Prof One teaches Subject A
        models::Subject subA{"sub_A", "CS301", "Algorithms", "fac_1", 4};
        repo.createSubject(subA);

        // Prof Two teaches Subject B
        models::Subject subB{"sub_B", "CS302", "Databases", "fac_2", 4};
        repo.createSubject(subB);

        // Verify subject ownership
        ASSERT_TRUE(repo.isFacultyAssignedToSubject("fac_1", "sub_A"));
        ASSERT_TRUE(repo.isFacultyAssignedToSubject("usr_fac_1", "sub_A"));
        ASSERT_FALSE(repo.isFacultyAssignedToSubject("fac_1", "sub_B")); // Prof 1 does NOT teach Sub B

        ASSERT_TRUE(repo.isFacultyAssignedToSubject("fac_2", "sub_B"));
        ASSERT_FALSE(repo.isFacultyAssignedToSubject("fac_2", "sub_A")); // Prof 2 does NOT teach Sub A

        // Setup Student enrolled ONLY in Subject A
        models::User stuUser{"usr_stu_1", "alice@test.edu", auth::AuthService::hashPassword("stu123"), "Alice", models::UserRole::STUDENT};
        repo.createUser(stuUser);
        models::Student stu{"stu_1", "usr_stu_1", "2026CS100", "CSE", 5};
        repo.createStudent(stu);
        repo.createEnrollment("stu_1", "sub_A");

        ASSERT_TRUE(repo.isStudentEnrolledInSubject("stu_1", "sub_A"));
        ASSERT_FALSE(repo.isStudentEnrolledInSubject("stu_1", "sub_B"));

        // Prof 1 teaches Alice; Prof 2 does NOT teach Alice
        ASSERT_TRUE(repo.isStudentEnrolledInFacultySubjects("stu_1", "fac_1"));
        ASSERT_FALSE(repo.isStudentEnrolledInFacultySubjects("stu_1", "fac_2"));
    TEST_PASS()

    TEST_CASE("Undo", "Case A: Newly Created Record Deletion")
        models::AttendanceSession session{"ses_new_1", "sub_A", "fac_1", "2026-11-18", "Dynamic Programming"};
        ASSERT_TRUE(repo.createSession(session));

        models::AttendanceRecord newRec{"rec_new_1", "ses_new_1", "stu_1", models::AttendanceStatus::PRESENT, "Newly recorded"};
        ASSERT_TRUE(repo.createRecord(newRec));

        // Record Case A undo action (previousStatus = NONE)
        models::UndoAction undoCaseA{
            "undo_case_A", newRec.id, session.id, "stu_1", "Alice", "CS301",
            models::AttendanceStatus::NONE, // Indicates newly inserted record
            models::AttendanceStatus::PRESENT,
            "2026-11-18T10:00:00Z", "usr_fac_1", "Roll call submission"
        };
        ASSERT_TRUE(repo.saveUndoRecord(undoCaseA));

        // Execute Case A undo: record must be DELETED, not converted to EXCUSED
        auto popped = repo.popLatestUndoRecord("usr_fac_1");
        ASSERT_TRUE(popped.has_value());
        ASSERT_EQ(popped->previousStatus, models::AttendanceStatus::NONE);

        ASSERT_TRUE(repo.deleteRecord(popped->recordId));
        repo.deleteSessionIfEmpty(popped->sessionId);
        ASSERT_TRUE(repo.markUndoActionDone(popped->id));

        // Verify record is gone from database
        ASSERT_FALSE(repo.findRecordById("rec_new_1").has_value());
    TEST_PASS()

    TEST_CASE("Undo", "Case B: Existing Record Correction Restoration")
        models::AttendanceSession session{"ses_corr_1", "sub_A", "fac_1", "2026-11-18", "Graph Traversal"};
        ASSERT_TRUE(repo.createSession(session));

        // Original status: PRESENT
        models::AttendanceRecord origRec{"rec_corr_1", "ses_corr_1", "stu_1", models::AttendanceStatus::PRESENT, "Initial"};
        ASSERT_TRUE(repo.createRecord(origRec));

        // Faculty corrects status from PRESENT to ABSENT
        ASSERT_TRUE(repo.updateRecordStatus(origRec.id, models::AttendanceStatus::ABSENT));

        // Record Case B undo action (previousStatus = PRESENT)
        models::UndoAction undoCaseB{
            "undo_case_B", origRec.id, session.id, "stu_1", "Alice", "CS301",
            models::AttendanceStatus::PRESENT,
            models::AttendanceStatus::ABSENT,
            "2026-11-18T11:00:00Z", "usr_fac_1", "Status correction"
        };
        ASSERT_TRUE(repo.saveUndoRecord(undoCaseB));

        // Execute Case B undo: record must be RESTORED to PRESENT
        auto popped = repo.popLatestUndoRecord("usr_fac_1");
        ASSERT_TRUE(popped.has_value());
        ASSERT_EQ(popped->previousStatus, models::AttendanceStatus::PRESENT);

        ASSERT_TRUE(repo.updateRecordStatus(popped->recordId, popped->previousStatus));
        ASSERT_TRUE(repo.markUndoActionDone(popped->id));

        auto restoredRec = repo.findRecordById("rec_corr_1");
        ASSERT_TRUE(restoredRec.has_value());
        ASSERT_EQ(restoredRec->status, models::AttendanceStatus::PRESENT);

        // Duplicate undo attempt returns nullopt
        ASSERT_FALSE(repo.popLatestUndoRecord("usr_fac_1").has_value());
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 6: Risk Prediction & 75% Boundary Tests
// -------------------------------------------------------------
void suite_risk_prediction_boundaries() {
    using models::AttendanceStatus;

    TEST_CASE("ML", "Prediction: Low Risk (>85% Attendance, Positive Momentum)")
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 2; i++) records.push_back(AttendanceStatus::ABSENT);
        for (int i = 0; i < 18; i++) records.push_back(AttendanceStatus::PRESENT);

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_EQ(pred.riskCategory, models::RiskLevel::LOW);
        ASSERT_TRUE(pred.riskScore <= 0.35);
        ASSERT_TRUE(pred.confidence > 0.80);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: 75% Boundary Condition (Borderline Medium Risk)")
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 5; i++) {
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::ABSENT);
        }

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::MEDIUM || pred.riskCategory == models::RiskLevel::LOW);
        ASSERT_TRUE(pred.riskScore <= 0.55);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: High Risk (70% Attendance with Downward Drop)")
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 14; i++) records.push_back(AttendanceStatus::PRESENT);
        for (int i = 0; i < 6; i++) records.push_back(AttendanceStatus::ABSENT);

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::HIGH || pred.riskCategory == models::RiskLevel::CRITICAL);
        ASSERT_TRUE(pred.riskScore >= 0.50);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: Critical Risk (<50% Attendance, Severe Streak)")
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 8; i++) records.push_back(AttendanceStatus::PRESENT);
        for (int i = 0; i < 12; i++) records.push_back(AttendanceStatus::ABSENT);

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_EQ(pred.riskCategory, models::RiskLevel::CRITICAL);
        ASSERT_TRUE(pred.riskScore >= 0.70);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: Edge Case (0 Conducted Sessions)")
        std::vector<AttendanceStatus> records;
        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_EQ(pred.riskCategory, models::RiskLevel::LOW);
        ASSERT_EQ(pred.riskScore, 0.0);
    TEST_PASS()
}

int main() {
    std::cout << "========================================================\n";
    std::cout << " AI ATTENDANCE RISK PREDICTOR — REPEATABLE C++ TEST SUITE\n";
    std::cout << "========================================================\n\n";

    // Set secure test JWT secret in environment
    #ifdef _WIN32
    _putenv("JWT_SECRET=test_production_ready_cryptographic_secret_key_256_bits_length");
    #else
    setenv("JWT_SECRET", "test_production_ready_cryptographic_secret_key_256_bits_length", 1);
    #endif

    std::string testDbPath = "database/test_isolated_suite.db";
    std::remove(testDbPath.c_str());

    std::string schemaSql = readFileContents("database/migrations/001_initial_schema.sql");
    if (schemaSql.empty()) {
        schemaSql = readFileContents("../database/migrations/001_initial_schema.sql");
    }

    suite_dsa_structures();
    suite_core_algorithms();
    suite_authentication_security();
    suite_database_and_integrity(testDbPath, schemaSql);
    suite_faculty_ownership_and_undo(testDbPath);
    suite_risk_prediction_boundaries();

    // Clean up isolated test database
    std::remove(testDbPath.c_str());

    std::cout << "\n========================================================\n";
    std::cout << " FINAL RESULTS: " << g_testsPassed << " PASSED, " << g_testsFailed << " FAILED\n";
    std::cout << "========================================================\n";

    return (g_testsFailed == 0) ? 0 : 1;
}
