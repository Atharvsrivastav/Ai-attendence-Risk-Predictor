#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>

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

#define TEST_CASE(name) \
    std::cout << "[RUNNING] " << name << " ... " << std::flush; \
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

void test_StudentHashMap() {
    TEST_CASE("StudentHashMap: Insert, Get, Update, Remove, and Auto-Resize")
        dsa::StudentHashMap<std::string, int> map(4);
        ASSERT_EQ(map.size(), 0);

        map.set("student_1", 85);
        map.set("student_2", 92);
        map.set("student_3", 64);

        ASSERT_EQ(map.size(), 3);
        int score = 0;
        ASSERT_TRUE(map.get("student_1", score));
        ASSERT_EQ(score, 85);

        // Update
        map.set("student_1", 88);
        ASSERT_TRUE(map.get("student_1", score));
        ASSERT_EQ(score, 88);
        ASSERT_EQ(map.size(), 3);

        // Trigger rehash with more keys
        map.set("student_4", 75);
        map.set("student_5", 99);
        map.set("student_6", 45);
        ASSERT_TRUE(map.capacity() >= 8);

        // Remove
        ASSERT_TRUE(map.remove("student_2"));
        ASSERT_FALSE(map.has("student_2"));
        ASSERT_EQ(map.size(), 5);
        ASSERT_FALSE(map.remove("non_existing"));
    TEST_PASS()
}

void test_AttendanceQueue() {
    TEST_CASE("AttendanceQueue: FIFO Enqueue, Dequeue, Peek, and Thread Safety")
        dsa::AttendanceQueue<std::string> queue;
        ASSERT_TRUE(queue.isEmpty());
        ASSERT_EQ(queue.size(), 0);

        queue.enqueue("event_1");
        queue.enqueue("event_2");
        queue.enqueue("event_3");

        ASSERT_EQ(queue.size(), 3);
        auto peeked = queue.peek();
        ASSERT_TRUE(peeked.has_value());
        ASSERT_EQ(peeked.value(), "event_1");

        auto d1 = queue.dequeue();
        ASSERT_TRUE(d1.has_value());
        ASSERT_EQ(d1.value(), "event_1");

        auto d2 = queue.dequeue();
        ASSERT_TRUE(d2.has_value());
        ASSERT_EQ(d2.value(), "event_2");

        ASSERT_EQ(queue.size(), 1);
        auto d3 = queue.dequeue();
        ASSERT_TRUE(d3.has_value());
        ASSERT_EQ(d3.value(), "event_3");
        ASSERT_TRUE(queue.isEmpty());
        ASSERT_FALSE(queue.dequeue().has_value());
    TEST_PASS()
}

void test_AttendanceStack() {
    TEST_CASE("AttendanceStack: LIFO Push, Pop, Peek, and Depth Pruning")
        dsa::AttendanceStack<int> stack(3); // maxDepth = 3
        ASSERT_TRUE(stack.isEmpty());

        stack.push(10);
        stack.push(20);
        stack.push(30);
        ASSERT_EQ(stack.size(), 3);

        // Pushing 4th item should prune bottom item (10)
        stack.push(40);
        ASSERT_EQ(stack.size(), 3);

        auto p1 = stack.pop();
        ASSERT_TRUE(p1.has_value());
        ASSERT_EQ(p1.value(), 40);

        auto p2 = stack.pop();
        ASSERT_TRUE(p2.has_value());
        ASSERT_EQ(p2.value(), 30);

        auto p3 = stack.pop();
        ASSERT_TRUE(p3.has_value());
        ASSERT_EQ(p3.value(), 20);

        ASSERT_TRUE(stack.isEmpty());
    TEST_PASS()
}

void test_AttendanceSet() {
    TEST_CASE("AttendanceSet: Add, Remove, Intersect, Union, and Difference")
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
}

void test_RiskDecisionTree() {
    TEST_CASE("RiskDecisionTree: Hierarchical Classification of Risk Levels")
        dsa::RiskDecisionTree tree(75.0);

        // Low risk test: 90% overall, 92% recent, 0 consecutive absences, positive trend
        dsa::EvaluationFeatureInput inputLow{90.0, 92.0, 0, 0.05, 30, 75.0};
        auto resLow = tree.classify(inputLow);
        ASSERT_TRUE(resLow.riskCategory == models::RiskLevel::LOW);

        // Medium risk test: 74% overall, but recent is stable and no consecutive absences
        dsa::EvaluationFeatureInput inputMed{74.0, 75.0, 1, 0.0, 30, 75.0};
        auto resMed = tree.classify(inputMed);
        ASSERT_TRUE(resMed.riskCategory == models::RiskLevel::MEDIUM);

        // High risk test: 65% overall, sharp drop in recent attendance
        dsa::EvaluationFeatureInput inputHigh{65.0, 50.0, 2, -0.15, 30, 75.0};
        auto resHigh = tree.classify(inputHigh);
        ASSERT_TRUE(resHigh.riskCategory == models::RiskLevel::HIGH || resHigh.riskCategory == models::RiskLevel::CRITICAL);

        // Critical risk test: 45% overall, 4 consecutive absences
        dsa::EvaluationFeatureInput inputCrit{45.0, 20.0, 4, -0.25, 30, 75.0};
        auto resCrit = tree.classify(inputCrit);
        ASSERT_TRUE(resCrit.riskCategory == models::RiskLevel::CRITICAL);
    TEST_PASS()
}

void test_AttendanceGraph() {
    TEST_CASE("AttendanceGraph: BFS, DFS, and Shortest Path Traversal")
        dsa::AttendanceGraph graph;
        graph.addNode({"FAC_1", "Dr. Alan", "FACULTY", {}});
        graph.addNode({"SUB_CS101", "Computer Science", "SUBJECT", {}});
        graph.addNode({"STU_1", "Alice", "STUDENT", {}});
        graph.addNode({"STU_2", "Bob", "STUDENT", {}});

        graph.addEdge({"FAC_1", "SUB_CS101", "TEACHES"}, true);
        graph.addEdge({"SUB_CS101", "STU_1", "ENROLLED_IN"}, true);
        graph.addEdge({"SUB_CS101", "STU_2", "ENROLLED_IN"}, true);

        // BFS from Faculty
        auto bfsRes = graph.bfs("FAC_1");
        ASSERT_EQ(bfsRes.nodesVisited, 4);
        ASSERT_EQ(bfsRes.order[0], "FAC_1");
        ASSERT_EQ(bfsRes.order[1], "SUB_CS101");

        // DFS from Faculty
        auto dfsRes = graph.dfs("FAC_1");
        ASSERT_EQ(dfsRes.nodesVisited, 4);

        // Shortest Path from Dr. Alan to Alice: FAC_1 -> SUB_CS101 -> STU_1
        auto path = graph.findShortestPath("FAC_1", "STU_1");
        ASSERT_EQ(path.size(), 3);
        ASSERT_EQ(path[0], "FAC_1");
        ASSERT_EQ(path[1], "SUB_CS101");
        ASSERT_EQ(path[2], "STU_1");
    TEST_PASS()
}

void test_SortingAlgorithms() {
    TEST_CASE("Algorithms: MergeSort and QuickSort with Metrics")
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
}

void test_SearchingAlgorithms() {
    TEST_CASE("Algorithms: LinearSearch and BinarySearch with Step Traces")
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
}

void test_AttendanceCalculatorRules() {
    TEST_CASE("Attendance Calculations: Present=1, Late=0.5, Absent=0, Excused=Excluded")
        using models::AttendanceStatus;
        // 4 Present (4), 2 Late (1), 2 Absent (0), 2 Excused (excluded)
        // Total points = 5.0, Total conducted = 8
        // Expected percentage = 5 / 8 * 100 = 62.5%
        std::vector<AttendanceStatus> records = {
            AttendanceStatus::PRESENT,
            AttendanceStatus::PRESENT,
            AttendanceStatus::LATE,
            AttendanceStatus::ABSENT,
            AttendanceStatus::EXCUSED,
            AttendanceStatus::PRESENT,
            AttendanceStatus::LATE,
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
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
}

void test_ConsecutiveAbsences() {
    TEST_CASE("Consecutive Absences: Active Backward Streak & Max Semester Streak")
        using models::AttendanceStatus;
        // History: [Present, Absent, Absent, Absent, Present, Absent, Absent] (chronological)
        // Latest sessions are at end: 2 consecutive absences at present
        // Max semester streak is 3
        std::vector<AttendanceStatus> history = {
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::PRESENT,
            AttendanceStatus::ABSENT,
            AttendanceStatus::ABSENT
        };

        auto streakRes = algorithms::detectConsecutiveAbsences(history);
        ASSERT_EQ(streakRes.currentConsecutive, 2);
        ASSERT_EQ(streakRes.maxConsecutive, 3);
    TEST_PASS()
}

void test_RecentWindowAndTrend() {
    TEST_CASE("Sliding Window Analysis & Ordinary Least Squares Trend")
        using models::AttendanceStatus;
        // Earlier: all present. Recent 5: all absent.
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
}

void test_RecoveryCalculator() {
    TEST_CASE("Recovery Calculator: Formulas & Simulation")
        // Currently conducted: 20 sessions, 12 attended (60%). Threshold = 75%.
        // Formula: needed = ceil((0.75 * 20 - 12) / (1 - 0.75)) = ceil((15 - 12) / 0.25) = ceil(3 / 0.25) = 12
        auto recovery = algorithms::calculateRecovery(12.0, 20, 75.0);
        ASSERT_TRUE(recovery.isBelowThreshold);
        ASSERT_EQ(recovery.classesNeededToRecover, 12);

        // If currently conducted: 20 sessions, 18 attended (90%). Threshold = 75%.
        // Formula: allowable misses = floor((18 - 0.75 * 20) / 0.75) = floor(3 / 0.75) = 4
        auto safeStudent = algorithms::calculateRecovery(18.0, 20, 75.0);
        ASSERT_FALSE(safeStudent.isBelowThreshold);
        ASSERT_EQ(safeStudent.classesCanBeMissed, 4);

        // Simulation What-If: attend 10 more, miss 2 more
        auto sim = algorithms::simulateAttendance(12.0, 20, 10, 2, 75.0);
        ASSERT_EQ(sim.projectedTotalSessions, 32);
        ASSERT_NEAR(sim.projectedAttendedSessions, 22.0, 0.001);
        ASSERT_NEAR(sim.projectedPercentage, (22.0 / 32.0) * 100.0, 0.01);
    TEST_PASS()
}

void test_MLRiskPrediction() {
    TEST_CASE("Machine Learning: Logistic Regression & Ensemble Risk Classification")
        using models::AttendanceStatus;

        // Student with near 100% attendance
        std::vector<AttendanceStatus> goodRecords(25, AttendanceStatus::PRESENT);
        auto goodPred = ml::RiskPredictor::predict(goodRecords, 75.0);
        ASSERT_TRUE(goodPred.riskCategory == models::RiskLevel::LOW);
        ASSERT_TRUE(goodPred.riskScore < 0.25);

        // Student with severe attendance issues (<50%)
        std::vector<AttendanceStatus> badRecords;
        for (int i = 0; i < 10; i++) badRecords.push_back(AttendanceStatus::PRESENT);
        for (int i = 0; i < 15; i++) badRecords.push_back(AttendanceStatus::ABSENT);
        auto badPred = ml::RiskPredictor::predict(badRecords, 75.0);
        ASSERT_TRUE(badPred.riskCategory == models::RiskLevel::CRITICAL || badPred.riskCategory == models::RiskLevel::HIGH);
        ASSERT_TRUE(badPred.riskScore > 0.60);
        ASSERT_TRUE(!badPred.contributingFactors.empty());
    TEST_PASS()
}

void test_AuthService() {
    TEST_CASE("AuthService: Hashing, Verification, and Signed Tokens")
        std::string password = "AdminPassword@123";
        std::string hashed = auth::AuthService::hashPassword(password);
        ASSERT_TRUE(hashed.length() > 0);
        ASSERT_TRUE(auth::AuthService::verifyPassword(password, hashed));
        ASSERT_FALSE(auth::AuthService::verifyPassword("WrongPassword", hashed));

        auth::SessionClaims claims{
            "user_123",
            "admin@univ.edu",
            "System Administrator",
            models::UserRole::ADMIN,
            "",
            ""
        };
        std::string token = auth::AuthService::generateToken(claims, "test_jwt_secret_key");
        ASSERT_TRUE(!token.empty());

        auto verified = auth::AuthService::verifyToken(token, "test_jwt_secret_key");
        ASSERT_TRUE(verified.has_value());
        ASSERT_EQ(verified->userId, "user_123");
        ASSERT_EQ(verified->email, "admin@univ.edu");
        ASSERT_TRUE(verified->role == models::UserRole::ADMIN);

        // Invalid token
        ASSERT_FALSE(auth::AuthService::verifyToken("invalid.token.payload", "test_jwt_secret_key").has_value());
    TEST_PASS()
}

void test_DatabaseAndRepositories() {
    TEST_CASE("Database & Repositories: SQLite Persistence, Users, Students, Sessions, Undo")
        auto& database = db::Database::instance();
        database.open("database/attendance.db");
        repositories::AttendanceRepository repo(database);

        // Query migrated admin
        auto admin = repo.findUserByEmail("admin@college.edu");
        ASSERT_TRUE(admin.has_value());
        ASSERT_EQ(admin->name, "Dr. K. R. Raman (Dean of Academics)");
        ASSERT_TRUE(admin->role == models::UserRole::ADMIN);

        // Query all students
        auto students = repo.findAllStudents();
        ASSERT_TRUE(!students.empty());

        // Query attendance records for first student
        auto records = repo.findRecordsByStudent(students[0].id);
        ASSERT_TRUE(!records.empty());

        // Test persistent undo record
        models::UndoAction testUndo;
        testUndo.id = "test_undo_001";
        testUndo.sessionId = "session_test";
        testUndo.recordId = "record_test";
        testUndo.studentId = students[0].id;
        testUndo.studentName = students[0].name;
        testUndo.subjectCode = "CS101";
        testUndo.previousStatus = models::AttendanceStatus::ABSENT;
        testUndo.newStatus = models::AttendanceStatus::PRESENT;
        testUndo.facultyId = admin->id;
        testUndo.timestamp = "2026-11-18T12:00:00Z";
        testUndo.reason = "Manual correction test";

        ASSERT_TRUE(repo.saveUndoRecord(testUndo));
        auto poppedUndo = repo.popLatestUndoRecord();
        ASSERT_TRUE(poppedUndo.has_value());
        ASSERT_EQ(poppedUndo->id, "test_undo_001");
        ASSERT_EQ(poppedUndo->studentId, students[0].id);
    TEST_PASS()
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "    AI ATTENDANCE RISK PREDICTOR - C++ TEST SUITE       \n";
    std::cout << "========================================================\n\n";

    // 1. DSA Tests
    test_StudentHashMap();
    test_AttendanceQueue();
    test_AttendanceStack();
    test_AttendanceSet();
    test_RiskDecisionTree();
    test_AttendanceGraph();

    // 2. Algorithm Tests
    test_SortingAlgorithms();
    test_SearchingAlgorithms();
    test_AttendanceCalculatorRules();
    test_ConsecutiveAbsences();
    test_RecentWindowAndTrend();
    test_RecoveryCalculator();

    // 3. ML Tests
    test_MLRiskPrediction();

    // 4. Auth & Database Tests
    test_AuthService();
    test_DatabaseAndRepositories();

    std::cout << "\n========================================================\n";
    std::cout << " TEST RESULTS: " << g_testsPassed << " PASSED, " << g_testsFailed << " FAILED\n";
    std::cout << "========================================================\n";

    return (g_testsFailed == 0) ? 0 : 1;
}
