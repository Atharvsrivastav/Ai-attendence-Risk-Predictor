/**
 * DSA & ML VERIFICATION TEST SUITE
 * Run with: npm run test:dsa or node scripts/test-dsa.mjs
 */

import { StudentHashMap } from "../lib/dsa/data-structures/StudentHashMap.js";
import { AttendanceQueue } from "../lib/dsa/data-structures/AttendanceQueue.js";
import { AttendanceStack } from "../lib/dsa/data-structures/AttendanceStack.js";
import { AttendanceSet } from "../lib/dsa/data-structures/AttendanceSet.js";
import { AttendanceGraph } from "../lib/dsa/data-structures/AttendanceGraph.js";
import { RiskDecisionTree } from "../lib/dsa/data-structures/RiskDecisionTree.js";
import { mergeSortWithMetrics, quickSortWithMetrics } from "../lib/dsa/algorithms/sorting.js";
import { linearSearch, binarySearch } from "../lib/dsa/algorithms/searching.js";
import { calculateAttendancePercentage } from "../lib/dsa/algorithms/attendanceCalculator.js";
import { detectConsecutiveAbsences } from "../lib/dsa/algorithms/consecutiveAbsences.js";
import { analyzeAttendanceTrend } from "../lib/dsa/algorithms/trendAnalysis.js";
import { calculateRecovery, simulateAttendance } from "../lib/dsa/algorithms/recoveryCalculator.js";
import { extractAttendanceFeatures } from "../lib/ml/featureEngineering.js";
import { runLogisticRegression } from "../lib/ml/logisticRegressionModel.js";
import { predictAttendanceRisk } from "../lib/ml/riskPredictor.js";

let passed = 0;
let failed = 0;

function assert(condition, message) {
  if (condition) {
    console.log(`  ✓ PASS: ${message}`);
    passed++;
  } else {
    console.error(`  ✗ FAIL: ${message}`);
    failed++;
  }
}

async function runTests() {
  console.log("=================================================");
  console.log("🔬 RUNNING DSA & ML VERIFICATION TESTS");
  console.log("=================================================\n");

  // 1. StudentHashMap Tests
  console.log("--- 1. Testing StudentHashMap (O(1) Hash Table) ---");
  const map = new StudentHashMap(16);
  map.set("2024CS001", { name: "Aryan", score: 92 });
  map.set("2024CS002", { name: "Sneha", score: 88 });
  map.set("2024CS003", { name: "Rahul", score: 64 });

  assert(map.get("2024CS001")?.name === "Aryan", "HashMap retrieves key '2024CS001'");
  assert(map.get("2024CS003")?.score === 64, "HashMap retrieves key '2024CS003'");
  assert(map.has("2024CS002") === true, "HashMap has() returns true for existing key");
  assert(map.has("2024CS999") === false, "HashMap has() returns false for missing key");
  assert(map.getSize() === 3, "HashMap size is exactly 3");

  // 2. AttendanceQueue (FIFO) Tests
  console.log("\n--- 2. Testing AttendanceQueue (FIFO Event Processing) ---");
  const queue = new AttendanceQueue();
  queue.enqueue({ id: "e1", msg: "First Event" });
  queue.enqueue({ id: "e2", msg: "Second Event" });
  queue.enqueue({ id: "e3", msg: "Third Event" });

  assert(queue.size() === 3, "Queue size is 3 after 3 enqueues");
  assert(queue.peek()?.id === "e1", "Queue peek() returns first enqueued element (e1)");
  const deq1 = queue.dequeue();
  assert(deq1?.id === "e1", "Queue dequeue() maintains strict FIFO order (returns e1)");
  const deq2 = queue.dequeue();
  assert(deq2?.id === "e2", "Queue dequeue() returns second element (e2)");
  assert(queue.size() === 1, "Queue size correctly decremented to 1");

  // 3. AttendanceStack (LIFO Undo) Tests
  console.log("\n--- 3. Testing AttendanceStack (LIFO Undo Mechanism) ---");
  const stack = new AttendanceStack();
  stack.push({ id: "u1", action: "Marked Absent" });
  stack.push({ id: "u2", action: "Marked Late" });
  stack.push({ id: "u3", action: "Marked Present" });

  assert(stack.size() === 3, "Stack size is 3");
  assert(stack.peek()?.id === "u3", "Stack peek() returns top item (u3)");
  const pop1 = stack.pop();
  assert(pop1?.id === "u3", "Stack pop() returns most recent action (u3 - LIFO)");
  const pop2 = stack.pop();
  assert(pop2?.id === "u2", "Stack pop() returns subsequent previous action (u2)");
  assert(stack.size() === 1, "Stack size is now 1");

  // 4. AttendanceSet Tests
  console.log("\n--- 4. Testing AttendanceSet (Unique Entities & Set Operations) ---");
  const setA = new AttendanceSet(["s1", "s2", "s3", "s4"]);
  const setB = new AttendanceSet(["s3", "s4", "s5", "s6"]);
  const intersection = setA.intersection(setB);
  const union = setA.union(setB);

  assert(intersection.size() === 2, "Set Intersection size is 2 (s3, s4)");
  assert(intersection.has("s3") && intersection.has("s4"), "Set Intersection contains s3 and s4");
  assert(union.size() === 6, "Set Union size is 6 unique elements");

  // 5. AttendanceGraph (BFS & DFS) Tests
  console.log("\n--- 5. Testing AttendanceGraph (Adjacency List & Traversals) ---");
  const graph = new AttendanceGraph();
  graph.addNode({ id: "F1", label: "Prof Sharma", type: "FACULTY" });
  graph.addNode({ id: "S1", label: "CS301", type: "SUBJECT" });
  graph.addNode({ id: "S2", label: "CS304", type: "SUBJECT" });
  graph.addNode({ id: "ST1", label: "Rahul", type: "STUDENT" });
  graph.addNode({ id: "ST2", label: "Aryan", type: "STUDENT" });

  graph.addEdge({ source: "F1", target: "S1", relationship: "TEACHES" });
  graph.addEdge({ source: "F1", target: "S2", relationship: "TEACHES" });
  graph.addEdge({ source: "S1", target: "ST1", relationship: "ENROLLED_IN" });
  graph.addEdge({ source: "S1", target: "ST2", relationship: "ENROLLED_IN" });

  const bfs = graph.bfs("F1");
  assert(bfs.order[0] === "F1", "BFS starts with Root Faculty F1");
  assert(bfs.levels["F1"] === 0, "Level of F1 is 0");
  assert(bfs.levels["S1"] === 1 && bfs.levels["S2"] === 1, "Level of Subjects is 1");
  assert(bfs.levels["ST1"] === 2 && bfs.levels["ST2"] === 2, "Level of Students is 2");

  const path = graph.findShortestPath("F1", "ST1");
  assert(path?.join(" -> ") === "F1 -> S1 -> ST1", "Graph finds shortest path from F1 to ST1");

  // 6. Sorting Algorithms (MergeSort & QuickSort) Tests
  console.log("\n--- 6. Testing Sorting Algorithms (MergeSort & QuickSort) ---");
  const testScores = [
    { id: 1, val: 68 },
    { id: 2, val: 94 },
    { id: 3, val: 42 },
    { id: 4, val: 85 },
    { id: 5, val: 75 },
    { id: 6, val: 60 },
  ];

  const ms = mergeSortWithMetrics(testScores, (a, b) => a.val - b.val);
  const qs = quickSortWithMetrics(testScores, (a, b) => a.val - b.val);

  assert(ms.sorted[0].val === 42 && ms.sorted[5].val === 94, "MergeSort correctly sorted ascending");
  assert(qs.sorted[0].val === 42 && qs.sorted[5].val === 94, "QuickSort correctly sorted ascending");
  assert(ms.comparisons > 0, `MergeSort comparisons tracked (${ms.comparisons})`);
  assert(qs.comparisons > 0 && qs.swaps >= 0, `QuickSort comparisons (${qs.comparisons}) and swaps (${qs.swaps}) tracked`);

  // 7. Searching Algorithms (Linear vs Binary) Tests
  console.log("\n--- 7. Testing Searching Algorithms (Linear vs Binary) ---");
  const sortedRolls = ["2024CS001", "2024CS002", "2024CS003", "2024CS004", "2024CS005", "2024CS006", "2024CS007"];
  const lin = linearSearch(sortedRolls, (r) => r === "2024CS005", (r) => r);
  const bin = binarySearch(sortedRolls, (r) => r, "2024CS005", (a, b) => a.localeCompare(b));

  assert(lin.found === true && lin.index === 4, "Linear search found key at index 4");
  assert(bin.found === true && bin.index === 4, "Binary search found key at index 4");
  assert(bin.stepsCount <= Math.ceil(Math.log2(sortedRolls.length)) + 1, "Binary search completed in O(log n) steps");

  // 8. Attendance Calculation & Recovery Tests
  console.log("\n--- 8. Testing Attendance & Recovery Algorithms ---");
  const sampleAttendance = [
    "PRESENT", "PRESENT", "PRESENT", "PRESENT", "PRESENT", // 5
    "PRESENT", "PRESENT", "PRESENT", "PRESENT", "PRESENT", // 10
    "PRESENT", "PRESENT", "PRESENT", "PRESENT", "PRESENT", // 15
    "ABSENT", "ABSENT", "ABSENT", "ABSENT"                 // 19 (15 present, 4 absent)
  ];

  const calc = calculateAttendancePercentage(sampleAttendance, 75);
  assert(calc.totalConducted === 19, "Total conducted is 19");
  assert(calc.presentCount === 15, "Present count is 15");
  assert(calc.percentage === 78.95, `Calculated % is 78.95% (got ${calc.percentage}%)`);

  const consecutive = detectConsecutiveAbsences(sampleAttendance);
  assert(consecutive.currentStreak === 4, `Detected current consecutive absence streak of 4 (got ${consecutive.currentStreak})`);

  const trend = analyzeAttendanceTrend(sampleAttendance);
  assert(trend.trend === "DECLINING", `Trend correctly identified as DECLINING due to recent 4 absences (got ${trend.trend})`);

  // Recovery formula test:
  // 15 present out of 20 total = 75%. If threshold is 75%, needed is 0.
  const recSafe = calculateRecovery(15, 20, 75);
  assert(recSafe.classesNeededToRecover === 0, "Safe student needs 0 classes to recover");
  // 14 present out of 20 = 70%. Need X classes where (14 + X)/(20 + X) >= 0.75 => 0.25X >= 15 - 14 = 1 => X >= 4.
  const recDrop = calculateRecovery(14, 20, 75);
  assert(recDrop.classesNeededToRecover === 4, `Student at 70% needs 4 consecutive classes to reach 75% (got ${recDrop.classesNeededToRecover})`);

  // Simulation test: attend next 4 classes -> (14+4)/(20+4) = 18/24 = 75.0%
  const sim = simulateAttendance(14, 20, 4, 0, 75);
  assert(sim.projectedPercentage === 75, `Simulating 4 attended classes brings % to 75% (got ${sim.projectedPercentage}%)`);
  assert(sim.meetsThreshold === true, "Projected attendance meets threshold");

  // 9. ML Risk Prediction Tests
  console.log("\n--- 9. Testing ML Risk Prediction (Decision Tree + Logistic Regression) ---");
  const lowRiskRecords = Array(20).fill("PRESENT");
  const lowRiskPred = predictAttendanceRisk(lowRiskRecords, 75);
  assert(lowRiskPred.riskCategory === "LOW", `Full attendance classified as LOW risk (got ${lowRiskPred.riskCategory})`);

  const highRiskRecords = [
    ...Array(12).fill("PRESENT"),
    ...Array(4).fill("ABSENT"),
    ...Array(4).fill("ABSENT")
  ]; // 12 present out of 20 = 60%, with 4 consecutive absences
  const highRiskPred = predictAttendanceRisk(highRiskRecords, 75);
  assert(highRiskPred.riskCategory === "CRITICAL" || highRiskPred.riskCategory === "HIGH", `60% with streak classified as HIGH or CRITICAL risk (got ${highRiskPred.riskCategory})`);
  assert(highRiskPred.contributingFactors.length >= 2, "Prediction contains at least 2 explainable contributing factors");
  assert(highRiskPred.recommendation.length > 0, "Prediction contains actionable recovery recommendation");

  console.log("\n=================================================");
  console.log(`✨ TEST SUMMARY: ${passed} PASSED, ${failed} FAILED`);
  console.log("=================================================\n");

  if (failed > 0) {
    process.exit(1);
  }
}

runTests().catch(console.error);
