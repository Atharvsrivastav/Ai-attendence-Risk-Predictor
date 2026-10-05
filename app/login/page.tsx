"use client";

import React, { useState } from "react";
import { useRouter } from "next/navigation";
import { GraduationCap, Lock, Mail, ArrowRight, ShieldAlert, CheckCircle2 } from "lucide-react";

export default function LoginPage() {
  const router = useRouter();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  const handleLogin = async (loginEmail?: string, loginPassword?: string) => {
    const targetEmail = loginEmail || email;
    const targetPassword = loginPassword || password;

    if (!targetEmail || !targetPassword) {
      setError("Please provide both email and password.");
      return;
    }

    try {
      setLoading(true);
      setError("");

      const res = await fetch("/api/auth/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ email: targetEmail, password: targetPassword }),
      });

      const data = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Login failed");
      }

      if (data.user.role === "FACULTY") {
        router.push("/faculty");
      } else if (data.user.role === "STUDENT") {
        router.push("/student");
      } else if (data.user.role === "ADMIN") {
        router.push("/admin");
      } else {
        router.push("/");
      }
      router.refresh();
    } catch (err: any) {
      setError(err.message || "An unexpected error occurred");
    } finally {
      setLoading(false);
    }
  };

  const demoAccounts = [
    {
      role: "Faculty Member",
      name: "Prof. Rajesh Sharma",
      email: "prof.sharma@college.edu",
      pass: "faculty123",
      desc: "Full class access, sorting, attendance marking & undo stack",
      badge: "Faculty",
      color: "border-indigo-200 dark:border-indigo-800 bg-indigo-50/50 dark:bg-indigo-950/20",
    },
    {
      role: "High-Risk Student",
      name: "Rahul Verma (2024CS003)",
      email: "rahul.verma@college.edu",
      pass: "student123",
      desc: "64% attendance, 4 consecutive absences, declining trend",
      badge: "High Risk",
      color: "border-amber-200 dark:border-amber-800 bg-amber-50/50 dark:bg-amber-950/20",
    },
    {
      role: "Safe Student",
      name: "Aryan Sharma (2024CS001)",
      email: "aryan.sharma@college.edu",
      pass: "student123",
      desc: "94% attendance, safe buffer, low risk",
      badge: "Low Risk",
      color: "border-emerald-200 dark:border-emerald-800 bg-emerald-50/50 dark:bg-emerald-950/20",
    },
    {
      role: "Dean of Academics",
      name: "Dr. K. R. Raman",
      email: "admin@college.edu",
      pass: "admin123",
      desc: "System-wide analytics, department oversight",
      badge: "Admin",
      color: "border-slate-200 dark:border-slate-800 bg-slate-50 dark:bg-slate-900/60",
    },
  ];

  return (
    <div className="max-w-md mx-auto py-12 space-y-8">
      <div className="text-center space-y-2">
        <div className="inline-flex h-12 w-12 rounded-2xl bg-indigo-600 text-white items-center justify-center shadow-lg shadow-indigo-600/30 mb-2">
          <GraduationCap className="h-6 w-6" />
        </div>
        <h1 className="text-2xl font-bold text-slate-900 dark:text-slate-100">
          Sign In to AttendRisk AI
        </h1>
        <p className="text-xs text-slate-500">
          Enter your credentials or choose a quick persona to inspect
        </p>
      </div>

      {error && (
        <div className="p-3 rounded-xl bg-red-50 dark:bg-red-950/60 border border-red-200 dark:border-red-800 text-xs text-red-700 dark:text-red-300 flex items-center gap-2">
          <ShieldAlert className="h-4 w-4 shrink-0" />
          <span>{error}</span>
        </div>
      )}

      <form
        onSubmit={(e) => {
          e.preventDefault();
          handleLogin();
        }}
        className="bg-white dark:bg-slate-900 rounded-2xl p-6 border border-slate-200 dark:border-slate-800 shadow-sm space-y-4"
      >
        <div className="space-y-1">
          <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">
            Email Address
          </label>
          <div className="relative">
            <Mail className="absolute left-3 top-3 h-4 w-4 text-slate-400" />
            <input
              type="email"
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              placeholder="e.g. prof.sharma@college.edu"
              className="w-full pl-9 pr-3 py-2 rounded-xl text-xs border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-slate-100 focus:outline-none focus:ring-2 focus:ring-indigo-500"
              required
            />
          </div>
        </div>

        <div className="space-y-1">
          <label className="text-xs font-semibold text-slate-700 dark:text-slate-300">
            Password
          </label>
          <div className="relative">
            <Lock className="absolute left-3 top-3 h-4 w-4 text-slate-400" />
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="••••••••"
              className="w-full pl-9 pr-3 py-2 rounded-xl text-xs border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 text-slate-900 dark:text-slate-100 focus:outline-none focus:ring-2 focus:ring-indigo-500"
              required
            />
          </div>
        </div>

        <button
          type="submit"
          disabled={loading}
          className="w-full py-2.5 px-4 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white font-semibold text-xs transition-colors shadow-md shadow-indigo-600/20 disabled:opacity-50 flex items-center justify-center gap-2"
        >
          {loading ? "Authenticating..." : "Sign In"}
          <ArrowRight className="h-4 w-4" />
        </button>
      </form>

      {/* Viva / Demo Quick Login Shortcuts */}
      <div className="space-y-3">
        <p className="text-[11px] font-bold text-slate-400 uppercase tracking-wider text-center">
          Instant Viva Demo Sign-In
        </p>

        <div className="space-y-2">
          {demoAccounts.map((acc) => (
            <button
              key={acc.email}
              type="button"
              onClick={() => {
                setEmail(acc.email);
                setPassword(acc.pass);
                handleLogin(acc.email, acc.pass);
              }}
              className={`w-full text-left p-3 rounded-xl border ${acc.color} hover:scale-[1.01] transition-transform flex items-center justify-between gap-3`}
            >
              <div>
                <div className="flex items-center gap-2">
                  <span className="font-bold text-xs text-slate-900 dark:text-slate-100">
                    {acc.name}
                  </span>
                  <span className="text-[10px] font-semibold px-2 py-0.5 rounded-full bg-white/80 dark:bg-slate-800 text-slate-700 dark:text-slate-300 border border-slate-200 dark:border-slate-700">
                    {acc.badge}
                  </span>
                </div>
                <p className="text-[11px] text-slate-500 dark:text-slate-400 mt-0.5">{acc.desc}</p>
              </div>
              <ArrowRight className="h-4 w-4 text-slate-400 shrink-0" />
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}
