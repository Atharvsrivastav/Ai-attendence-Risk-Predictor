import React from "react";
import { RiskCategory } from "@/lib/dsa/types";
import { AlertTriangle, AlertCircle, CheckCircle2, ShieldAlert } from "lucide-react";

interface RiskBadgeProps {
  category: RiskCategory | string;
  score?: number;
  showIcon?: boolean;
  size?: "sm" | "md" | "lg";
}

export default function RiskBadge({
  category,
  score,
  showIcon = true,
  size = "md",
}: RiskBadgeProps) {
  const cat = (category || "LOW").toUpperCase();

  const sizeClasses = {
    sm: "px-2 py-0.5 text-xs gap-1",
    md: "px-2.5 py-1 text-xs gap-1.5 font-medium",
    lg: "px-3.5 py-1.5 text-sm gap-2 font-semibold",
  };

  if (cat === "CRITICAL") {
    return (
      <span
        className={`inline-flex items-center rounded-full bg-red-100 text-red-700 dark:bg-red-950/70 dark:text-red-300 border border-red-300 dark:border-red-800 ${sizeClasses[size]}`}
      >
        {showIcon && <ShieldAlert className="h-3.5 w-3.5 text-red-600 animate-pulse" />}
        <span>CRITICAL RISK</span>
        {score !== undefined && <span className="opacity-80 font-mono">({Math.round(score * 100)}%)</span>}
      </span>
    );
  }

  if (cat === "HIGH") {
    return (
      <span
        className={`inline-flex items-center rounded-full bg-amber-100 text-amber-800 dark:bg-amber-950/60 dark:text-amber-300 border border-amber-300 dark:border-amber-800 ${sizeClasses[size]}`}
      >
        {showIcon && <AlertTriangle className="h-3.5 w-3.5 text-amber-600" />}
        <span>HIGH RISK</span>
        {score !== undefined && <span className="opacity-80 font-mono">({Math.round(score * 100)}%)</span>}
      </span>
    );
  }

  if (cat === "MEDIUM") {
    return (
      <span
        className={`inline-flex items-center rounded-full bg-yellow-100 text-yellow-800 dark:bg-yellow-950/60 dark:text-yellow-300 border border-yellow-300 dark:border-yellow-800 ${sizeClasses[size]}`}
      >
        {showIcon && <AlertCircle className="h-3.5 w-3.5 text-yellow-600" />}
        <span>MEDIUM RISK</span>
        {score !== undefined && <span className="opacity-80 font-mono">({Math.round(score * 100)}%)</span>}
      </span>
    );
  }

  return (
    <span
      className={`inline-flex items-center rounded-full bg-emerald-100 text-emerald-800 dark:bg-emerald-950/60 dark:text-emerald-300 border border-emerald-300 dark:border-emerald-800 ${sizeClasses[size]}`}
    >
      {showIcon && <CheckCircle2 className="h-3.5 w-3.5 text-emerald-600" />}
      <span>LOW RISK</span>
      {score !== undefined && <span className="opacity-80 font-mono">({Math.round(score * 100)}%)</span>}
    </span>
  );
}
