"use client";

import React, { useEffect, useState } from "react";
import {
  Users,
  AlertTriangle,
  ShieldAlert,
  CheckCircle2,
  TrendingDown,
  RotateCcw,
  Search,
  Filter,
  ArrowUpDown,
  Download,
  PlusCircle,
  Cpu,
  Clock,
  ChevronRight,
  Sliders,
  Sparkles,
  Info,
} from "lucide-react";
import RiskBadge from "@/components/RiskBadge";
import TrendIndicator from "@/components/TrendIndicator";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  BarChart,
  Bar,
  PieChart,
  Pie,
  Cell,
  Legend,
} from "recharts";

export default function FacultyDashboard() {
  const [threshold, setThreshold] = useState<number>(75);
  const [students, setStudents] = useState<any[]>([]);
  const [analytics, setAnalytics] = useState<any>(null);
  const [subjects, setSubjects] = useState<any[]>([]);
  const [loading, setLoading] = useState<boolean>(true);
  const [mounted, setMounted] = useState<boolean>(false);
  const [dsaMetrics, setDsaMetrics] = useState<any>(null);

  // Table controls
  const [search, setSearch] = useState("");
  const [sortBy, setSortBy] = useState("attendance");
  const [sortOrder, setSortOrder] = useState<"asc" | "desc">("asc");
  const [sortAlgorithm, setSortAlgorithm] = useState<"mergeSort" | "quickSort">("mergeSort");
  const [filterRisk, setFilterRisk] = useState<string>("");
  const [belowThresholdOnly, setBelowThresholdOnly] = useState<boolean>(false);

  // Undo Stack notification banner
  const [undoMessage, setUndoMessage] = useState<string | null>(null);
  const [undoStackDepth, setUndoStackDepth] = useState<number>(0);

  useEffect(() => {
    setMounted(true);
  }, []);

  // Mark Attendance Modal
  const [markModalOpen, setMarkModalOpen] = useState(false);
  const [selectedSubjectId, setSelectedSubjectId] = useState("");
  const [sessionTopic, setSessionTopic] = useState("");
  const [attendanceInputs, setAttendanceInputs] = useState<Record<string, string>>({});
  const [submittingAttendance, setSubmittingAttendance] = useState(false);

  useEffect(() => {
    fetchData();
  }, [threshold, sortBy, sortOrder, sortAlgorithm, filterRisk, belowThresholdOnly]);

  const fetchData = async () => {
    try {
      setLoading(true);
      const query = new URLSearchParams({
        threshold: threshold.toString(),
        sortBy,
        sortOrder,
        sortAlgorithm,
        ...(filterRisk ? { filterRisk } : {}),
        ...(belowThresholdOnly ? { belowThreshold: "true" } : {}),
        ...(search ? { search } : {}),
      });

      const [stuRes, anaRes, subRes] = await Promise.all([
        fetch(`/api/students?${query.toString()}`),
        fetch(`/api/analytics?threshold=${threshold}`),
        fetch("/api/subjects"),
      ]);

      if (stuRes.ok) {
        const stuData = await stuRes.json();
        setStudents(stuData.students || []);
        setDsaMetrics(stuData.dsaMetrics || null);
      }

      if (anaRes.ok) {
        const anaData = await anaRes.json();
        setAnalytics(anaData);
        setUndoStackDepth(anaData.undoStackDepth || 0);
      }

      if (subRes.ok) {
        const subData = await subRes.json();
        setSubjects(subData.subjects || []);
        if (subData.subjects?.length > 0 && !selectedSubjectId) {
          setSelectedSubjectId(subData.subjects[0].id);
        }
      }
    } catch (err) {
      console.error("Faculty dashboard fetch error:", err);
    } finally {
      setLoading(false);
    }
  };

  const handleSearchSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    fetchData();
  };

  // Undo Last Action via AttendanceStack (LIFO)
  const handleUndo = async () => {
    try {
      const res = await fetch("/api/attendance/undo", { method: "POST" });
      const data = await res.json();
      if (data.success) {
        setUndoMessage(`✓ AttendanceStack Undo: ${data.message}`);
        setUndoStackDepth(data.remainingStackDepth);
        setTimeout(() => setUndoMessage(null), 6000);
        fetchData();
      } else {
        setUndoMessage(data.message || "No operations in Undo Stack.");
        setTimeout(() => setUndoMessage(null), 4000);
      }
    } catch (err) {
      console.error("Undo error:", err);
    }
  };

  // Mark Session Attendance Modal Submit
  const handleMarkAttendanceSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!selectedSubjectId) return;

    try {
      setSubmittingAttendance(true);
      const records = students.map((s) => ({
        studentId: s.studentId,
        status: attendanceInputs[s.studentId] || "PRESENT",
      }));

      const res = await fetch("/api/attendance", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          subjectId: selectedSubjectId,
          topic: sessionTopic || "Lecture Session",
          records,
        }),
      });

      if (res.ok) {
        setMarkModalOpen(false);
        setSessionTopic("");
        setUndoMessage("✓ Attendance recorded! Pushed to AttendanceQueue (FIFO) & AttendanceStack (LIFO).");
        setTimeout(() => setUndoMessage(null), 5000);
        fetchData();
      }
    } catch (err) {
      console.error("Failed to mark attendance:", err);
    } finally {
      setSubmittingAttendance(false);
    }
  };

  // CSV Export
  const exportCSV = () => {
    const headers = [
      "Roll Number",
      "Name",
      "Attendance %",
      "Conducted",
      "Attended",
      "Absences",
      "Consecutive Absences",
      "Trend",
      "Risk Category",
      "Risk Score",
      "Classes Needed for 75%",
    ];

    const rows = students.map((s) => [
      s.rollNumber,
      `"${s.name}"`,
      s.overallPercentage,
      s.totalSessionsConducted,
      s.totalSessionsAttended,
      s.totalSessionsAbsent,
      s.currentConsecutiveAbsences,
      s.trend,
      s.riskCategory,
      s.riskScore,
      s.classesNeededToRecover,
    ]);

    const csvContent = "data:text/csv;charset=utf-8," + [headers.join(","), ...rows.map((r) => r.join(","))].join("\n");
    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", `Attendance_Risk_Report_${new Date().toISOString().split("T")[0]}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  // Chronological session timeline computed from database AttendanceSession records
  const trendChartData =
    analytics?.sessionTimeline && analytics.sessionTimeline.length > 0
      ? analytics.sessionTimeline
      : [
          { session: "Sess 1-4", avg: 88, atRisk: 2 },
          { session: "Sess 5-8", avg: 85, atRisk: 3 },
          { session: "Sess 9-12", avg: 81, atRisk: 5 },
          { session: "Sess 13-16", avg: 76, atRisk: 7 },
          { session: "Sess 17-20", avg: 74, atRisk: 8 },
        ];

  const pieData = analytics
    ? [
        { name: "Low Risk", value: analytics.riskDistribution.low, color: "#10b981" },
        { name: "Medium Risk", value: analytics.riskDistribution.medium, color: "#f59e0b" },
        { name: "High Risk", value: analytics.riskDistribution.high, color: "#f97316" },
        { name: "Critical Risk", value: analytics.riskDistribution.critical, color: "#ef4444" },
      ]
    : [];

  return (
    <div className="space-y-8">
      {/* Header with Title and Actions */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 border-b border-slate-200 dark:border-slate-800 pb-5">
        <div>
          <div className="flex items-center gap-2">
            <h1 className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 dark:text-slate-100">
              Faculty Intelligence Dashboard
            </h1>
            <span className="text-[11px] font-semibold px-2.5 py-0.5 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950 dark:text-indigo-300 border border-indigo-200 dark:border-indigo-800">
              CS Department
            </span>
          </div>
          <p className="text-xs text-slate-500 mt-1">
            Real-time attendance calculations, DSA sorting & search benchmarks, and ML risk classification.
          </p>
        </div>

        <div className="flex flex-wrap items-center gap-2.5">
          {/* AttendanceStack Undo Button */}
          <button
            onClick={handleUndo}
            className="inline-flex items-center gap-1.5 px-3 py-2 text-xs font-semibold rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 hover:bg-slate-50 dark:hover:bg-slate-700/80 text-slate-700 dark:text-slate-200 shadow-sm transition-all"
            title="Revert the most recent attendance edit using the LIFO AttendanceStack"
          >
            <RotateCcw className="h-3.5 w-3.5 text-indigo-600" />
            <span>Undo Last Action</span>
            {undoStackDepth > 0 && (
              <span className="h-4 w-4 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950 text-[10px] flex items-center justify-center font-mono">
                {undoStackDepth}
              </span>
            )}
          </button>

          {/* Mark Attendance Modal Trigger */}
          <button
            onClick={() => {
              // Pre-fill inputs with PRESENT
              const initial: Record<string, string> = {};
              students.forEach((s) => (initial[s.studentId] = "PRESENT"));
              setAttendanceInputs(initial);
              setMarkModalOpen(true);
            }}
            className="inline-flex items-center gap-1.5 px-3.5 py-2 text-xs font-semibold rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white shadow-md shadow-indigo-600/20 transition-all"
          >
            <PlusCircle className="h-4 w-4" />
            <span>Mark Attendance</span>
          </button>

          {/* CSV Export */}
          <button
            onClick={exportCSV}
            className="inline-flex items-center gap-1.5 px-3 py-2 text-xs font-semibold rounded-xl border border-slate-300 dark:border-slate-700 bg-white dark:bg-slate-800 hover:bg-slate-50 text-slate-700 dark:text-slate-200 shadow-sm transition-all"
          >
            <Download className="h-3.5 w-3.5 text-slate-500" />
            <span>Export CSV</span>
          </button>
        </div>
      </div>

      {/* Undo Message Toast Banner */}
      {undoMessage && (
        <div className="p-3.5 rounded-xl bg-indigo-50 dark:bg-indigo-950/60 border border-indigo-200 dark:border-indigo-800 text-xs font-medium text-indigo-800 dark:text-indigo-200 flex items-center justify-between transition-all">
          <div className="flex items-center gap-2">
            <RotateCcw className="h-4 w-4 text-indigo-600 shrink-0" />
            <span>{undoMessage}</span>
          </div>
          <span className="text-[10px] font-mono opacity-70">Stack Op (LIFO)</span>
        </div>
      )}

      {/* Configurable Threshold Bar (Requirement 1 & 7) */}
      <div className="bg-white dark:bg-slate-900 rounded-2xl p-4 border border-slate-200 dark:border-slate-800 shadow-sm flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div className="flex items-center gap-3">
          <div className="p-2 rounded-xl bg-indigo-50 dark:bg-indigo-950 text-indigo-600 dark:text-indigo-400">
            <Sliders className="h-4 w-4" />
          </div>
          <div>
            <span className="text-xs font-bold text-slate-800 dark:text-slate-200">
              Attendance Eligibility Threshold
            </span>
            <p className="text-[11px] text-slate-500">
              Default is 75%. Adjusting instantly re-evaluates all recovery equations, feature vectors, and risk bounds.
            </p>
          </div>
        </div>

        <div className="flex items-center gap-3">
          <input
            type="range"
            min="60"
            max="90"
            step="1"
            value={threshold}
            onChange={(e) => setThreshold(Number(e.target.value))}
            className="w-32 accent-indigo-600 cursor-pointer"
          />
          <span className="font-mono text-sm font-bold px-3 py-1 rounded-lg bg-indigo-50 dark:bg-indigo-950 text-indigo-700 dark:text-indigo-300 border border-indigo-200 dark:border-indigo-800">
            {threshold}%
          </span>
        </div>
      </div>

      {/* KPI Stats Cards */}
      {analytics && (
        <div className="grid grid-cols-2 md:grid-cols-5 gap-4">
          <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-medium text-slate-500 uppercase tracking-wider">Total Students</p>
            <p className="text-2xl font-bold text-slate-900 dark:text-slate-100">{analytics.totalStudents}</p>
            <p className="text-[10px] text-slate-400">Indexed in StudentHashMap</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-medium text-slate-500 uppercase tracking-wider">Class Average</p>
            <p className="text-2xl font-bold text-indigo-600 dark:text-indigo-400">{analytics.averageAttendance}%</p>
            <p className="text-[10px] text-slate-400">Actual session formula</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-medium text-slate-500 uppercase tracking-wider">Below {threshold}%</p>
            <p className="text-2xl font-bold text-amber-600 dark:text-amber-400">{analytics.belowThresholdCount}</p>
            <p className="text-[10px] text-slate-400">Eligibility at risk</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-medium text-slate-500 uppercase tracking-wider">High Risk</p>
            <p className="text-2xl font-bold text-orange-600 dark:text-orange-400">{analytics.riskDistribution.high}</p>
            <p className="text-[10px] text-slate-400">Declining & streak warnings</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-medium text-slate-500 uppercase tracking-wider">Critical Risk</p>
            <p className="text-2xl font-bold text-red-600 dark:text-red-400">{analytics.riskDistribution.critical}</p>
            <p className="text-[10px] text-slate-400">Severe absenteeism</p>
          </div>
        </div>
      )}

      {/* Charts Section: Attendance Trend & Risk Distribution */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Trend Line Chart */}
        <div className="lg:col-span-2 bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
          <div className="flex items-center justify-between">
            <div>
              <h3 className="font-bold text-sm text-slate-900 dark:text-slate-100">
                Attendance Progression & At-Risk Momentum
              </h3>
              <p className="text-[11px] text-slate-500">
                Ordinary Least Squares trend analysis across semester session intervals
              </p>
            </div>
            <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500">
              Chronological Array
            </span>
          </div>

          <div className="h-60 w-full flex items-center justify-center">
            {mounted ? (
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={trendChartData}>
                  <CartesianGrid strokeDasharray="3 3" opacity={0.2} />
                  <XAxis dataKey="session" tick={{ fontSize: 11 }} />
                  <YAxis domain={[50, 100]} tick={{ fontSize: 11 }} />
                  <Tooltip
                    contentStyle={{
                      backgroundColor: "rgba(15, 23, 42, 0.9)",
                      borderRadius: "8px",
                      color: "#fff",
                      fontSize: "12px",
                    }}
                  />
                  <Line type="monotone" dataKey="avg" name="Average %" stroke="#6366f1" strokeWidth={3} dot={{ r: 4 }} />
                  <Line type="monotone" dataKey="atRisk" name="At-Risk Students" stroke="#ef4444" strokeWidth={2} strokeDasharray="4 4" />
                </LineChart>
              </ResponsiveContainer>
            ) : (
              <div className="text-xs text-slate-400">Rendering trend chart...</div>
            )}
          </div>
        </div>

        {/* Risk Distribution Donut */}
        <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
          <div>
            <h3 className="font-bold text-sm text-slate-900 dark:text-slate-100">
              Risk Category Breakdown
            </h3>
            <p className="text-[11px] text-slate-500">
              Decision Tree + Logistic Regression classifications
            </p>
          </div>

          <div className="h-60 w-full flex items-center justify-center">
            {mounted && pieData.length > 0 ? (
              <ResponsiveContainer width="100%" height="100%">
                <PieChart>
                  <Pie
                    data={pieData}
                    cx="50%"
                    cy="50%"
                    innerRadius={50}
                    outerRadius={75}
                    paddingAngle={4}
                    dataKey="value"
                  >
                    {pieData.map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={entry.color} />
                    ))}
                  </Pie>
                  <Tooltip />
                  <Legend iconSize={8} wrapperStyle={{ fontSize: "11px" }} />
                </PieChart>
              </ResponsiveContainer>
            ) : (
              <div className="text-xs text-slate-400">Rendering distribution...</div>
            )}
          </div>
        </div>
      </div>

      {/* Subject-Wise Attendance Breakdown */}
      {analytics && analytics.subjectBreakdown && (
        <div className="bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm space-y-3">
          <div className="flex items-center justify-between">
            <h3 className="font-bold text-sm text-slate-900 dark:text-slate-100">
              Subject-Wise Attendance Comparison
            </h3>
            <span className="text-[11px] text-slate-400">
              Grouped via Subject HashMap
            </span>
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-5 gap-3">
            {analytics.subjectBreakdown.map((sub: any) => (
              <div
                key={sub.subjectId}
                className="p-3.5 rounded-xl border border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-800/50 space-y-1.5"
              >
                <div className="flex items-center justify-between">
                  <span className="font-bold text-xs font-mono text-indigo-600 dark:text-indigo-400">
                    {sub.subjectCode}
                  </span>
                  <span className="text-[10px] font-semibold text-slate-500">
                    {sub.averageAttendance}%
                  </span>
                </div>
                <p className="text-[11px] font-medium text-slate-800 dark:text-slate-200 truncate">
                  {sub.subjectName}
                </p>
                <p className="text-[10px] text-slate-400 truncate">{sub.faculty}</p>
                <div className="w-full bg-slate-200 dark:bg-slate-700 h-1.5 rounded-full overflow-hidden mt-1">
                  <div
                    className={`h-full ${
                      sub.averageAttendance >= threshold
                        ? "bg-emerald-500"
                        : "bg-amber-500"
                    }`}
                    style={{ width: `${Math.min(sub.averageAttendance, 100)}%` }}
                  ></div>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* DSA Live Telemetry & Algorithm Comparison Box */}
      {dsaMetrics && (
        <div className="rounded-2xl p-4 bg-slate-900 text-slate-100 border border-slate-800 shadow-sm space-y-3">
          <div className="flex items-center justify-between flex-wrap gap-2">
            <div className="flex items-center gap-2">
              <Cpu className="h-4 w-4 text-cyan-400" />
              <span className="text-xs font-bold text-white uppercase tracking-wider">
                DSA Live Telemetry & Complexity Verification
              </span>
            </div>
            <span className="text-[10px] font-mono text-slate-400">
              Verified in runtime memory
            </span>
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 text-xs font-mono">
            {/* Sorting Telemetry */}
            <div className="p-3 rounded-xl bg-slate-800/80 border border-slate-700 space-y-1">
              <p className="text-[10px] text-cyan-300 font-bold uppercase">
                Active Sort: {dsaMetrics.sort.algorithm}
              </p>
              <p className="text-slate-300">
                Time: <strong className="text-white">{dsaMetrics.sort.timeTakenMicroseconds} µs</strong>
              </p>
              <p className="text-slate-400">
                Comparisons: {dsaMetrics.sort.comparisons} | Swaps: {dsaMetrics.sort.swaps}
              </p>
              <p className="text-[10px] text-slate-400">
                Complexity: {dsaMetrics.sort.timeComplexity}
              </p>
            </div>

            {/* Searching Telemetry */}
            <div className="p-3 rounded-xl bg-slate-800/80 border border-slate-700 space-y-1">
              <p className="text-[10px] text-indigo-300 font-bold uppercase">
                Active Search: {dsaMetrics.search?.algorithm || "LinearSearch"}
              </p>
              <p className="text-slate-300">
                Time: <strong className="text-white">{dsaMetrics.search?.timeTakenMicroseconds || 5} µs</strong>
              </p>
              <p className="text-slate-400">
                Steps Taken: {dsaMetrics.search?.stepsCount || 1}
              </p>
              <p className="text-[10px] text-slate-400">
                Complexity: {dsaMetrics.search?.timeComplexity || "O(n)"}
              </p>
            </div>

            {/* HashMap Telemetry */}
            <div className="p-3 rounded-xl bg-slate-800/80 border border-slate-700 space-y-1">
              <p className="text-[10px] text-emerald-300 font-bold uppercase">
                StudentHashMap Index
              </p>
              <p className="text-slate-300">
                Size: <strong className="text-white">{dsaMetrics.hashMapStats.size}</strong> entries
              </p>
              <p className="text-slate-400">
                Load Factor: {dsaMetrics.hashMapStats.loadFactor} | Collisions: {dsaMetrics.hashMapStats.collisionCount}
              </p>
              <p className="text-[10px] text-slate-400">
                Lookup Complexity: O(1) average
              </p>
            </div>
          </div>
        </div>
      )}

      {/* Students Table with Search, Filter & DSA Sort */}
      <div className="bg-white dark:bg-slate-900 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm overflow-hidden">
        {/* Table Controls */}
        <div className="p-4 border-b border-slate-200 dark:border-slate-800 flex flex-wrap items-center justify-between gap-3">
          <form onSubmit={handleSearchSubmit} className="relative flex-1 min-w-[240px]">
            <Search className="absolute left-3 top-2.5 h-4 w-4 text-slate-400" />
            <input
              type="text"
              placeholder="Search student by name, roll number (Linear Search)..."
              value={search}
              onChange={(e) => setSearch(e.target.value)}
              className="w-full pl-9 pr-3 py-1.5 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-slate-100 focus:outline-none focus:ring-2 focus:ring-indigo-500"
            />
          </form>

          <div className="flex flex-wrap items-center gap-2">
            {/* Sort Field */}
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value)}
              className="text-xs px-2.5 py-1.5 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-700 dark:text-slate-200"
            >
              <option value="attendance">Sort: Attendance %</option>
              <option value="riskScore">Sort: Risk Score</option>
              <option value="absences">Sort: Total Absences</option>
              <option value="rollNumber">Sort: Roll Number</option>
            </select>

            {/* Sort Order */}
            <button
              onClick={() => setSortOrder(sortOrder === "asc" ? "desc" : "asc")}
              className="px-2.5 py-1.5 text-xs rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-700 dark:text-slate-200 flex items-center gap-1 font-mono"
              title="Toggle Ascending / Descending"
            >
              <ArrowUpDown className="h-3.5 w-3.5" />
              <span>{sortOrder.toUpperCase()}</span>
            </button>

            {/* DSA Algorithm Selector */}
            <select
              value={sortAlgorithm}
              onChange={(e) => setSortAlgorithm(e.target.value as any)}
              className="text-xs px-2.5 py-1.5 rounded-xl border border-indigo-300 dark:border-indigo-700 bg-indigo-50 dark:bg-indigo-950/60 text-indigo-700 dark:text-indigo-300 font-semibold"
            >
              <option value="mergeSort">Algorithm: MergeSort (O(n log n))</option>
              <option value="quickSort">Algorithm: QuickSort (O(n log n))</option>
            </select>

            {/* Filter by Risk Category */}
            <select
              value={filterRisk}
              onChange={(e) => setFilterRisk(e.target.value)}
              className="text-xs px-2.5 py-1.5 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-700 dark:text-slate-200"
            >
              <option value="">Filter: All Risks</option>
              <option value="LOW">Low Risk</option>
              <option value="MEDIUM">Medium Risk</option>
              <option value="HIGH">High Risk</option>
              <option value="CRITICAL">Critical Risk</option>
            </select>

            {/* Below Threshold Toggle */}
            <button
              onClick={() => setBelowThresholdOnly(!belowThresholdOnly)}
              className={`px-3 py-1.5 text-xs rounded-xl font-medium border transition-colors ${
                belowThresholdOnly
                  ? "bg-amber-600 text-white border-amber-600"
                  : "border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-700 dark:text-slate-300"
              }`}
            >
              &lt; {threshold}% Only
            </button>
          </div>
        </div>

        {/* Table Content */}
        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs">
            <thead className="bg-slate-50 dark:bg-slate-800/60 text-slate-500 uppercase tracking-wider font-semibold border-b border-slate-200 dark:border-slate-800">
              <tr>
                <th className="py-3 px-4">Student</th>
                <th className="py-3 px-4">Roll Number</th>
                <th className="py-3 px-4">Attendance %</th>
                <th className="py-3 px-4">Conducted / Attended</th>
                <th className="py-3 px-4">Absence Streak</th>
                <th className="py-3 px-4">Trend</th>
                <th className="py-3 px-4">Risk Category</th>
                <th className="py-3 px-4">Action</th>
              </tr>
            </thead>

            <tbody className="divide-y divide-slate-100 dark:divide-slate-800">
              {students.length === 0 ? (
                <tr>
                  <td colSpan={8} className="py-8 text-center text-slate-400">
                    No student records match the specified filters.
                  </td>
                </tr>
              ) : (
                students.map((student) => {
                  const isBelow = student.overallPercentage < threshold;

                  return (
                    <tr
                      key={student.studentId}
                      className="hover:bg-slate-50/80 dark:hover:bg-slate-800/40 transition-colors"
                    >
                      <td className="py-3 px-4">
                        <div>
                          <p className="font-semibold text-slate-900 dark:text-slate-100">{student.name}</p>
                          <p className="text-[10px] text-slate-400">{student.email}</p>
                        </div>
                      </td>

                      <td className="py-3 px-4 font-mono text-slate-600 dark:text-slate-400">
                        {student.rollNumber}
                      </td>

                      <td className="py-3 px-4">
                        <div className="flex items-center gap-2">
                          <span
                            className={`font-bold font-mono text-sm ${
                              isBelow
                                ? "text-amber-600 dark:text-amber-400"
                                : "text-emerald-600 dark:text-emerald-400"
                            }`}
                          >
                            {student.overallPercentage}%
                          </span>
                          {isBelow && (
                            <span className="text-[10px] font-semibold text-rose-500 bg-rose-50 dark:bg-rose-950/60 px-1.5 py-0.5 rounded">
                              Below {threshold}%
                            </span>
                          )}
                        </div>
                      </td>

                      <td className="py-3 px-4 font-mono text-slate-600 dark:text-slate-400">
                        {student.totalSessionsAttended} / {student.totalSessionsConducted} classes
                      </td>

                      <td className="py-3 px-4">
                        {student.currentConsecutiveAbsences > 0 ? (
                          <span
                            className={`inline-flex items-center gap-1 font-mono font-bold ${
                              student.currentConsecutiveAbsences >= 3
                                ? "text-red-600"
                                : "text-amber-600"
                            }`}
                          >
                            <AlertTriangle className="h-3 w-3" />
                            {student.currentConsecutiveAbsences} consecutive
                          </span>
                        ) : (
                          <span className="text-slate-400">0 streak</span>
                        )}
                      </td>

                      <td className="py-3 px-4">
                        <TrendIndicator trend={student.trend} slope={student.trendSlope} showSlope />
                      </td>

                      <td className="py-3 px-4">
                        <RiskBadge category={student.riskCategory} score={student.riskScore} />
                      </td>

                      <td className="py-3 px-4">
                        <a
                          href={`/student?id=${student.studentId}`}
                          className="inline-flex items-center gap-1 text-xs font-semibold text-indigo-600 hover:text-indigo-700 dark:text-indigo-400"
                        >
                          <span>Analyze</span>
                          <ChevronRight className="h-3.5 w-3.5" />
                        </a>
                      </td>
                    </tr>
                  );
                })
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Mark Attendance Modal */}
      {markModalOpen && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/60 backdrop-blur-sm">
          <div className="bg-white dark:bg-slate-900 rounded-2xl max-w-2xl w-full max-h-[85vh] flex flex-col shadow-2xl border border-slate-200 dark:border-slate-800 overflow-hidden">
            <div className="p-4 border-b border-slate-200 dark:border-slate-800 flex items-center justify-between">
              <div>
                <h3 className="font-bold text-base text-slate-900 dark:text-slate-100">
                  Mark Attendance Session
                </h3>
                <p className="text-[11px] text-slate-500">
                  Pushes event to AttendanceQueue (FIFO) and saves state to AttendanceStack (LIFO) for Undo.
                </p>
              </div>
              <button
                onClick={() => setMarkModalOpen(false)}
                className="text-slate-400 hover:text-slate-600 text-sm font-bold p-1"
              >
                ✕
              </button>
            </div>

            <form onSubmit={handleMarkAttendanceSubmit} className="flex-1 overflow-y-auto p-4 space-y-4">
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                <div className="space-y-1">
                  <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">
                    Subject
                  </label>
                  <select
                    value={selectedSubjectId}
                    onChange={(e) => setSelectedSubjectId(e.target.value)}
                    className="w-full text-xs px-3 py-2 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-slate-100"
                  >
                    {subjects.map((sub) => (
                      <option key={sub.id} value={sub.id}>
                        {sub.code}: {sub.name}
                      </option>
                    ))}
                  </select>
                </div>

                <div className="space-y-1">
                  <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">
                    Session Topic
                  </label>
                  <input
                    type="text"
                    placeholder="e.g. Graph Traversals & Applications"
                    value={sessionTopic}
                    onChange={(e) => setSessionTopic(e.target.value)}
                    className="w-full text-xs px-3 py-2 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-slate-100"
                  />
                </div>
              </div>

              {/* Student Roll Call Toggles */}
              <div className="space-y-2">
                <p className="text-xs font-bold text-slate-700 dark:text-slate-300">
                  Class Roll Call ({students.length} Students)
                </p>
                <div className="divide-y divide-slate-100 dark:divide-slate-800 max-h-60 overflow-y-auto border border-slate-200 dark:border-slate-800 rounded-xl p-2">
                  {students.map((stu) => {
                    const currentStatus = attendanceInputs[stu.studentId] || "PRESENT";

                    return (
                      <div
                        key={stu.studentId}
                        className="py-2 px-2 flex items-center justify-between text-xs"
                      >
                        <div>
                          <span className="font-semibold text-slate-900 dark:text-slate-100">
                            {stu.name}
                          </span>
                          <span className="ml-2 font-mono text-[10px] text-slate-400">
                            {stu.rollNumber}
                          </span>
                        </div>

                        <div className="flex items-center gap-1">
                          {(["PRESENT", "LATE", "ABSENT"] as const).map((stat) => (
                            <button
                              key={stat}
                              type="button"
                              onClick={() =>
                                setAttendanceInputs((prev) => ({
                                  ...prev,
                                  [stu.studentId]: stat,
                                }))
                              }
                              className={`px-2 py-0.5 rounded text-[10px] font-bold transition-colors ${
                                currentStatus === stat
                                  ? stat === "PRESENT"
                                    ? "bg-emerald-600 text-white"
                                    : stat === "LATE"
                                    ? "bg-yellow-500 text-white"
                                    : "bg-red-600 text-white"
                                  : "bg-slate-100 dark:bg-slate-800 text-slate-500 hover:bg-slate-200"
                              }`}
                            >
                              {stat}
                            </button>
                          ))}
                        </div>
                      </div>
                    );
                  })}
                </div>
              </div>

              <div className="pt-2 flex items-center justify-end gap-2 border-t border-slate-200 dark:border-slate-800">
                <button
                  type="button"
                  onClick={() => setMarkModalOpen(false)}
                  className="px-4 py-2 text-xs font-semibold rounded-xl border border-slate-300 dark:border-slate-700 text-slate-600 hover:bg-slate-100"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  disabled={submittingAttendance}
                  className="px-5 py-2 text-xs font-semibold rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white shadow-md disabled:opacity-50"
                >
                  {submittingAttendance ? "Saving..." : "Submit Attendance Session"}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}
