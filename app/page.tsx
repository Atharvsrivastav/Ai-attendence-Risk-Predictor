import Link from "next/link";
import {
  GraduationCap,
  Binary,
  BrainCircuit,
  Users,
  UserCheck,
  TrendingDown,
  ShieldCheck,
  Cpu,
  ArrowRight,
  Database,
  GitBranch,
  Layers,
} from "lucide-react";
import { AttendanceService } from "@/lib/services/attendanceService";
import RiskBadge from "@/components/RiskBadge";

export const dynamic = "force-dynamic";

export default async function HomePage() {
  let analytics: any = null;
  try {
    analytics = await AttendanceService.getClassAnalytics(75);
  } catch (err) {
    console.error("Home analytics fetch error:", err);
  }

  const dsaStructures = [
    {
      name: "Array / List",
      icon: Layers,
      complexity: "O(1) access, O(n) scan",
      role: "Sliding window of recent sessions, chronologically ordered records, and history slices.",
    },
    {
      name: "HashMap (O(1))",
      icon: Database,
      complexity: "O(1) avg lookup / insert",
      role: "Separate-chaining hash table indexing student records and subject-wise logs without DB scans.",
    },
    {
      name: "Set",
      icon: ShieldCheck,
      complexity: "O(1) add / has, O(n) intersection",
      role: "Enforcing distinct student IDs, unique absence patterns, and multi-subject absentee intersections.",
    },
    {
      name: "Queue (FIFO)",
      icon: GitBranch,
      complexity: "O(1) enqueue / dequeue",
      role: "Chronological attendance ingestion, risk alert processing, and sequential notification dispatch.",
    },
    {
      name: "Stack (LIFO)",
      icon: Cpu,
      complexity: "O(1) push / pop",
      role: "Audit trail and Undo mechanism for faculty roll-call mistakes and attendance corrections.",
    },
    {
      name: "Tree (Hierarchical)",
      icon: BrainCircuit,
      complexity: "O(h) traversal",
      role: "Rule-based decision tree for white-box, explainable attendance risk classification.",
    },
    {
      name: "Graph (Adjacency List)",
      icon: Binary,
      complexity: "O(V + E) BFS / DFS",
      role: "Modeling Faculty -> Subject -> Student relationships with layer-by-layer BFS and DFS pathfinding.",
    },
  ];

  return (
    <div className="space-y-12 py-4">
      {/* Hero Section */}
      <div className="relative rounded-3xl overflow-hidden p-8 sm:p-12 bg-gradient-to-br from-slate-900 via-indigo-950 to-slate-900 text-white shadow-2xl border border-indigo-900/50">
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top_right,_var(--tw-gradient-stops))] from-indigo-500/20 via-transparent to-transparent pointer-events-none"></div>

        <div className="relative max-w-3xl space-y-6">
          <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-indigo-500/20 text-indigo-300 text-xs font-semibold border border-indigo-500/30 backdrop-blur-md">
            <Cpu className="h-3.5 w-3.5" />
            DSA College Mini Project &bull; Semester 5/6 CSE
          </div>

          <h1 className="text-3xl sm:text-5xl font-extrabold tracking-tight leading-tight">
            AI Attendance Risk <br />
            <span className="bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-indigo-300 to-cyan-300">
              Prediction & Analytics
            </span>
          </h1>

          <p className="text-base sm:text-lg text-slate-300 leading-relaxed">
            A production-quality attendance intelligence platform built from the ground up to demonstrate{" "}
            <strong className="text-white font-semibold">Data Structures & Algorithms (Array, HashMap, Set, Queue, Stack, Tree, Graph)</strong>{" "}
            paired with <strong className="text-white font-semibold">Explainable AI & Machine Learning</strong>.
          </p>

          <div className="flex flex-wrap items-center gap-4 pt-2">
            <Link
              href="/faculty"
              className="inline-flex items-center gap-2 px-6 py-3 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white font-semibold text-sm shadow-lg shadow-indigo-600/30 transition-all hover:scale-[1.02]"
            >
              <Users className="h-4 w-4" />
              <span>Faculty Dashboard</span>
              <ArrowRight className="h-4 w-4" />
            </Link>

            <Link
              href="/student"
              className="inline-flex items-center gap-2 px-6 py-3 rounded-xl bg-slate-800 hover:bg-slate-700 text-slate-200 font-semibold text-sm border border-slate-700 transition-all hover:scale-[1.02]"
            >
              <UserCheck className="h-4 w-4" />
              <span>Student Portal</span>
            </Link>

            <Link
              href="/dsa-lab"
              className="inline-flex items-center gap-2 px-6 py-3 rounded-xl bg-gradient-to-r from-cyan-600 to-blue-600 hover:from-cyan-500 hover:to-blue-500 text-white font-semibold text-sm shadow-md transition-all hover:scale-[1.02]"
            >
              <Binary className="h-4 w-4" />
              <span>DSA Analysis Lab (Viva)</span>
            </Link>
          </div>
        </div>
      </div>

      {/* Live System Stats Bar */}
      {analytics && (
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm">
            <p className="text-xs text-slate-500 font-medium uppercase tracking-wider">Total Students</p>
            <p className="text-2xl sm:text-3xl font-bold mt-1 text-slate-900 dark:text-slate-100">
              {analytics.totalStudents}
            </p>
            <p className="text-[11px] text-slate-400 mt-1">Stored in StudentHashMap</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm">
            <p className="text-xs text-slate-500 font-medium uppercase tracking-wider">Class Average</p>
            <p className="text-2xl sm:text-3xl font-bold mt-1 text-indigo-600 dark:text-indigo-400">
              {analytics.averageAttendance}%
            </p>
            <p className="text-[11px] text-slate-400 mt-1">Calculated across 100 sessions</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm">
            <p className="text-xs text-slate-500 font-medium uppercase tracking-wider">Below Threshold (&lt;75%)</p>
            <p className="text-2xl sm:text-3xl font-bold mt-1 text-amber-600 dark:text-amber-400">
              {analytics.belowThresholdCount}
            </p>
            <p className="text-[11px] text-slate-400 mt-1">Identified via Linear & Binary Search</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm">
            <p className="text-xs text-slate-500 font-medium uppercase tracking-wider">Critical Risk</p>
            <p className="text-2xl sm:text-3xl font-bold mt-1 text-red-600 dark:text-red-400">
              {analytics.riskDistribution.critical}
            </p>
            <p className="text-[11px] text-slate-400 mt-1">Classified by DecisionTree ML</p>
          </div>
        </div>
      )}

      {/* DSA System Architecture Grid */}
      <div className="space-y-6">
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-2 border-b border-slate-200 dark:border-slate-800 pb-4">
          <div>
            <h2 className="text-xl sm:text-2xl font-bold text-slate-900 dark:text-slate-100 flex items-center gap-2">
              <Binary className="h-6 w-6 text-indigo-600" />
              Core Data Structures & Algorithms
            </h2>
            <p className="text-xs text-slate-500">
              Every data structure solves an explicit engineering problem in the monitoring pipeline.
            </p>
          </div>

          <Link
            href="/dsa-lab"
            className="text-xs font-semibold text-indigo-600 dark:text-indigo-400 hover:underline flex items-center gap-1"
          >
            <span>Open Interactive Viva Visualizer</span>
            <ArrowRight className="h-3.5 w-3.5" />
          </Link>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5">
          {dsaStructures.map((ds) => {
            const Icon = ds.icon;
            return (
              <div
                key={ds.name}
                className="bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm hover:border-indigo-300 dark:hover:border-indigo-800 transition-colors space-y-3"
              >
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2.5">
                    <div className="p-2 rounded-xl bg-indigo-50 dark:bg-indigo-950/60 text-indigo-600 dark:text-indigo-400">
                      <Icon className="h-5 w-5" />
                    </div>
                    <h3 className="font-bold text-sm text-slate-900 dark:text-slate-100">{ds.name}</h3>
                  </div>
                  <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-600 dark:text-slate-400">
                    {ds.complexity}
                  </span>
                </div>
                <p className="text-xs text-slate-600 dark:text-slate-400 leading-relaxed">
                  {ds.role}
                </p>
              </div>
            );
          })}
        </div>
      </div>

      {/* ML & Pipeline Architecture Card */}
      <div className="bg-gradient-to-r from-indigo-50 via-blue-50 to-slate-50 dark:from-slate-900 dark:via-indigo-950/40 dark:to-slate-900 rounded-2xl p-6 sm:p-8 border border-indigo-100 dark:border-slate-800">
        <h3 className="text-lg font-bold text-slate-900 dark:text-slate-100 flex items-center gap-2">
          <BrainCircuit className="h-5 w-5 text-indigo-600" />
          End-to-End Processing Architecture
        </h3>
        <p className="text-xs text-slate-600 dark:text-slate-400 mt-1 max-w-2xl">
          Clean separation of concerns: DSA handles high-throughput organization, lookups, and traversals; ML handles statistical risk prediction and white-box explainability.
        </p>

        <div className="mt-6 flex flex-wrap items-center justify-between gap-3 text-xs font-mono">
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700">
            1. Attendance Data
          </div>
          <ArrowRight className="h-4 w-4 text-slate-400" />
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700 text-indigo-600 font-semibold">
            2. Data Structures
          </div>
          <ArrowRight className="h-4 w-4 text-slate-400" />
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700 text-indigo-600 font-semibold">
            3. DSA Algorithms
          </div>
          <ArrowRight className="h-4 w-4 text-slate-400" />
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700 text-cyan-600 font-semibold">
            4. Feature Engineering
          </div>
          <ArrowRight className="h-4 w-4 text-slate-400" />
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700 text-cyan-600 font-semibold">
            5. ML Prediction
          </div>
          <ArrowRight className="h-4 w-4 text-slate-400" />
          <div className="px-3 py-2 rounded-lg bg-white dark:bg-slate-800 shadow-sm border border-slate-200 dark:border-slate-700 text-emerald-600 font-bold">
            6. Explainable Dashboard
          </div>
        </div>
      </div>
    </div>
  );
}
