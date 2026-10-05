import type { Metadata } from "next";
import "./globals.css";
import Navbar from "@/components/Navbar";

export const metadata: Metadata = {
  title: "AI Attendance Risk Prediction System | DSA + ML Mini Project",
  description:
    "An intelligent attendance monitoring and risk prediction system powered by core Data Structures & Algorithms and explainable Machine Learning.",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <body className="bg-slate-50 dark:bg-slate-950 text-slate-900 dark:text-slate-100 min-h-screen flex flex-col antialiased selection:bg-indigo-500 selection:text-white">
        <Navbar />
        <main className="flex-1 w-full max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
          {children}
        </main>
        <footer className="border-t border-slate-200 dark:border-slate-800 bg-white dark:bg-slate-900/50 py-6 text-center text-xs text-slate-500">
          <div className="max-w-7xl mx-auto px-4 flex flex-col sm:flex-row items-center justify-between gap-3">
            <p>
              AI Attendance Risk Prediction System &bull; DSA Mini Project (Array, HashMap, Set, Queue, Stack, Tree, Graph)
            </p>
            <div className="flex items-center gap-4 text-xs font-mono text-slate-600 dark:text-slate-400">
              <span className="flex items-center gap-1.5">
                <span className="h-2 w-2 rounded-full bg-emerald-500 animate-pulse"></span>
                DSA Core: Active
              </span>
              <span>&bull;</span>
              <span>ML Engine: DecisionTree + Logistic Regression</span>
            </div>
          </div>
        </footer>
      </body>
    </html>
  );
}
