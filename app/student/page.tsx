"use client";

import React, { useEffect, useState, Suspense } from "react";
import { useSearchParams } from "next/navigation";
import {
  GraduationCap,
  TrendingDown,
  TrendingUp,
  AlertTriangle,
  CheckCircle2,
  BrainCircuit,
  Sliders,
  Sparkles,
  BookOpen,
  ArrowRight,
  ShieldAlert,
  Info,
  Layers,
} from "lucide-react";
import RiskBadge from "@/components/RiskBadge";
import TrendIndicator from "@/components/TrendIndicator";

function StudentDashboardContent() {
  const searchParams = useSearchParams();
  const queryStudentId = searchParams.get("id");

  const [profile, setProfile] = useState<any>(null);
  const [loading, setLoading] = useState(true);
  const [threshold, setThreshold] = useState(75);

  // What-If Simulator state
  const [attendNext, setAttendNext] = useState(5);
  const [missNext, setMissNext] = useState(0);
  const [simResult, setSimResult] = useState<any>(null);

  const [allStudents, setAllStudents] = useState<any[]>([]);
  const [selectedStudentId, setSelectedStudentId] = useState<string>(queryStudentId || "");

  useEffect(() => {
    fetchAllStudents();
  }, []);

  useEffect(() => {
    fetchProfile(selectedStudentId);
  }, [selectedStudentId, threshold]);

  const fetchAllStudents = async () => {
    try {
      const res = await fetch("/api/students");
      if (res.ok) {
        const data = await res.json();
        const list = data.students || [];
        setAllStudents(list);
        if (!selectedStudentId && list.length > 0) {
          // Default to Rahul Verma (High Risk) for vivid demonstration
          const rahul = list.find((s: any) => s.rollNumber === "2024CS003");
          setSelectedStudentId(rahul ? rahul.studentId : list[0].studentId);
        }
      }
    } catch (err) {
      console.error("Fetch all students error:", err);
    }
  };

  const fetchProfile = async (targetId: string) => {
    if (!targetId) return;
    try {
      setLoading(true);
      const res = await fetch(`/api/students/${targetId}?threshold=${threshold}`);
      if (res.ok) {
        const data = await res.json();
        setProfile(data.profile || null);
      }
    } catch (err) {
      console.error("Student profile fetch error:", err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    if (profile) {
      runSimulation();
    }
  }, [attendNext, missNext, profile, threshold]);

  const runSimulation = async () => {
    if (!profile) return;
    try {
      const res = await fetch("/api/prediction", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          studentId: profile.studentId,
          attendNext,
          missNext,
          threshold,
        }),
      });

      if (res.ok) {
        const data = await res.json();
        setSimResult(data);
      }
    } catch (err) {
      console.error("Simulation error:", err);
    }
  };

  if (loading && !profile) {
    return (
      <div className="py-20 text-center space-y-3">
        <div className="inline-block h-8 w-8 animate-spin rounded-full border-4 border-indigo-600 border-r-transparent"></div>
        <p className="text-xs text-slate-500 font-mono">Running DSA algorithms & ML risk pipeline...</p>
      </div>
    );
  }

  if (!profile) {
    return (
      <div className="py-20 text-center space-y-4 max-w-md mx-auto">
        <div className="p-4 rounded-2xl bg-amber-50 dark:bg-amber-950/60 border border-amber-200 dark:border-amber-800 text-amber-800 dark:text-amber-200 text-xs">
          No student profile found. Please login as a student or select a student from the Faculty Dashboard.
        </div>
      </div>
    );
  }

  const isBelowThreshold = profile.overallPercentage < threshold;

  return (
    <div className="space-y-6">
      {/* Student Persona Quick Switcher for Viva Demo */}
      {allStudents.length > 0 && (
        <div className="p-3 rounded-2xl bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 shadow-sm flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          <div className="flex items-center gap-2">
            <span className="text-xs font-bold text-slate-700 dark:text-slate-300">
              Select Student Persona:
            </span>
            <span className="text-[10px] text-slate-400 font-mono">
              (Live profile re-evaluation)
            </span>
          </div>

          <div className="flex flex-wrap items-center gap-2">
            {allStudents.slice(0, 5).map((stu) => {
              const isSelected = selectedStudentId === stu.studentId;
              return (
                <button
                  key={stu.studentId}
                  onClick={() => setSelectedStudentId(stu.studentId)}
                  className={`px-3 py-1.5 rounded-xl text-xs font-semibold transition-all flex items-center gap-1.5 ${
                    isSelected
                      ? "bg-indigo-600 text-white shadow-md shadow-indigo-600/20"
                      : "bg-slate-100 dark:bg-slate-800 text-slate-700 dark:text-slate-300 hover:bg-slate-200"
                  }`}
                >
                  <span>{stu.name}</span>
                  <span className="opacity-75 font-mono text-[10px]">({stu.overallPercentage}%)</span>
                </button>
              );
            })}
          </div>
        </div>
      )}

      {/* Student Welcome Header */}
      <div className="bg-gradient-to-r from-slate-900 via-indigo-950 to-slate-900 rounded-3xl p-6 sm:p-8 text-white border border-indigo-900/50 shadow-xl flex flex-col sm:flex-row sm:items-center justify-between gap-6">
        <div className="space-y-2">

          <div className="flex items-center gap-2">
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono font-semibold bg-indigo-500/20 text-indigo-300 border border-indigo-500/30">
              {profile.rollNumber}
            </span>
            <span className="text-xs text-slate-400">&bull;</span>
            <span className="text-xs text-slate-300">{profile.department} (Sem {profile.semester})</span>
          </div>

          <h1 className="text-2xl sm:text-3xl font-bold tracking-tight">
            {profile.name}
          </h1>
          <p className="text-xs text-slate-400">
            Attendance Health & Predictive Risk Evaluation Portal
          </p>
        </div>

        {/* Big Overall Attendance Ring Badge */}
        <div className="flex items-center gap-5 bg-white/5 backdrop-blur-md p-4 rounded-2xl border border-white/10 self-start sm:self-center">
          <div className="text-center">
            <p className="text-[10px] text-slate-400 uppercase font-semibold">Overall Attendance</p>
            <p
              className={`text-3xl sm:text-4xl font-extrabold font-mono mt-0.5 ${
                isBelowThreshold ? "text-amber-400" : "text-emerald-400"
              }`}
            >
              {profile.overallPercentage}%
            </p>
          </div>
          <div className="h-10 w-[1px] bg-white/10"></div>
          <div className="space-y-1">
            <RiskBadge category={profile.riskCategory} score={profile.riskScore} size="md" />
            <p className="text-[10px] text-slate-400">
              Target Threshold: <strong className="text-white font-mono">{threshold}%</strong>
            </p>
          </div>
        </div>
      </div>

      {/* Primary KPI Metrics Grid */}
      <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
        {/* Classes Attended */}
        <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
          <p className="text-[11px] font-semibold text-slate-500 uppercase tracking-wider">Attendance Ratio</p>
          <p className="text-xl sm:text-2xl font-bold font-mono text-slate-900 dark:text-slate-100">
            {profile.totalSessionsAttended} / {profile.totalSessionsConducted}
          </p>
          <p className="text-[10px] text-slate-400">
            {profile.totalSessionsAbsent} missed lectures
          </p>
        </div>

        {/* Attendance Trend */}
        <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
          <p className="text-[11px] font-semibold text-slate-500 uppercase tracking-wider">Trend Momentum</p>
          <div className="pt-1">
            <TrendIndicator trend={profile.trend} slope={profile.trendSlope} showSlope />
          </div>
          <p className="text-[10px] text-slate-400">
            Recent 5 classes: <strong className="font-mono">{profile.recentWindowPercentage}%</strong>
          </p>
        </div>

        {/* Consecutive Absences */}
        <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
          <p className="text-[11px] font-semibold text-slate-500 uppercase tracking-wider">Active Absence Streak</p>
          <p
            className={`text-xl sm:text-2xl font-bold font-mono ${
              profile.currentConsecutiveAbsences >= 2 ? "text-rose-600" : "text-slate-800 dark:text-slate-200"
            }`}
          >
            {profile.currentConsecutiveAbsences} class{profile.currentConsecutiveAbsences === 1 ? "" : "es"}
          </p>
          <p className="text-[10px] text-slate-400">
            Max history streak: {profile.maxConsecutiveAbsences}
          </p>
        </div>

        {/* Recovery / Buffer Status */}
        <div className="bg-white dark:bg-slate-900 p-4 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
          <p className="text-[11px] font-semibold text-slate-500 uppercase tracking-wider">
            {isBelowThreshold ? "Classes Needed for 75%" : "Missable Buffer"}
          </p>
          <p
            className={`text-xl sm:text-2xl font-bold font-mono ${
              isBelowThreshold ? "text-amber-600" : "text-emerald-600"
            }`}
          >
            {isBelowThreshold ? profile.classesNeededToRecover : profile.classesCanBeMissed} class
            {(isBelowThreshold ? profile.classesNeededToRecover : profile.classesCanBeMissed) === 1 ? "" : "es"}
          </p>
          <p className="text-[10px] text-slate-400">
            {isBelowThreshold ? "Consecutive attendance required" : "Without dropping below threshold"}
          </p>
        </div>
      </div>

      {/* Explainable AI (XAI) Risk Breakdown Box */}
      <div className="rounded-2xl p-6 bg-gradient-to-br from-indigo-50/70 via-white to-blue-50/70 dark:from-slate-900 dark:via-slate-900 dark:to-indigo-950/40 border border-indigo-200/80 dark:border-slate-800 shadow-sm space-y-4">
        <div className="flex items-center justify-between flex-wrap gap-2">
          <div className="flex items-center gap-2.5">
            <div className="p-2 rounded-xl bg-indigo-600 text-white shadow-md shadow-indigo-600/20">
              <BrainCircuit className="h-5 w-5" />
            </div>
            <div>
              <h2 className="font-bold text-sm sm:text-base text-slate-900 dark:text-slate-100 flex items-center gap-2">
                Explainable AI: Why am I classified as {profile.riskCategory}?
              </h2>
              <p className="text-[11px] text-slate-500">
                Transparent factor attribution generated from Decision Tree rules & Logistic Regression weights
              </p>
            </div>
          </div>
          <span className="text-[10px] font-mono px-2.5 py-1 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950 dark:text-indigo-300 font-semibold">
            Ensemble-DT-LogReg
          </span>
        </div>

        {/* Contributing Factors Bullet List */}
        <div className="space-y-2 pt-1">
          <p className="text-xs font-semibold text-slate-700 dark:text-slate-300">
            Contributing Risk Factors:
          </p>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-2.5">
            {profile.contributingFactors.map((factor: string, idx: number) => (
              <div
                key={idx}
                className="p-3 rounded-xl bg-white dark:bg-slate-800/80 border border-slate-200 dark:border-slate-700/80 text-xs text-slate-700 dark:text-slate-300 flex items-start gap-2.5"
              >
                <div className="h-2 w-2 rounded-full bg-indigo-600 mt-1.5 shrink-0"></div>
                <span className="leading-relaxed">{factor}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Actionable Recommendation */}
        <div className="p-4 rounded-xl bg-amber-50 dark:bg-amber-950/40 border border-amber-200 dark:border-amber-800/70 text-xs text-amber-900 dark:text-amber-200 flex items-start gap-3">
          <Sparkles className="h-4 w-4 text-amber-600 shrink-0 mt-0.5" />
          <div>
            <span className="font-bold">System Recommendation: </span>
            <span>{profile.recommendation}</span>
          </div>
        </div>
      </div>

      {/* Interactive Recovery Simulator (Requirement 17) */}
      <div className="bg-white dark:bg-slate-900 rounded-2xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-6">
        <div className="flex items-center justify-between flex-wrap gap-2">
          <div className="flex items-center gap-2.5">
            <div className="p-2 rounded-xl bg-emerald-50 dark:bg-emerald-950 text-emerald-600 dark:text-emerald-400">
              <Sliders className="h-5 w-5" />
            </div>
            <div>
              <h2 className="font-bold text-sm sm:text-base text-slate-900 dark:text-slate-100">
                Interactive Attendance Recovery Simulator
              </h2>
              <p className="text-[11px] text-slate-500">
                Simulate future class attendance to see how your eligibility percentage and risk tier change in real time.
              </p>
            </div>
          </div>
          <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-emerald-100 text-emerald-700 dark:bg-emerald-950 dark:text-emerald-300 font-bold">
            What-If Projection
          </span>
        </div>

        {/* Sliders */}
        <div className="grid grid-cols-1 sm:grid-cols-2 gap-6 p-4 rounded-xl bg-slate-50 dark:bg-slate-800/50 border border-slate-200 dark:border-slate-800">
          <div className="space-y-2">
            <div className="flex items-center justify-between text-xs">
              <span className="font-semibold text-slate-700 dark:text-slate-300">
                Attend next consecutive classes:
              </span>
              <span className="font-mono font-bold text-emerald-600 px-2 py-0.5 rounded bg-emerald-50 dark:bg-emerald-950">
                +{attendNext} classes
              </span>
            </div>
            <input
              type="range"
              min="0"
              max="20"
              value={attendNext}
              onChange={(e) => setAttendNext(Number(e.target.value))}
              className="w-full accent-emerald-600 cursor-pointer"
            />
          </div>

          <div className="space-y-2">
            <div className="flex items-center justify-between text-xs">
              <span className="font-semibold text-slate-700 dark:text-slate-300">
                Miss next classes:
              </span>
              <span className="font-mono font-bold text-rose-600 px-2 py-0.5 rounded bg-rose-50 dark:bg-rose-950">
                +{missNext} classes
              </span>
            </div>
            <input
              type="range"
              min="0"
              max="10"
              value={missNext}
              onChange={(e) => setMissNext(Number(e.target.value))}
              className="w-full accent-rose-600 cursor-pointer"
            />
          </div>
        </div>

        {/* Simulation Output Card */}
        {simResult && (
          <div className="p-4 rounded-xl border border-indigo-200 dark:border-indigo-900 bg-indigo-50/40 dark:bg-indigo-950/20 flex flex-col sm:flex-row sm:items-center justify-between gap-4">
            <div className="space-y-1">
              <div className="flex items-center gap-2">
                <span className="text-xs font-semibold text-slate-600 dark:text-slate-300">
                  Current: <strong className="font-mono">{profile.overallPercentage}%</strong>
                </span>
                <ArrowRight className="h-3.5 w-3.5 text-slate-400" />
                <span className="text-xs font-semibold text-slate-900 dark:text-slate-100">
                  Projected:{" "}
                  <strong className="text-base font-mono font-bold text-indigo-600 dark:text-indigo-400">
                    {simResult.simulation.projectedPercentage}%
                  </strong>{" "}
                  ({simResult.simulation.deltaPercentage >= 0 ? "+" : ""}
                  {simResult.simulation.deltaPercentage}%)
                </span>
              </div>
              <p className="text-[11px] text-slate-500">
                {simResult.simulation.meetsThreshold
                  ? "✓ Satisfies the 75% university eligibility requirement!"
                  : `✗ Still below the ${threshold}% requirement. Attend ${profile.classesNeededToRecover} classes.`}
              </p>
            </div>

            <div className="flex items-center gap-3">
              <span className="text-xs font-semibold text-slate-500">Projected Risk:</span>
              <RiskBadge
                category={simResult.projectedRisk.category}
                score={simResult.projectedRisk.score}
              />
            </div>
          </div>
        )}
      </div>

      {/* Subject-Wise Attendance Breakdown */}
      <div className="space-y-4">
        <h2 className="text-base sm:text-lg font-bold text-slate-900 dark:text-slate-100 flex items-center gap-2">
          <BookOpen className="h-5 w-5 text-indigo-600" />
          Subject-Wise Detailed Records
        </h2>

        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
          {Object.values(profile.subjectWise || {}).map((sub: any) => {
            const isSubBelow = sub.percentage < threshold;

            return (
              <div
                key={sub.subjectId}
                className="bg-white dark:bg-slate-900 rounded-2xl p-5 border border-slate-200 dark:border-slate-800 shadow-sm space-y-3"
              >
                <div className="flex items-center justify-between">
                  <span className="font-mono text-xs font-bold text-indigo-600 dark:text-indigo-400 px-2 py-0.5 rounded bg-indigo-50 dark:bg-indigo-950">
                    {sub.subjectCode}
                  </span>
                  <span
                    className={`font-mono font-bold text-sm ${
                      isSubBelow ? "text-amber-600" : "text-emerald-600"
                    }`}
                  >
                    {sub.percentage}%
                  </span>
                </div>

                <div>
                  <h3 className="font-bold text-sm text-slate-900 dark:text-slate-100">{sub.subjectName}</h3>
                  <p className="text-[11px] text-slate-500">
                    {sub.presentCount} attended / {sub.totalConducted} conducted ({sub.absentCount} missed)
                  </p>
                </div>

                <div className="w-full bg-slate-200 dark:bg-slate-700 h-2 rounded-full overflow-hidden">
                  <div
                    className={`h-full ${isSubBelow ? "bg-amber-500" : "bg-emerald-500"}`}
                    style={{ width: `${Math.min(sub.percentage, 100)}%` }}
                  ></div>
                </div>

                <div className="pt-2 border-t border-slate-100 dark:border-slate-800 flex items-center justify-between text-xs">
                  <TrendIndicator trend={sub.trend} />
                  <span className="font-mono text-[11px] text-slate-500">
                    {isSubBelow
                      ? `Need ${sub.classesNeededToRecover} to reach ${threshold}%`
                      : `Can miss ${sub.classesCanBeMissed}`}
                  </span>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}

export default function StudentPage() {
  return (
    <Suspense fallback={<div className="py-20 text-center text-xs text-slate-400">Loading student portal...</div>}>
      <StudentDashboardContent />
    </Suspense>
  );
}
