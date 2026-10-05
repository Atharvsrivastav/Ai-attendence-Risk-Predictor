"use client";

import React, { useEffect, useState } from "react";
import {
  Shield,
  Users,
  BookOpen,
  UserPlus,
  PlusCircle,
  Database,
  CheckCircle2,
  AlertTriangle,
  RotateCcw,
} from "lucide-react";
import RiskBadge from "@/components/RiskBadge";

export default function AdminPage() {
  const [analytics, setAnalytics] = useState<any>(null);
  const [students, setStudents] = useState<any[]>([]);
  const [subjects, setSubjects] = useState<any[]>([]);
  const [loading, setLoading] = useState(true);

  // New Student modal / state
  const [newStudentName, setNewStudentName] = useState("");
  const [newStudentEmail, setNewStudentEmail] = useState("");
  const [newStudentRoll, setNewStudentRoll] = useState("");
  const [creatingStudent, setCreatingStudent] = useState(false);
  const [createMsg, setCreateMsg] = useState("");

  useEffect(() => {
    fetchAdminData();
  }, []);

  const fetchAdminData = async () => {
    try {
      setLoading(true);
      const [anaRes, stuRes, subRes] = await Promise.all([
        fetch("/api/analytics"),
        fetch("/api/students"),
        fetch("/api/subjects"),
      ]);

      if (anaRes.ok) setAnalytics(await anaRes.json());
      if (stuRes.ok) {
        const d = await stuRes.json();
        setStudents(d.students || []);
      }
      if (subRes.ok) {
        const d = await subRes.json();
        setSubjects(d.subjects || []);
      }
    } catch (err) {
      console.error("Admin data fetch error:", err);
    } finally {
      setLoading(false);
    }
  };

  const handleCreateStudent = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      setCreatingStudent(true);
      setCreateMsg("");
      const res = await fetch("/api/students", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          name: newStudentName,
          email: newStudentEmail,
          rollNumber: newStudentRoll,
        }),
      });

      const d = await res.json();
      if (res.ok) {
        setCreateMsg("✓ Student successfully enrolled and indexed in StudentHashMap!");
        setNewStudentName("");
        setNewStudentEmail("");
        setNewStudentRoll("");
        fetchAdminData();
      } else {
        setCreateMsg(`✗ Error: ${d.error}`);
      }
    } catch (err: any) {
      setCreateMsg(`✗ Error: ${err.message}`);
    } finally {
      setCreatingStudent(false);
    }
  };

  return (
    <div className="space-y-8">
      {/* Header */}
      <div className="border-b border-slate-200 dark:border-slate-800 pb-5">
        <div className="flex items-center gap-2.5">
          <div className="p-2 rounded-xl bg-slate-900 text-white shadow-md">
            <Shield className="h-6 w-6 text-indigo-400" />
          </div>
          <div>
            <h1 className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 dark:text-slate-100">
              Dean & Administrator Control Console
            </h1>
            <p className="text-xs text-slate-500">
              Department governance, student enrollment, faculty allocations, and university-wide analytics
            </p>
          </div>
        </div>
      </div>

      {/* Global Analytics Cards */}
      {analytics && (
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-semibold text-slate-500 uppercase">Enrolled Students</p>
            <p className="text-3xl font-bold text-slate-900 dark:text-slate-100">{analytics.totalStudents}</p>
            <p className="text-[10px] text-slate-400">Computer Science & Engineering</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-semibold text-slate-500 uppercase">Average Attendance</p>
            <p className="text-3xl font-bold text-indigo-600 dark:text-indigo-400">{analytics.averageAttendance}%</p>
            <p className="text-[10px] text-slate-400">Institutional baseline</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-semibold text-slate-500 uppercase">Defaulters (&lt;75%)</p>
            <p className="text-3xl font-bold text-amber-600 dark:text-amber-400">{analytics.belowThresholdCount}</p>
            <p className="text-[10px] text-slate-400">Exam debarment risk</p>
          </div>

          <div className="bg-white dark:bg-slate-900 p-5 rounded-2xl border border-slate-200 dark:border-slate-800 shadow-sm space-y-1">
            <p className="text-[11px] font-semibold text-slate-500 uppercase">Queue Alerts Processed</p>
            <p className="text-3xl font-bold text-emerald-600 dark:text-emerald-400">{analytics.recentAlerts?.length || 8}</p>
            <p className="text-[10px] text-slate-400">In AttendanceQueue (FIFO)</p>
          </div>
        </div>
      )}

      {/* Two-Column Administration Layout */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column: Quick Student Enrollment Form */}
        <div className="bg-white dark:bg-slate-900 rounded-2xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
          <div className="flex items-center gap-2">
            <UserPlus className="h-5 w-5 text-indigo-600" />
            <h2 className="font-bold text-base text-slate-900 dark:text-slate-100">
              Enroll New Student
            </h2>
          </div>
          <p className="text-xs text-slate-500">
            Automatically provisions student record, credentials, and auto-enrolls into core curriculum subjects.
          </p>

          {createMsg && (
            <div className={`p-3 rounded-xl text-xs font-semibold ${createMsg.startsWith("✓") ? "bg-emerald-50 text-emerald-800 dark:bg-emerald-950 dark:text-emerald-200" : "bg-red-50 text-red-800"}`}>
              {createMsg}
            </div>
          )}

          <form onSubmit={handleCreateStudent} className="space-y-3">
            <div className="space-y-1">
              <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">Full Name</label>
              <input
                type="text"
                placeholder="e.g. Varun Kapoor"
                value={newStudentName}
                onChange={(e) => setNewStudentName(e.target.value)}
                className="w-full text-xs px-3 py-2 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800"
                required
              />
            </div>

            <div className="space-y-1">
              <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">Roll Number</label>
              <input
                type="text"
                placeholder="e.g. 2024CS025"
                value={newStudentRoll}
                onChange={(e) => setNewStudentRoll(e.target.value)}
                className="w-full text-xs px-3 py-2 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800"
                required
              />
            </div>

            <div className="space-y-1">
              <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">Email Address</label>
              <input
                type="email"
                placeholder="e.g. varun.kapoor@college.edu"
                value={newStudentEmail}
                onChange={(e) => setNewStudentEmail(e.target.value)}
                className="w-full text-xs px-3 py-2 rounded-xl border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800"
                required
              />
            </div>

            <button
              type="submit"
              disabled={creatingStudent}
              className="w-full py-2.5 px-4 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white font-semibold text-xs shadow-md shadow-indigo-600/20 disabled:opacity-50 transition-colors"
            >
              {creatingStudent ? "Enrolling..." : "Enroll Student"}
            </button>
          </form>
        </div>

        {/* Right Column: Subjects & Faculty Allocations */}
        <div className="lg:col-span-2 bg-white dark:bg-slate-900 rounded-2xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <BookOpen className="h-5 w-5 text-indigo-600" />
              <h2 className="font-bold text-base text-slate-900 dark:text-slate-100">
                Department Subjects & Faculty Roster
              </h2>
            </div>
            <span className="text-xs font-mono text-slate-400">
              {subjects.length} Active Courses
            </span>
          </div>

          <div className="divide-y divide-slate-100 dark:divide-slate-800">
            {subjects.map((sub) => (
              <div key={sub.id} className="py-3 flex items-center justify-between gap-4 text-xs">
                <div>
                  <div className="flex items-center gap-2">
                    <span className="font-bold font-mono text-indigo-600 dark:text-indigo-400">
                      {sub.code}
                    </span>
                    <span className="font-semibold text-slate-900 dark:text-slate-100">
                      {sub.name}
                    </span>
                  </div>
                  <p className="text-[11px] text-slate-500 mt-0.5">
                    Faculty: <strong>{sub.facultyName}</strong> ({sub.facultyEmail}) &bull; {sub.credits} Credits
                  </p>
                </div>

                <div className="text-right font-mono">
                  <span className="px-2.5 py-1 rounded-lg bg-slate-100 dark:bg-slate-800 text-slate-700 dark:text-slate-300 font-semibold">
                    {sub.totalSessions} sessions held
                  </span>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
