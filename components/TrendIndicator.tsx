import React from "react";
import { TrendType } from "@/lib/dsa/types";
import { TrendingUp, TrendingDown, Minus } from "lucide-react";

interface TrendIndicatorProps {
  trend: TrendType | string;
  slope?: number;
  showSlope?: boolean;
}

export default function TrendIndicator({
  trend,
  slope,
  showSlope = false,
}: TrendIndicatorProps) {
  const t = (trend || "STABLE").toUpperCase();

  if (t === "IMPROVING") {
    return (
      <span className="inline-flex items-center gap-1 text-xs font-medium text-emerald-600 dark:text-emerald-400">
        <TrendingUp className="h-3.5 w-3.5" />
        <span>Improving</span>
        {showSlope && slope !== undefined && (
          <span className="text-[10px] font-mono text-slate-400">({slope > 0 ? "+" : ""}{slope})</span>
        )}
      </span>
    );
  }

  if (t === "DECLINING") {
    return (
      <span className="inline-flex items-center gap-1 text-xs font-medium text-rose-600 dark:text-rose-400">
        <TrendingDown className="h-3.5 w-3.5" />
        <span>Declining</span>
        {showSlope && slope !== undefined && (
          <span className="text-[10px] font-mono text-slate-400">({slope})</span>
        )}
      </span>
    );
  }

  return (
    <span className="inline-flex items-center gap-1 text-xs font-medium text-slate-500 dark:text-slate-400">
      <Minus className="h-3.5 w-3.5" />
      <span>Stable</span>
    </span>
  );
}
