"use client";

import React, { useEffect, useState } from "react";
import Link from "next/link";
import { usePathname, useRouter } from "next/navigation";
import {
  GraduationCap,
  Users,
  UserCheck,
  Binary,
  Shield,
  LogOut,
  ChevronDown,
  Sparkles,
  Layers,
  Menu,
  X,
} from "lucide-react";

interface UserSession {
  id: string;
  name: string;
  email: string;
  role: "ADMIN" | "FACULTY" | "STUDENT";
  studentId?: string;
  facultyId?: string;
}

export default function Navbar() {
  const pathname = usePathname();
  const router = useRouter();
  const [currentUser, setCurrentUser] = useState<UserSession | null>(null);
  const [loading, setLoading] = useState(true);
  const [switchOpen, setSwitchOpen] = useState(false);
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  useEffect(() => {
    fetchCurrentUser();
    setMobileMenuOpen(false);
  }, [pathname]);

  const fetchCurrentUser = async () => {
    try {
      const res = await fetch("/api/auth/me");
      if (res.ok) {
        const data = await res.json();
        if (data.authenticated) {
          setCurrentUser(data.user);
        } else {
          setCurrentUser(null);
        }
      } else {
        setCurrentUser(null);
      }
    } catch (err) {
      setCurrentUser(null);
    } finally {
      setLoading(false);
    }
  };

  const handleQuickLogin = async (email: string, pass: string) => {
    try {
      setLoading(true);
      const res = await fetch("/api/auth/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ email, password: pass }),
      });
      if (res.ok) {
        const data = await res.json();
        setCurrentUser(data.user);
        setSwitchOpen(false);

        // Redirect appropriately
        if (data.user.role === "FACULTY") {
          router.push("/faculty");
        } else if (data.user.role === "STUDENT") {
          router.push("/student");
        } else if (data.user.role === "ADMIN") {
          router.push("/admin");
        }
        router.refresh();
      }
    } catch (err) {
      console.error("Quick login failed:", err);
    } finally {
      setLoading(false);
    }
  };

  const handleLogout = async () => {
    try {
      await fetch("/api/auth/logout", { method: "POST" });
      setCurrentUser(null);
      router.push("/login");
      router.refresh();
    } catch (err) {
      console.error("Logout failed:", err);
    }
  };

  const navLinks = [
    { href: "/faculty", label: "Faculty Dashboard", icon: Users, roleReq: ["FACULTY", "ADMIN"] },
    { href: "/student", label: "Student Portal", icon: UserCheck, roleReq: ["STUDENT", "FACULTY", "ADMIN"] },
    { href: "/dsa-lab", label: "DSA Analysis (Viva)", icon: Binary, highlight: true },
    { href: "/admin", label: "Admin Panel", icon: Shield, roleReq: ["ADMIN"] },
  ];

  return (
    <header className="sticky top-0 z-50 border-b border-slate-200 dark:border-slate-800 bg-white/90 dark:bg-slate-900/90 backdrop-blur-md">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Logo */}
          <Link href="/" className="flex items-center gap-3 group">
            <div className="h-10 w-10 rounded-xl bg-gradient-to-tr from-indigo-600 via-blue-600 to-cyan-500 flex items-center justify-center text-white shadow-md shadow-indigo-500/20 group-hover:scale-105 transition-transform">
              <GraduationCap className="h-5 w-5" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <span className="font-bold text-base tracking-tight text-slate-900 dark:text-white">
                  AttendRisk AI
                </span>
                <span className="text-[10px] uppercase font-bold tracking-wider px-2 py-0.5 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950/80 dark:text-indigo-300 border border-indigo-200 dark:border-indigo-800">
                  DSA + ML
                </span>
              </div>
              <p className="text-[11px] text-slate-500 hidden sm:block">
                Predictive Attendance & Risk Monitoring
              </p>
            </div>
          </Link>

          {/* Nav Links */}
          <nav className="hidden md:flex items-center gap-1.5">
            {navLinks.map((link) => {
              const Icon = link.icon;
              const isActive = pathname === link.href;

              return (
                <Link
                  key={link.href}
                  href={link.href}
                  className={`flex items-center gap-2 px-3 py-2 rounded-lg text-xs font-medium transition-all ${
                    isActive
                      ? "bg-indigo-50 dark:bg-indigo-950/60 text-indigo-700 dark:text-indigo-300 font-semibold"
                      : link.highlight
                      ? "text-indigo-600 dark:text-indigo-400 bg-indigo-50/50 hover:bg-indigo-100/70 dark:bg-indigo-950/30"
                      : "text-slate-600 dark:text-slate-300 hover:bg-slate-100 dark:hover:bg-slate-800"
                  }`}
                >
                  <Icon className={`h-4 w-4 ${link.highlight ? "text-indigo-600 dark:text-indigo-400" : ""}`} />
                  <span>{link.label}</span>
                  {link.highlight && (
                    <span className="flex h-1.5 w-1.5 rounded-full bg-indigo-600 animate-ping"></span>
                  )}
                </Link>
              );
            })}
          </nav>

          {/* User & Role Switcher */}
          <div className="flex items-center gap-2">
            {/* Quick Demo Switcher Dropdown */}
            <div className="relative">
              <button
                onClick={() => setSwitchOpen(!switchOpen)}
                className="flex items-center gap-1.5 px-2.5 py-1.5 text-xs font-medium rounded-lg border border-slate-300 dark:border-slate-700 bg-slate-50 dark:bg-slate-800 hover:bg-slate-100 dark:hover:bg-slate-700/80 transition-colors"
                title="Switch test persona"
              >
                <Layers className="h-3.5 w-3.5 text-indigo-600 dark:text-indigo-400" />
                <span className="hidden sm:inline">Role Switcher</span>
                <ChevronDown className="h-3 w-3 text-slate-500" />
              </button>

              {switchOpen && (
                <div className="absolute right-0 mt-2 w-64 rounded-xl border border-slate-200 dark:border-slate-800 bg-white dark:bg-slate-900 shadow-xl p-2 z-50 text-xs">
                  <p className="px-2 py-1 text-[11px] font-semibold text-slate-400 uppercase tracking-wider">
                    Quick-Switch Persona (Viva Demo)
                  </p>
                  <button
                    onClick={() => handleQuickLogin("prof.sharma@college.edu", "faculty123")}
                    className="w-full text-left px-2.5 py-2 rounded-lg hover:bg-indigo-50 dark:hover:bg-slate-800 flex flex-col transition-colors"
                  >
                    <span className="font-semibold text-slate-900 dark:text-slate-100">
                      Prof. Rajesh Sharma
                    </span>
                    <span className="text-[11px] text-slate-500">Faculty (CS301 & CS304)</span>
                  </button>

                  <button
                    onClick={() => handleQuickLogin("rahul.verma@college.edu", "student123")}
                    className="w-full text-left px-2.5 py-2 rounded-lg hover:bg-amber-50 dark:hover:bg-slate-800 flex flex-col transition-colors border-t border-slate-100 dark:border-slate-800"
                  >
                    <span className="font-semibold text-amber-700 dark:text-amber-400">
                      Rahul Verma (High Risk)
                    </span>
                    <span className="text-[11px] text-slate-500">64% &bull; 4 consecutive absences</span>
                  </button>

                  <button
                    onClick={() => handleQuickLogin("aryan.sharma@college.edu", "student123")}
                    className="w-full text-left px-2.5 py-2 rounded-lg hover:bg-emerald-50 dark:hover:bg-slate-800 flex flex-col transition-colors"
                  >
                    <span className="font-semibold text-emerald-700 dark:text-emerald-400">
                      Aryan Sharma (Low Risk)
                    </span>
                    <span className="text-[11px] text-slate-500">94% &bull; Exemplary attendance</span>
                  </button>

                  <button
                    onClick={() => handleQuickLogin("admin@college.edu", "admin123")}
                    className="w-full text-left px-2.5 py-2 rounded-lg hover:bg-indigo-50 dark:hover:bg-slate-800 flex flex-col transition-colors border-t border-slate-100 dark:border-slate-800"
                  >
                    <span className="font-semibold text-slate-900 dark:text-slate-100">
                      Dean of Academics
                    </span>
                    <span className="text-[11px] text-slate-500">Admin full-access</span>
                  </button>
                </div>
              )}
            </div>

            {currentUser ? (
              <div className="flex items-center gap-2">
                <div className="hidden sm:flex flex-col text-right">
                  <span className="text-xs font-semibold text-slate-800 dark:text-slate-200">
                    {currentUser.name}
                  </span>
                  <span className="text-[10px] font-mono text-indigo-600 dark:text-indigo-400 uppercase">
                    {currentUser.role}
                  </span>
                </div>
                <button
                  onClick={handleLogout}
                  className="p-1.5 rounded-lg text-slate-500 hover:text-red-600 hover:bg-red-50 dark:hover:bg-red-950/50 transition-colors"
                  title="Logout"
                >
                  <LogOut className="h-4 w-4" />
                </button>
              </div>
            ) : (
              <Link
                href="/login"
                className="px-3 py-1.5 text-xs font-semibold rounded-lg bg-indigo-600 text-white hover:bg-indigo-700 transition-colors shadow-sm"
              >
                Login
              </Link>
            )}

            {/* Mobile Hamburger Toggle */}
            <button
              onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
              className="md:hidden p-2 rounded-lg text-slate-600 dark:text-slate-300 hover:bg-slate-100 dark:hover:bg-slate-800 transition-colors"
              aria-label="Toggle navigation menu"
            >
              {mobileMenuOpen ? <X className="h-5 w-5" /> : <Menu className="h-5 w-5" />}
            </button>
          </div>
        </div>
      </div>

      {/* Mobile Drawer Menu */}
      {mobileMenuOpen && (
        <div className="md:hidden border-t border-slate-200 dark:border-slate-800 bg-white/95 dark:bg-slate-900/95 backdrop-blur-lg px-4 pt-3 pb-5 space-y-3 animate-in slide-in-from-top-2 duration-200">
          <nav className="flex flex-col gap-1">
            {navLinks.map((link) => {
              const Icon = link.icon;
              const isActive = pathname === link.href;

              return (
                <Link
                  key={link.href}
                  href={link.href}
                  onClick={() => setMobileMenuOpen(false)}
                  className={`flex items-center justify-between px-3.5 py-2.5 rounded-xl text-sm font-medium transition-all ${
                    isActive
                      ? "bg-indigo-50 dark:bg-indigo-950/60 text-indigo-700 dark:text-indigo-300 font-semibold"
                      : "text-slate-700 dark:text-slate-200 hover:bg-slate-100 dark:hover:bg-slate-800"
                  }`}
                >
                  <div className="flex items-center gap-3">
                    <Icon className={`h-4 w-4 ${link.highlight ? "text-indigo-600 dark:text-indigo-400" : ""}`} />
                    <span>{link.label}</span>
                  </div>
                  {link.highlight && (
                    <span className="text-[10px] uppercase font-bold tracking-wider px-2 py-0.5 rounded-full bg-indigo-100 text-indigo-700 dark:bg-indigo-950 dark:text-indigo-300">
                      Viva Ready
                    </span>
                  )}
                </Link>
              );
            })}
          </nav>

          {/* Mobile Persona Switcher Quick Links */}
          <div className="pt-3 border-t border-slate-100 dark:border-slate-800 space-y-2">
            <p className="text-[11px] font-semibold text-slate-400 uppercase tracking-wider px-1">
              Switch Persona (Viva Demo)
            </p>
            <div className="grid grid-cols-2 gap-2">
              <button
                onClick={() => handleQuickLogin("prof.sharma@college.edu", "faculty123")}
                className="p-2 text-left rounded-lg bg-slate-50 dark:bg-slate-800/80 hover:bg-indigo-50 text-xs border border-slate-200/80 dark:border-slate-700"
              >
                <div className="font-semibold text-slate-800 dark:text-slate-200">Prof. Sharma</div>
                <div className="text-[10px] text-slate-400">Faculty</div>
              </button>
              <button
                onClick={() => handleQuickLogin("rahul.verma@college.edu", "student123")}
                className="p-2 text-left rounded-lg bg-amber-50/60 dark:bg-amber-950/20 hover:bg-amber-100/80 text-xs border border-amber-200/80 dark:border-amber-800/50"
              >
                <div className="font-semibold text-amber-700 dark:text-amber-300">Rahul Verma</div>
                <div className="text-[10px] text-amber-600/75">64% &bull; High Risk</div>
              </button>
              <button
                onClick={() => handleQuickLogin("aryan.sharma@college.edu", "student123")}
                className="p-2 text-left rounded-lg bg-emerald-50/60 dark:bg-emerald-950/20 hover:bg-emerald-100/80 text-xs border border-emerald-200/80 dark:border-emerald-800/50"
              >
                <div className="font-semibold text-emerald-700 dark:text-emerald-300">Aryan Sharma</div>
                <div className="text-[10px] text-emerald-600/75">94% &bull; Safe</div>
              </button>
              <button
                onClick={() => handleQuickLogin("admin@college.edu", "admin123")}
                className="p-2 text-left rounded-lg bg-slate-50 dark:bg-slate-800/80 hover:bg-indigo-50 text-xs border border-slate-200/80 dark:border-slate-700"
              >
                <div className="font-semibold text-slate-800 dark:text-slate-200">Dean Raman</div>
                <div className="text-[10px] text-slate-400">Admin</div>
              </button>
            </div>
          </div>
        </div>
      )}
    </header>
  );
}
