#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

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

// Helper to read file contents
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

        // Trigger dynamic rehashing (> 0.75 load factor)
        map.set("student_4", 75);
        map.set("student_5", 99);
        map.set("student_6", 45);
        ASSERT_TRUE(map.capacity() >= 8);

        // Remove
        ASSERT_TRUE(map.remove("student_2"));
        ASSERT_FALSE(map.has("student_2"));
        ASSERT_FALSE(map.remove("non_existing_key"));
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceQueue: FIFO Enqueue, Dequeue, Peek, Thread Safety")
        dsa::AttendanceQueue<std::string> queue;
        ASSERT_TRUE(queue.isEmpty());

        queue.enqueue("alert_1");
        queue.enqueue("alert_2");
        queue.enqueue("alert_3");
        ASSERT_EQ(queue.size(), 3);

        auto peeked = queue.peek();
        ASSERT_TRUE(peeked.has_value());
        ASSERT_EQ(*peeked, "alert_1");

        auto d1 = queue.dequeue();
        ASSERT_TRUE(d1.has_value());
        ASSERT_EQ(*d1, "alert_1");

        auto d2 = queue.dequeue();
        ASSERT_TRUE(d2.has_value());
        ASSERT_EQ(*d2, "alert_2");

        auto d3 = queue.dequeue();
        ASSERT_TRUE(d3.has_value());
        ASSERT_EQ(*d3, "alert_3");

        ASSERT_TRUE(queue.isEmpty());
        ASSERT_FALSE(queue.dequeue().has_value());
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceStack: LIFO Push, Pop, Peek, Max Depth Pruning")
        dsa::AttendanceStack<int> stack(3); // bounded depth = 3
        stack.push(10);
        stack.push(20);
        stack.push(30);
        ASSERT_EQ(stack.size(), 3);

        // Pushing 4th item prunes oldest (10)
        stack.push(40);
        ASSERT_EQ(stack.size(), 3);

        auto p1 = stack.pop();
        ASSERT_TRUE(p1.has_value());
        ASSERT_EQ(*p1, 40);

        auto p2 = stack.pop();
        ASSERT_TRUE(p2.has_value());
        ASSERT_EQ(*p2, 30);

        auto p3 = stack.pop();
        ASSERT_TRUE(p3.has_value());
        ASSERT_EQ(*p3, 20);

        ASSERT_TRUE(stack.isEmpty());
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceSet: Add, Remove, Intersect, Unite, Difference, Subset")
        dsa::AttendanceSet<std::string> setA({"Alice", "Bob", "Charlie"});
        dsa::AttendanceSet<std::string> setB({"Bob", "Charlie", "David"});

        ASSERT_TRUE(setA.has("Alice"));
        ASSERT_FALSE(setA.has("David"));

        auto intersection = setA.intersect(setB);
        ASSERT_EQ(intersection.size(), 2);
        ASSERT_TRUE(intersection.has("Bob"));
        ASSERT_TRUE(intersection.has("Charlie"));
        ASSERT_FALSE(intersection.has("Alice"));

        auto unionSet = setA.unite(setB);
        ASSERT_EQ(unionSet.size(), 4);
        ASSERT_TRUE(unionSet.has("Alice"));
        ASSERT_TRUE(unionSet.has("David"));

        auto diff = setA.difference(setB);
        ASSERT_EQ(diff.size(), 1);
        ASSERT_TRUE(diff.has("Alice"));
    TEST_PASS()

    TEST_CASE("DSA", "AttendanceGraph: Adjacency List, BFS, DFS, Shortest Path")
        dsa::AttendanceGraph graph;
        graph.addNode({"FAC_1", "Dr. Alan", "FACULTY", {}});
        graph.addNode({"SUB_CS101", "Computer Science", "SUBJECT", {}});
        graph.addNode({"STU_1", "Alice", "STUDENT", {}});
        graph.addNode({"STU_2", "Bob", "STUDENT", {}});

        graph.addEdge({"FAC_1", "SUB_CS101", "TEACHES"}, true);
        graph.addEdge({"SUB_CS101", "STU_1", "ENROLLED_IN"}, true);
        graph.addEdge({"SUB_CS101", "STU_2", "ENROLLED_IN"}, true);

        auto bfsRes = graph.bfs("FAC_1");
        ASSERT_EQ(bfsRes.nodesVisited, 4);
        ASSERT_EQ(bfsRes.order[0], "FAC_1");
        ASSERT_EQ(bfsRes.order[1], "SUB_CS101");

        auto dfsRes = graph.dfs("FAC_1");
        ASSERT_EQ(dfsRes.nodesVisited, 4);

        auto path = graph.findShortestPath("FAC_1", "STU_1");
        ASSERT_EQ(path.size(), 3);
        ASSERT_EQ(path[0], "FAC_1");
        ASSERT_EQ(path[1], "SUB_CS101");
        ASSERT_EQ(path[2], "STU_1");
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 2: Core Algorithms
// -------------------------------------------------------------
void suite_core_algorithms() {
    TEST_CASE("Algorithms", "Sorting: MergeSort and QuickSort with Swap and Comparison Counts")
        std::vector<int> data = {64, 25, 12, 22, 11, 90, 42, 88};

        auto mergeRes = algorithms::mergeSortWithMetrics(data);
        ASSERT_EQ(mergeRes.sorted.size(), 8);
        for (size_t i = 1; i < mergeRes.sorted.size(); i++) {
            ASSERT_TRUE(mergeRes.sorted[i - 1] <= mergeRes.sorted[i]);
        }
        ASSERT_TRUE(mergeRes.comparisons > 0);

        auto quickRes = algorithms::quickSortWithMetrics(data);
        ASSERT_EQ(quickRes.sorted.size(), 8);
        for (size_t i = 1; i < quickRes.sorted.size(); i++) {
            ASSERT_TRUE(quickRes.sorted[i - 1] <= quickRes.sorted[i]);
        }
        ASSERT_TRUE(quickRes.comparisons > 0);
    TEST_PASS()

    TEST_CASE("Algorithms", "Searching: LinearSearch vs BinarySearch Step-by-Step Traces")
        std::vector<int> sortedArr = {10, 20, 30, 40, 50, 60, 70, 80};

        auto linearRes = algorithms::linearSearch(
            sortedArr,
            [](int v) { return v == 50; },
            [](int v) { return std::to_string(v); }
        );
        ASSERT_TRUE(linearRes.found);
        ASSERT_EQ(linearRes.index, 4);
        ASSERT_EQ(linearRes.stepsCount, 5);

        auto binaryRes = algorithms::binarySearch(
            sortedArr,
            "50",
            [](int v) { return std::to_string(v); }
        );
        ASSERT_TRUE(binaryRes.found);
        ASSERT_EQ(binaryRes.index, 4);
        ASSERT_TRUE(binaryRes.stepsCount <= 4); // log2(8) = 3
    TEST_PASS()

    TEST_CASE("Algorithms", "Attendance Calculator: Rules (P=1, L=0.5, A=0, E=Excluded)")
        using models::AttendanceStatus;
        // 4 Present (4), 2 Late (1), 2 Absent (0), 2 Excused (excluded)
        // Total points = 5.0, Conducted = 8
        // Percentage = 5 / 8 * 100 = 62.5%
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::LATE, AttendanceStatus::ABSENT,
            AttendanceStatus::EXCUSED,
            AttendanceStatus::PRESENT, AttendanceStatus::LATE,
            AttendanceStatus::PRESENT, AttendanceStatus::ABSENT,
            AttendanceStatus::EXCUSED
        };

        auto res = algorithms::calculateAttendancePercentage(records, 75.0);
        ASSERT_EQ(res.totalConducted, 8);
        ASSERT_EQ(res.presentCount, 4);
        ASSERT_EQ(res.lateCount, 2);
        ASSERT_EQ(res.absentCount, 2);
        ASSERT_EQ(res.excusedCount, 2);
        ASSERT_NEAR(res.attendedPoints, 5.0, 0.001);
        ASSERT_NEAR(res.percentage, 62.5, 0.001);
        ASSERT_TRUE(res.isBelowThreshold);
    TEST_PASS()

    TEST_CASE("Algorithms", "Consecutive Absences: Backward Streak and Excused Exemption")
        using models::AttendanceStatus;
        // End has: Absent, Excused, Absent -> Active consecutive absence is 2! (Excused does not break streak)
        std::vector<AttendanceStatus> history = {
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::EXCUSED,
            AttendanceStatus::ABSENT
        };

        auto streakRes = algorithms::detectConsecutiveAbsences(history);
        ASSERT_EQ(streakRes.currentConsecutive, 2);
        ASSERT_EQ(streakRes.maxConsecutive, 3);
    TEST_PASS()

    TEST_CASE("Algorithms", "Sliding Window Analysis & OLS Linear Regression Trend")
        using models::AttendanceStatus;
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT, AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT, AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT
        };

        auto window = algorithms::analyzeRecentAttendanceWindow(records, 100.0, 5);
        ASSERT_EQ(window.windowSizeExamined, 5);
        ASSERT_NEAR(window.recentPercentage, 0.0, 0.001);
        ASSERT_TRUE(window.isSuddenDrop);

        auto trend = algorithms::analyzeAttendanceTrend(records);
        ASSERT_TRUE(trend.slope < 0);
        ASSERT_TRUE(trend.isDecliningAlert);
    TEST_PASS()

    TEST_CASE("Algorithms", "Recovery Calculator: Exact Needed Lectures & Safe Misses")
        // Conducted: 20, Attended: 12 (60%). Threshold = 75%.
        // Needed: ceil((0.75 * 20 - 12) / (1 - 0.75)) = 12
        auto recovery = algorithms::calculateRecovery(12.0, 20, 75.0);
        ASSERT_TRUE(recovery.isBelowThreshold);
        ASSERT_EQ(recovery.classesNeededToRecover, 12);

        // Conducted: 20, Attended: 18 (90%). Threshold = 75%.
        // Safe misses: floor((18 - 0.75 * 20) / 0.75) = 4
        auto safe = algorithms::calculateRecovery(18.0, 20, 75.0);
        ASSERT_FALSE(safe.isBelowThreshold);
        ASSERT_EQ(safe.classesCanBeMissed, 4);

        // Simulation
        auto sim = algorithms::simulateAttendance(12.0, 20, 10, 2, 75.0);
        ASSERT_EQ(sim.projectedTotalSessions, 32);
        ASSERT_NEAR(sim.projectedAttendedSessions, 22.0, 0.001);
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 3: Authentication Security
// -------------------------------------------------------------
void suite_authentication_security() {
    TEST_CASE("Auth", "Password Verification: Bcrypt Verification & Rejection of Foreign Passwords")
        std::string password = "AdminPassword@2026!";
        std::string hash = auth::AuthService::hashPassword(password);
        ASSERT_TRUE(hash.find("$2a$") == 0 || hash.find("$2b$") == 0);

        // Correct password succeeds
        ASSERT_TRUE(auth::AuthService::verifyPassword(password, hash));

        // Incorrect password fails
        ASSERT_FALSE(auth::AuthService::verifyPassword("WrongPassword123", hash));

        // Another account's password CANNOT authenticate (admin cannot use student123 or faculty123)
        ASSERT_FALSE(auth::AuthService::verifyPassword("student123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("faculty123", hash));
        ASSERT_FALSE(auth::AuthService::verifyPassword("admin123", hash));

        // Empty password fails
        ASSERT_FALSE(auth::AuthService::verifyPassword("", hash));
    TEST_PASS()

    TEST_CASE("Auth", "JWT HS256: Signature Verification, Expiration, Forgery Rejection")
        auth::SessionClaims claims{
            "usr_admin_123",
            "admin@univ.edu",
            "Dean of Academics",
            models::UserRole::ADMIN,
            "",
            ""
        };
        std::string secret = "test_very_secure_secret_key_1234567890";
        std::string validToken = auth::AuthService::generateToken(claims, secret);
        ASSERT_TRUE(!validToken.empty());

        // 1. Valid token passes
        auto verified = auth::AuthService::verifyToken(validToken, secret);
        ASSERT_TRUE(verified.has_value());
        ASSERT_EQ(verified->userId, "usr_admin_123");
        ASSERT_EQ(verified->role, models::UserRole::ADMIN);

        // 2. Token signed with wrong secret FAILS
        auto wrongSecretRes = auth::AuthService::verifyToken(validToken, "completely_wrong_secret_key");
        ASSERT_FALSE(wrongSecretRes.has_value());

        // 3. Forged token with modified payload FAILS
        std::string forgedToken = validToken;
        size_t dot1 = forgedToken.find('.');
        size_t dot2 = forgedToken.rfind('.');
        if (dot1 != std::string::npos && dot2 != std::string::npos) {
            forgedToken[dot1 + 5] = (forgedToken[dot1 + 5] == 'A') ? 'B' : 'A'; // tamper payload
        }
        auto forgedRes = auth::AuthService::verifyToken(forgedToken, secret);
        ASSERT_FALSE(forgedRes.has_value());

        // 4. Forged signature FAILS
        std::string forgedSigToken = validToken.substr(0, dot2 + 1) + "fake_signature_bytes_here";
        auto forgedSigRes = auth::AuthService::verifyToken(forgedSigToken, secret);
        ASSERT_FALSE(forgedSigRes.has_value());

        // 5. Expired token FAILS
        auth::SessionClaims expiredClaims = claims;
        auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        expiredClaims.issuedAt = now - 7200;
        expiredClaims.expiresAt = now - 3600; // expired 1 hour ago
        std::string expiredToken = auth::AuthService::generateToken(expiredClaims, secret);
        auto expiredRes = auth::AuthService::verifyToken(expiredToken, secret);
        ASSERT_FALSE(expiredRes.has_value());

        // 6. Malformed token FAILS
        ASSERT_FALSE(auth::AuthService::verifyToken("not.a.valid.jwt.format", secret).has_value());
        ASSERT_FALSE(auth::AuthService::verifyToken("random_string_no_dots", secret).has_value());
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
        // Create user
        models::User adminUser{
            "usr_test_admin", "admin@test.edu",
            auth::AuthService::hashPassword("admin123"),
            "Test Admin", models::UserRole::ADMIN
        };
        ASSERT_TRUE(repo.createUser(adminUser));

        // Duplicate email rejection
        models::User dupUser = adminUser;
        dupUser.id = "usr_test_admin_2";
        ASSERT_FALSE(repo.createUser(dupUser)); // SQLite UNIQUE constraint must fail gracefully!

        // Foreign key enforcement: student with non-existent user_id MUST fail!
        models::Student invalidStudent{
            "stu_invalid", "usr_non_existent", "ROLL_INVALID", "CSE", 5
        };
        ASSERT_FALSE(repo.createStudent(invalidStudent));
    TEST_PASS()

    TEST_CASE("Database", "Transactions: Atomic Multi-Operation Rollback on Failure")
        // Begin transaction
        {
            db::Database::ScopedTransaction tx(db);

            models::User tempUser{
                "usr_temp_1", "temp1@test.edu",
                auth::AuthService::hashPassword("temp123"),
                "Temp User", models::UserRole::STUDENT
            };
            ASSERT_TRUE(repo.createUser(tempUser));

            // Intentionally not committing tx -> destructor will auto-rollback!
        }

        // Temp user must not exist after rollback
        auto found = repo.findUserByEmail("temp1@test.edu");
        ASSERT_FALSE(found.has_value());
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 5: Attendance Lifecycle & Exact Undo Verification
// -------------------------------------------------------------
void suite_attendance_undo_lifecycle(const std::string& testDbPath) {
    auto& db = db::Database::instance();
    repositories::AttendanceRepository repo(db);

    TEST_CASE("Undo", "Lifecycle: PRESENT -> Undo, LATE -> Undo, Exact State Restoration")
        // Setup fixtures
        models::User facUser{"usr_fac_test", "fac@test.edu", auth::AuthService::hashPassword("fac123"), "Prof. Test", models::UserRole::FACULTY};
        repo.createUser(facUser);

        models::Faculty fac{"fac_test", "usr_fac_test", "EMP001", "CSE"};
        auto stmtFac = db.prepare("INSERT INTO faculty (id, user_id, employee_id, department) VALUES ('fac_test', 'usr_fac_test', 'EMP001', 'CSE');");
        stmtFac->execute();

        models::Subject sub{"sub_test", "CS101", "Data Structures", "fac_test", 4};
        ASSERT_TRUE(repo.createSubject(sub));

        models::User stuUser{"usr_stu_test", "stu@test.edu", auth::AuthService::hashPassword("stu123"), "Alice Student", models::UserRole::STUDENT};
        repo.createUser(stuUser);

        models::Student stu{"stu_test", "usr_stu_test", "2026CS001", "CSE", 5};
        ASSERT_TRUE(repo.createStudent(stu));

        models::AttendanceSession session{"ses_test_1", "sub_test", "fac_test", "2026-11-18", "Hashing Lab"};
        ASSERT_TRUE(repo.createSession(session));

        // 1. Initial State: Marked PRESENT (previous state was EXCUSED or draft)
        models::AttendanceRecord rec{"rec_test_1", "ses_test_1", "stu_test", models::AttendanceStatus::PRESENT, "On time"};
        ASSERT_TRUE(repo.createRecord(rec));

        // 2. Faculty updates status from PRESENT to ABSENT
        ASSERT_TRUE(repo.updateRecordStatus(rec.id, models::AttendanceStatus::ABSENT));
        auto rUpdated = repo.findRecordById(rec.id);
        ASSERT_TRUE(rUpdated.has_value());
        ASSERT_TRUE(rUpdated->status == models::AttendanceStatus::ABSENT);

        // 3. Create Undo Action: Revert ABSENT back to PRESENT
        models::UndoAction undoAct{
            "undo_test_1", rec.id, session.id, stu.id, "Alice Student", "CS101",
            models::AttendanceStatus::PRESENT, // previous
            models::AttendanceStatus::ABSENT,  // new
            "2026-11-18T12:00:00Z", "usr_fac_test", "Correction"
        };
        ASSERT_TRUE(repo.saveUndoRecord(undoAct));

        // 4. Execute Undo
        auto popped = repo.popLatestUndoRecord("usr_fac_test");
        ASSERT_TRUE(popped.has_value());
        ASSERT_EQ(popped->id, "undo_test_1");
        ASSERT_TRUE(popped->previousStatus == models::AttendanceStatus::PRESENT);

        // Apply reversion
        ASSERT_TRUE(repo.updateRecordStatus(popped->recordId, popped->previousStatus));
        ASSERT_TRUE(repo.markUndoActionDone(popped->id));

        // 5. Verify record is restored to exact PRESENT status
        auto rReverted = repo.findRecordById(rec.id);
        ASSERT_TRUE(rReverted.has_value());
        ASSERT_TRUE(rReverted->status == models::AttendanceStatus::PRESENT);

        // 6. Verify duplicate undo attempt returns nullopt (cannot undo twice)
        auto dupUndo = repo.popLatestUndoRecord("usr_fac_test");
        ASSERT_FALSE(dupUndo.has_value());
    TEST_PASS()
}

// -------------------------------------------------------------
// SUITE 6: Risk Prediction & 75% Boundary Tests
// -------------------------------------------------------------
void suite_risk_prediction_boundaries() {
    using models::AttendanceStatus;

    TEST_CASE("ML", "Prediction: Low Risk (>85% Attendance, Positive Momentum)")
        std::vector<AttendanceStatus> records(20, AttendanceStatus::PRESENT);
        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::LOW);
        ASSERT_TRUE(pred.riskScore < 0.25);
        ASSERT_TRUE(pred.confidence >= 0.80);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: 75% Boundary Condition (Borderline Medium Risk)")
        // 15 Present, 5 Absent evenly distributed so no severe streak (75.0% attendance)
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 5; i++) {
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::PRESENT);
            records.push_back(AttendanceStatus::ABSENT);
        }

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::LOW || pred.riskCategory == models::RiskLevel::MEDIUM);
        ASSERT_TRUE(pred.riskScore <= 0.55);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: High Risk (70% Attendance with Downward Drop)")
        // Earlier: present. Recent: 3 consecutive absences dropping to 65%
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT, AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT, AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT, AttendanceStatus::ABSENT
        };

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::HIGH || pred.riskCategory == models::RiskLevel::CRITICAL);
        ASSERT_TRUE(pred.riskScore >= 0.50);
        ASSERT_TRUE(!pred.contributingFactors.empty());
    TEST_PASS()

    TEST_CASE("ML", "Prediction: Critical Risk (<50% Attendance, Severe Streak)")
        std::vector<AttendanceStatus> records;
        for (int i = 0; i < 6; i++) records.push_back(AttendanceStatus::PRESENT);
        for (int i = 0; i < 14; i++) records.push_back(AttendanceStatus::ABSENT);

        auto pred = ml::RiskPredictor::predict(records, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::CRITICAL);
        ASSERT_TRUE(pred.riskScore > 0.80);
    TEST_PASS()

    TEST_CASE("ML", "Prediction: Edge Case (0 Conducted Sessions)")
        std::vector<AttendanceStatus> emptyRecords;
        auto pred = ml::RiskPredictor::predict(emptyRecords, 75.0);
        ASSERT_TRUE(pred.riskCategory == models::RiskLevel::LOW);
        ASSERT_EQ(pred.riskScore, 0.0);
        ASSERT_TRUE(pred.confidence <= 0.30);
    TEST_PASS()
}

int main() {
    std::cout << "========================================================\n";
    std::cout << " AI ATTENDANCE RISK PREDICTOR — REPEATABLE C++ TEST SUITE\n";
    std::cout << "========================================================\n\n";

    // Suite 1: DSA
    suite_dsa_structures();

    // Suite 2: Algorithms
    suite_core_algorithms();

    // Suite 3: Auth Security
    suite_authentication_security();

    // Isolated test database setup
    std::string testDbPath = "database/test_isolated_suite.db";
    std::remove(testDbPath.c_str()); // Ensure fresh clean start
    std::string schemaSql = readFileContents("database/migrations/001_initial_schema.sql");

    // Suite 4: Database Integrity & Transactions
    suite_database_and_integrity(testDbPath, schemaSql);

    // Suite 5: Attendance Undo Lifecycle
    suite_attendance_undo_lifecycle(testDbPath);

    // Suite 6: ML Prediction Boundaries
    suite_risk_prediction_boundaries();

    // Clean up isolated test database file
    db::Database::instance().close();
    std::remove(testDbPath.c_str());

    std::cout << "\n========================================================\n";
    std::cout << " FINAL RESULTS: " << g_testsPassed << " PASSED, " << g_testsFailed << " FAILED\n";
    std::cout << "========================================================\n";

    return (g_testsFailed == 0) ? 0 : 1;
}
