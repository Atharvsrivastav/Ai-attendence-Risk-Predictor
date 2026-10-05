"use client";

import React, { useEffect, useState } from "react";
import {
  Binary,
  Layers,
  Database,
  Cpu,
  GitBranch,
  BrainCircuit,
  ShieldCheck,
  Zap,
  Play,
  CheckCircle2,
  AlertCircle,
  HelpCircle,
  Clock,
  ArrowRight,
  TrendingDown,
} from "lucide-react";

export default function DsaLabPage() {
  const [activeTab, setActiveTab] = useState<"structures" | "algorithms" | "viva">("structures");
  const [activeDS, setActiveDS] = useState<"hashmap" | "queue" | "stack" | "set" | "tree" | "graph" | "array">("hashmap");

  // Benchmark API data
  const [benchmarkData, setBenchmarkData] = useState<any>(null);
  const [graphData, setGraphData] = useState<any>(null);
  const [loadingBenchmarks, setLoadingBenchmarks] = useState(false);

  // Interactive Live Benchmarking state
  const [searchTarget, setSearchTarget] = useState("2024CS015");
  const [activeBfsOrder, setActiveBfsOrder] = useState<string[]>([]);
  const [activeDfsOrder, setActiveDfsOrder] = useState<string[]>([]);

  useEffect(() => {
    fetchBenchmarks();
    fetchGraph();
  }, []);

  const fetchBenchmarks = async () => {
    try {
      setLoadingBenchmarks(true);
      const res = await fetch("/api/dsa/benchmark");
      if (res.ok) {
        const data = await res.json();
        setBenchmarkData(data);
      }
    } catch (err) {
      console.error("Benchmark fetch error:", err);
    } finally {
      setLoadingBenchmarks(false);
    }
  };

  const fetchGraph = async () => {
    try {
      const res = await fetch("/api/dsa/graph");
      if (res.ok) {
        const data = await res.json();
        setGraphData(data);
        setActiveBfsOrder(data.traversals.bfs.traversalOrder);
        setActiveDfsOrder(data.traversals.dfs.traversalOrder);
      }
    } catch (err) {
      console.error("Graph fetch error:", err);
    }
  };

  const dsaCards = [
    {
      id: "hashmap",
      name: "HashMap / Dictionary",
      icon: Database,
      timeComp: "O(1) Avg",
      spaceComp: "O(n)",
      purpose: "O(1) average lookup for student records, subject-wise grouping, and roll number indexing.",
      whereUsed: "lib/dsa/data-structures/StudentHashMap.ts & AttendanceService",
      input: "Student ID or Roll Number string key",
      output: "StudentAttendanceProfile object",
      vivaQuestion: "Why did you use HashMap instead of a database scan or LinkedList?",
      vivaAnswer:
        "Database scans or LinkedList traversals take O(n) linear time per search. With 24 to 2,000 students, HashMap provides deterministic O(1) hash bucket indexing via polynomial rolling hash with separate chaining for collision resolution.",
    },
    {
      id: "queue",
      name: "Queue (FIFO)",
      icon: GitBranch,
      timeComp: "O(1)",
      spaceComp: "O(n)",
      purpose: "Processes chronological attendance events and risk alert dispatches sequentially.",
      whereUsed: "lib/dsa/data-structures/AttendanceQueue.ts & POST /api/attendance",
      input: "AttendanceEvent object (timestamp, status, studentId)",
      output: "Sequentially processed event at queue head",
      vivaQuestion: "Why use a linked-node Queue rather than an Array with shift()?",
      vivaAnswer:
        "Array.shift() takes O(n) because every remaining element must shift left in contiguous memory. Our AttendanceQueue uses head and tail node pointers, achieving true O(1) Enqueue and O(1) Dequeue.",
    },
    {
      id: "stack",
      name: "Stack (LIFO)",
      icon: Cpu,
      timeComp: "O(1)",
      spaceComp: "O(k)",
      purpose: "Undo mechanism for faculty attendance roll call corrections and audit trail rollback.",
      whereUsed: "lib/dsa/data-structures/AttendanceStack.ts & POST /api/attendance/undo",
      input: "AttendanceCorrectionAction (recordId, previousStatus, newStatus)",
      output: "Most recent action popped for rollback",
      vivaQuestion: "How is Stack meaningfully used in this attendance project?",
      vivaAnswer:
        "During live roll calls, professors often accidentally mark a student absent. The AttendanceStack stores historical state changes. Clicking 'Undo Last Action' executes a LIFO pop in O(1) time and restores the exact previous attendance state in the database.",
    },
    {
      id: "set",
      name: "Set (HashSet)",
      icon: ShieldCheck,
      timeComp: "O(1) add / has",
      spaceComp: "O(n)",
      purpose: "Enforcing unique student IDs, distinct dates, and computing absentee intersections across subjects.",
      whereUsed: "lib/dsa/data-structures/AttendanceSet.ts",
      input: "Collection of identifiers",
      output: "Distinct set or set intersection / union",
      vivaQuestion: "What set operation did you implement and why?",
      vivaAnswer:
        "We implemented Set Intersection in O(min(|A|, |B|)) to find multi-subject chronic absentees (students absent in both CS301 and CS302 on the same day), and Set Union to aggregate unique high-risk cohorts.",
    },
    {
      id: "tree",
      name: "Hierarchical Tree",
      icon: BrainCircuit,
      timeComp: "O(h) traversal",
      spaceComp: "O(nodes)",
      purpose: "White-box, explainable risk decision tree with branching conditions on thresholds and streaks.",
      whereUsed: "lib/dsa/data-structures/RiskDecisionTree.ts & lib/ml/riskPredictor.ts",
      input: "Student feature values (overall %, trend, consecutive absences)",
      output: "Risk category (LOW, MEDIUM, HIGH, CRITICAL) + Decision Path",
      vivaQuestion: "How does the Tree relate to Machine Learning in your project?",
      vivaAnswer:
        "It provides white-box Explainable AI (XAI). Unlike opaque neural nets, the Tree structure records each binary decision node (e.g. Attendance <= 75% -> Declining Trend -> Streak >= 3), outputting the exact logical path explaining why the student is at risk.",
    },
    {
      id: "graph",
      name: "Graph (Adjacency List)",
      icon: Binary,
      timeComp: "O(V + E)",
      spaceComp: "O(V + E)",
      purpose: "Models college relationships: Faculty -> Subject -> Student, with BFS and DFS traversals.",
      whereUsed: "lib/dsa/data-structures/AttendanceGraph.ts & GET /api/dsa/graph",
      input: "Vertices (Users, Subjects) and Edges (TEACHES, ENROLLED_IN)",
      output: "BFS layer order, DFS paths, shortest connection routes",
      vivaQuestion: "Explain how BFS and DFS are used on your attendance graph.",
      vivaAnswer:
        "BFS uses a FIFO queue to discover entities layer-by-layer (Level 0: Faculty -> Level 1: Subjects -> Level 2: Enrolled Students). DFS uses recursion/stack to trace deep paths and verify connectivity and co-enrollment cycles.",
    },
    {
      id: "array",
      name: "Array / Sliding Window",
      icon: Layers,
      timeComp: "O(1) index, O(k) window",
      spaceComp: "O(n)",
      purpose: "Chronological sequence storage, tail sliding window for recent 5 classes, and contiguous streak scanning.",
      whereUsed: "lib/dsa/algorithms/recentWindowAnalysis.ts & consecutiveAbsences.ts",
      input: "Chronological AttendanceStatus[]",
      output: "Recent window percentage, active absence streak",
      vivaQuestion: "How does the sliding window algorithm work on attendance arrays?",
      vivaAnswer:
        "It takes a slice of the last k sessions (e.g., k = 5) to compute short-term momentum. If recent attendance drops by 15% compared to the semester baseline, the system flags an early warning before the student fails eligibility.",
    },
  ];

  const vivaQuestions = [
    {
      q: "1. What is the core problem statement of this project?",
      a: "Colleges struggle with late identification of attendance defaulters. Students only discover they are ineligible (below 75%) right before exams when it is mathematically impossible to recover. This system uses DSA for data organization and ML for predictive early warnings with exact recovery paths.",
    },
    {
      q: "2. How did you implement MergeSort and QuickSort?",
      a: "Both were manually implemented in TypeScript. MergeSort uses a divide-and-conquer strategy that guarantees O(n log n) comparisons in all cases with O(n) auxiliary buffer. QuickSort uses Lomuto partitioning in O(n log n) average time and O(log n) stack space. Our dashboard tracks comparisons, swaps, and microseconds live.",
    },
    {
      q: "3. What is the mathematical formula used for attendance recovery?",
      a: "Let T = total conducted classes, P = attended classes, and R = 0.75 (threshold). For a student below 75%, the minimum consecutive classes X to attend is: (P + X) / (T + X) >= R ==> X >= (R*T - P) / (1 - R). For 75%, X = ceil((0.75*T - P) / 0.25).",
    },
    {
      q: "4. Why use both Decision Tree and Logistic Regression?",
      a: "Decision Trees give transparent, rule-based branching (Explainable AI) that examiners can verify step-by-step. Logistic Regression outputs a continuous mathematical probability P(Risk) in [0, 1] with feature log-odds weights. We ensemble them for robust, explainable risk classifications.",
    },
    {
      q: "5. What is the difference between Linear Search and Binary Search in your app?",
      a: "Linear Search operates on unsorted arrays in O(n) time to find students by name or keyword. Binary Search operates on roll-number sorted arrays in O(log n) time by halving the search space at each mid-point check.",
    },
    {
      q: "6. How is the attendance trend calculated mathematically?",
      a: "We perform Ordinary Least Squares (OLS) Linear Regression over chronological session outcomes (Present = 1.0, Absent = 0.0). A negative slope (< -0.05) indicates declining attendance, while a positive slope indicates recovery.",
    },
    {
      q: "7. How do you handle collisions in your StudentHashMap?",
      a: "We use Separate Chaining. Each hash table bucket points to a linked list of HashNodes. If two student IDs hash to the same bucket index via our djb2 polynomial rolling hash, they are appended to that bucket's chain. When load factor exceeds 0.75, the table dynamically doubles capacity and rehashes.",
    },
    {
      q: "8. Is there any fake data in the system?",
      a: "No. The system strictly complies with the 'No Fake Data' rule. Every statistic, percentage, streak, slope, and ML probability is dynamically calculated from actual stored AttendanceRecord rows in the PostgreSQL/SQLite database.",
    },
  ];

  return (
    <div className="space-y-8">
      {/* Page Header */}
      <div className="border-b border-slate-200 dark:border-slate-800 pb-5">
        <div className="flex items-center gap-2.5">
          <div className="p-2 rounded-xl bg-indigo-600 text-white shadow-md shadow-indigo-600/20">
            <Binary className="h-6 w-6" />
          </div>
          <div>
            <h1 className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 dark:text-slate-100">
              DSA Analysis & Viva Center
            </h1>
            <p className="text-xs text-slate-500">
              Interactive demonstration of Data Structures, Sorting, Searching, Graph Traversals, and Viva Q&A
            </p>
          </div>
        </div>

        {/* Navigation Tabs */}
        <div className="flex items-center gap-2 mt-6">
          <button
            onClick={() => setActiveTab("structures")}
            className={`px-4 py-2 rounded-xl text-xs font-semibold transition-all ${
              activeTab === "structures"
                ? "bg-indigo-600 text-white shadow-md shadow-indigo-600/20"
                : "bg-white dark:bg-slate-800 text-slate-600 dark:text-slate-300 border border-slate-200 dark:border-slate-700"
            }`}
          >
            1. Data Structures in Action
          </button>

          <button
            onClick={() => setActiveTab("algorithms")}
            className={`px-4 py-2 rounded-xl text-xs font-semibold transition-all ${
              activeTab === "algorithms"
                ? "bg-indigo-600 text-white shadow-md shadow-indigo-600/20"
                : "bg-white dark:bg-slate-800 text-slate-600 dark:text-slate-300 border border-slate-200 dark:border-slate-700"
            }`}
          >
            2. Algorithms & Live Benchmarks
          </button>

          <button
            onClick={() => setActiveTab("viva")}
            className={`px-4 py-2 rounded-xl text-xs font-semibold transition-all ${
              activeTab === "viva"
                ? "bg-indigo-600 text-white shadow-md shadow-indigo-600/20"
                : "bg-white dark:bg-slate-800 text-slate-600 dark:text-slate-300 border border-slate-200 dark:border-slate-700"
            }`}
          >
            3. Viva Exam Cheatsheet (Q&A)
          </button>
        </div>
      </div>

      {/* TAB 1: DATA STRUCTURES IN ACTION */}
      {activeTab === "structures" && (
        <div className="space-y-6">
          {/* Data Structure Selector Chips */}
          <div className="flex flex-wrap items-center gap-2">
            {dsaCards.map((ds) => {
              const Icon = ds.icon;
              const isActive = activeDS === ds.id;
              return (
                <button
                  key={ds.id}
                  onClick={() => setActiveDS(ds.id as any)}
                  className={`flex items-center gap-2 px-3 py-2 rounded-xl text-xs font-semibold transition-all ${
                    isActive
                      ? "bg-indigo-600 text-white shadow-md shadow-indigo-600/20"
                      : "bg-white dark:bg-slate-800 text-slate-700 dark:text-slate-300 border border-slate-200 dark:border-slate-700 hover:border-indigo-400"
                  }`}
                >
                  <Icon className="h-4 w-4" />
                  <span>{ds.name}</span>
                </button>
              );
            })}
          </div>

          {/* Detailed Data Structure Inspector Card */}
          {(() => {
            const current = dsaCards.find((c) => c.id === activeDS)!;
            const Icon = current.icon;

            return (
              <div className="bg-white dark:bg-slate-900 rounded-3xl p-6 sm:p-8 border border-slate-200 dark:border-slate-800 shadow-sm space-y-6">
                <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 border-b border-slate-100 dark:border-slate-800 pb-5">
                  <div className="flex items-center gap-3">
                    <div className="p-3 rounded-2xl bg-indigo-50 dark:bg-indigo-950 text-indigo-600 dark:text-indigo-400">
                      <Icon className="h-6 w-6" />
                    </div>
                    <div>
                      <h2 className="text-xl font-bold text-slate-900 dark:text-slate-100">
                        {current.name}
                      </h2>
                      <p className="text-xs text-slate-500 font-mono mt-0.5">
                        Implemented in: {current.whereUsed}
                      </p>
                    </div>
                  </div>

                  <div className="flex items-center gap-2 font-mono text-xs">
                    <span className="px-3 py-1 rounded-lg bg-emerald-50 dark:bg-emerald-950 text-emerald-700 dark:text-emerald-300 font-bold border border-emerald-200 dark:border-emerald-800">
                      Time: {current.timeComp}
                    </span>
                    <span className="px-3 py-1 rounded-lg bg-indigo-50 dark:bg-indigo-950 text-indigo-700 dark:text-indigo-300 font-bold border border-indigo-200 dark:border-indigo-800">
                      Space: {current.spaceComp}
                    </span>
                  </div>
                </div>

                <div className="grid grid-cols-1 md:grid-cols-3 gap-4 text-xs">
                  <div className="p-4 rounded-2xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-800 space-y-1">
                    <p className="font-bold text-slate-400 uppercase text-[10px]">1. Engineering Purpose</p>
                    <p className="text-slate-800 dark:text-slate-200 leading-relaxed font-medium">
                      {current.purpose}
                    </p>
                  </div>

                  <div className="p-4 rounded-2xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-800 space-y-1">
                    <p className="font-bold text-slate-400 uppercase text-[10px]">2. Input & Output Contract</p>
                    <p className="text-slate-800 dark:text-slate-200">
                      <strong>Input:</strong> {current.input}
                    </p>
                    <p className="text-slate-800 dark:text-slate-200">
                      <strong>Output:</strong> {current.output}
                    </p>
                  </div>

                  <div className="p-4 rounded-2xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-800 space-y-1">
                    <p className="font-bold text-slate-400 uppercase text-[10px]">3. Complexity Justification</p>
                    <p className="text-slate-800 dark:text-slate-200 leading-relaxed">
                      Matches the real-world workload: lookups are bounded, preventing O(n) table scans on high concurrent query rates.
                    </p>
                  </div>
                </div>

                {/* Viva Defense Highlight */}
                <div className="p-4 rounded-2xl bg-indigo-50/70 dark:bg-indigo-950/40 border border-indigo-200 dark:border-indigo-800/70 space-y-2">
                  <p className="text-xs font-bold text-indigo-900 dark:text-indigo-200 flex items-center gap-1.5">
                    <HelpCircle className="h-4 w-4 text-indigo-600" />
                    Viva Defense: {current.vivaQuestion}
                  </p>
                  <p className="text-xs text-indigo-800 dark:text-indigo-300 leading-relaxed">
                    {current.vivaAnswer}
                  </p>
                </div>

                {/* Live Interactive Visualization for Selected Structure */}
                {activeDS === "hashmap" && benchmarkData && (
                  <div className="p-4 rounded-2xl bg-slate-900 text-white space-y-3 font-mono text-xs">
                    <div className="flex items-center justify-between">
                      <span className="text-cyan-400 font-bold">
                        Live HashMap Diagnostics & Hash Table Layout
                      </span>
                      <span className="text-[10px] text-slate-400">Polynomial Rolling Hash djb2</span>
                    </div>

                    <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 text-center">
                      <div className="p-2 rounded bg-slate-800">
                        <p className="text-[10px] text-slate-400">Total Entries</p>
                        <p className="text-base font-bold text-white">
                          {benchmarkData.lookupBenchmark.hashMapDiagnostics.size}
                        </p>
                      </div>
                      <div className="p-2 rounded bg-slate-800">
                        <p className="text-[10px] text-slate-400">Table Capacity</p>
                        <p className="text-base font-bold text-white">
                          {benchmarkData.lookupBenchmark.hashMapDiagnostics.capacity}
                        </p>
                      </div>
                      <div className="p-2 rounded bg-slate-800">
                        <p className="text-[10px] text-slate-400">Load Factor</p>
                        <p className="text-base font-bold text-emerald-400">
                          {benchmarkData.lookupBenchmark.hashMapDiagnostics.loadFactor}
                        </p>
                      </div>
                      <div className="p-2 rounded bg-slate-800">
                        <p className="text-[10px] text-slate-400">Collisions</p>
                        <p className="text-base font-bold text-amber-400">
                          {benchmarkData.lookupBenchmark.hashMapDiagnostics.collisionCount}
                        </p>
                      </div>
                    </div>

                    <div className="p-3 rounded bg-slate-800/80 border border-slate-700 flex items-center justify-between">
                      <span>
                        Live Benchmark (10,000 queries): HashMap O(1) avg{" "}
                        <strong className="text-emerald-400">{benchmarkData.lookupBenchmark.hashMapAverageUs} µs</strong>{" "}
                        vs Array.find O(n) avg{" "}
                        <strong className="text-rose-400">{benchmarkData.lookupBenchmark.arrayScanAverageUs} µs</strong>
                      </span>
                      <span className="text-xs font-bold text-cyan-300">
                        {benchmarkData.lookupBenchmark.speedupFactor}x faster!
                      </span>
                    </div>
                  </div>
                )}

                {activeDS === "graph" && graphData && (
                  <div className="p-4 rounded-2xl bg-slate-900 text-white space-y-4 font-mono text-xs">
                    <div className="flex items-center justify-between">
                      <span className="text-cyan-400 font-bold">
                        Academic Graph Traversal Visualizer (Faculty &rarr; Subject &rarr; Students)
                      </span>
                      <span className="text-[10px] text-slate-400">
                        {graphData.graph.nodes.length} Nodes &bull; {graphData.graph.edges.length} Edges
                      </span>
                    </div>

                    {/* BFS Display */}
                    <div className="p-3 rounded bg-slate-800 border border-slate-700 space-y-1.5">
                      <div className="flex items-center justify-between">
                        <span className="font-bold text-emerald-400">
                          Breadth-First Search (BFS) Traversal Order:
                        </span>
                        <span className="text-[10px] text-slate-400">Queue (FIFO) &bull; O(V + E)</span>
                      </div>
                      <div className="flex flex-wrap items-center gap-1.5 pt-1">
                        {activeBfsOrder.slice(0, 10).map((nodeId, idx) => (
                          <span
                            key={nodeId}
                            className="px-2 py-0.5 rounded bg-slate-700 text-white text-[11px]"
                          >
                            {idx + 1}. {nodeId}
                          </span>
                        ))}
                        {activeBfsOrder.length > 10 && (
                          <span className="text-slate-400 text-[10px]">
                            +{activeBfsOrder.length - 10} more nodes
                          </span>
                        )}
                      </div>
                    </div>

                    {/* DFS Display */}
                    <div className="p-3 rounded bg-slate-800 border border-slate-700 space-y-1.5">
                      <div className="flex items-center justify-between">
                        <span className="font-bold text-indigo-400">
                          Depth-First Search (DFS) Traversal Path:
                        </span>
                        <span className="text-[10px] text-slate-400">Recursion / Stack &bull; O(V + E)</span>
                      </div>
                      <div className="flex flex-wrap items-center gap-1.5 pt-1">
                        {activeDfsOrder.slice(0, 10).map((nodeId, idx) => (
                          <span
                            key={nodeId}
                            className="px-2 py-0.5 rounded bg-slate-700 text-white text-[11px]"
                          >
                            {idx + 1}. {nodeId}
                          </span>
                        ))}
                      </div>
                    </div>
                  </div>
                )}
              </div>
            );
          })()}
        </div>
      )}

      {/* TAB 2: ALGORITHMS & BENCHMARKS */}
      {activeTab === "algorithms" && (
        <div className="space-y-6">
          {benchmarkData ? (
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
              {/* Sorting Algorithms Benchmark Card */}
              <div className="bg-white dark:bg-slate-900 rounded-3xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
                <div className="flex items-center justify-between">
                  <div>
                    <h3 className="font-bold text-base text-slate-900 dark:text-slate-100 flex items-center gap-2">
                      <ArrowRight className="h-4 w-4 text-indigo-600" />
                      MergeSort vs QuickSort Benchmark
                    </h3>
                    <p className="text-xs text-slate-500">
                      Executed over {benchmarkData.datasetSize} live student attendance profiles
                    </p>
                  </div>
                  <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-indigo-50 dark:bg-indigo-950 text-indigo-600">
                    O(n log n)
                  </span>
                </div>

                <div className="space-y-3 font-mono text-xs">
                  {/* MergeSort */}
                  <div className="p-3.5 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-800/60 space-y-1">
                    <div className="flex items-center justify-between">
                      <span className="font-bold text-indigo-600 dark:text-indigo-400">MergeSort (Divide & Conquer)</span>
                      <span className="font-bold text-slate-900 dark:text-slate-100">
                        {benchmarkData.sorting.mergeSort.durationMicroseconds} µs
                      </span>
                    </div>
                    <p className="text-slate-500 text-[11px]">
                      Comparisons: {benchmarkData.sorting.mergeSort.comparisons} &bull; Swaps: 0 (Buffer merge)
                    </p>
                    <p className="text-[10px] text-slate-400">
                      Stable: Yes &bull; Guaranteed O(n log n) even in worst case. Space O(n).
                    </p>
                  </div>

                  {/* QuickSort */}
                  <div className="p-3.5 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-800/60 space-y-1">
                    <div className="flex items-center justify-between">
                      <span className="font-bold text-cyan-600 dark:text-cyan-400">QuickSort (Partitioning)</span>
                      <span className="font-bold text-slate-900 dark:text-slate-100">
                        {benchmarkData.sorting.quickSort.durationMicroseconds} µs
                      </span>
                    </div>
                    <p className="text-slate-500 text-[11px]">
                      Comparisons: {benchmarkData.sorting.quickSort.comparisons} &bull; Swaps: {benchmarkData.sorting.quickSort.swaps}
                    </p>
                    <p className="text-[10px] text-slate-400">
                      In-place: Yes &bull; Space O(log n) call stack. Avg O(n log n).
                    </p>
                  </div>

                  {/* Native TimSort */}
                  <div className="p-3.5 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-800/60 space-y-1">
                    <div className="flex items-center justify-between">
                      <span className="font-bold text-slate-600 dark:text-slate-400">V8 Native Sort (TimSort)</span>
                      <span className="font-bold text-slate-900 dark:text-slate-100">
                        {benchmarkData.sorting.nativeSort.durationMicroseconds} µs
                      </span>
                    </div>
                    <p className="text-slate-500 text-[11px]">Hybrid Insertion + MergeSort</p>
                  </div>
                </div>
              </div>

              {/* Searching Algorithms Benchmark Card */}
              <div className="bg-white dark:bg-slate-900 rounded-3xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
                <div className="flex items-center justify-between">
                  <div>
                    <h3 className="font-bold text-base text-slate-900 dark:text-slate-100 flex items-center gap-2">
                      <ArrowRight className="h-4 w-4 text-emerald-600" />
                      Linear Search vs Binary Search Trace
                    </h3>
                    <p className="text-xs text-slate-500">
                      Target Search Key: <strong className="font-mono">{benchmarkData.searching.target}</strong>
                    </p>
                  </div>
                  <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-emerald-50 dark:bg-emerald-950 text-emerald-600">
                    O(n) vs O(log n)
                  </span>
                </div>

                <div className="space-y-3 font-mono text-xs">
                  {/* Binary Search */}
                  <div className="p-3.5 rounded-xl border border-emerald-200 dark:border-emerald-800/60 bg-emerald-50/50 dark:bg-emerald-950/30 space-y-2">
                    <div className="flex items-center justify-between">
                      <span className="font-bold text-emerald-700 dark:text-emerald-400">
                        Binary Search (Requires Sorted Array)
                      </span>
                      <span className="font-bold text-emerald-700 dark:text-emerald-300">
                        {benchmarkData.searching.binarySearch.stepsCount} steps &bull; {benchmarkData.searching.binarySearch.timeMicroseconds} µs
                      </span>
                    </div>
                    <div className="space-y-1 text-[11px] text-slate-600 dark:text-slate-300">
                      {benchmarkData.searching.binarySearch.trace.map((step: any, i: number) => (
                        <div key={i} className="flex items-center gap-2">
                          <span className="text-emerald-600 font-bold">Step {i + 1}:</span>
                          <span>{step.actionTaken}</span>
                        </div>
                      ))}
                    </div>
                  </div>

                  {/* Linear Search */}
                  <div className="p-3.5 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-800/60 space-y-2">
                    <div className="flex items-center justify-between">
                      <span className="font-bold text-slate-700 dark:text-slate-300">
                        Linear Search (Unsorted Fallback)
                      </span>
                      <span className="font-bold text-slate-700 dark:text-slate-300">
                        {benchmarkData.searching.linearSearch.stepsCount} steps &bull; {benchmarkData.searching.linearSearch.timeMicroseconds} µs
                      </span>
                    </div>
                    <p className="text-[11px] text-slate-500">
                      Scanned through {benchmarkData.searching.linearSearch.stepsCount} array elements sequentially before finding match.
                    </p>
                  </div>
                </div>
              </div>
            </div>
          ) : (
            <div className="py-12 text-center text-xs text-slate-400">Running DSA benchmarks...</div>
          )}
        </div>
      )}

      {/* TAB 3: VIVA CHEATSHEET */}
      {activeTab === "viva" && (
        <div className="space-y-4">
          <div className="p-4 rounded-2xl bg-indigo-50/70 dark:bg-indigo-950/40 border border-indigo-200 dark:border-indigo-800 text-xs text-indigo-900 dark:text-indigo-200">
            <span className="font-bold">Viva Examination Guidelines: </span>
            Use these precise, technically verified answers when explaining the project to external examiners. Every answer matches the active code implementation in the repository.
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {vivaQuestions.map((item, idx) => (
              <div
                key={idx}
                className="bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm space-y-2"
              >
                <h3 className="font-bold text-xs text-slate-900 dark:text-slate-100 flex items-start gap-2">
                  <span className="h-5 w-5 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950 dark:text-indigo-300 flex items-center justify-center font-mono text-[10px] shrink-0 mt-0.5">
                    {idx + 1}
                  </span>
                  <span>{item.q}</span>
                </h3>
                <p className="text-xs text-slate-600 dark:text-slate-400 leading-relaxed pl-7">
                  {item.a}
                </p>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
