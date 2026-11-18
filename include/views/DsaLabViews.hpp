#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "views/LayoutView.hpp"
#include "algorithms/Sorting.hpp"
#include "algorithms/Searching.hpp"
#include "dsa/StudentHashMap.hpp"
#include "dsa/AttendanceGraph.hpp"

namespace attendance::views {

struct DsaLabData {
    algorithms::SortMetrics<models::StudentAttendanceProfile> mergeSortMetrics;
    algorithms::SortMetrics<models::StudentAttendanceProfile> quickSortMetrics;
    algorithms::SearchMetrics linearSearchMetrics;
    algorithms::SearchMetrics binarySearchMetrics;
    size_t hashMapCapacity{0};
    size_t hashMapSize{0};
    double hashMapLoadFactor{0.0};
    size_t hashMapCollisions{0};
    dsa::TraversalResult bfsResult;
    dsa::TraversalResult dfsResult;
    std::vector<std::string> shortestPath;
    std::string startNode;
    std::string targetNode;
};

class DsaLabViews {
public:
    static std::string renderDsaLab(const DsaLabData& data, const PageContext& ctx) {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Interactive Data Structures & Algorithms Lab</h1>\n";
        ss << "    <p class=\"page-subtitle\">Real-time execution telemetry and complexity verification across C++20 custom DSA engines.</p>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Section 1: Sorting Algorithms Comparison (MergeSort vs QuickSort)
        ss << "<div class=\"chart-card\" style=\"margin-bottom:28px;\">\n";
        ss << "  <div class=\"chart-header\">\n";
        ss << "    <div>\n";
        ss << "      <h3 class=\"chart-title\">Sorting Engine Benchmark: MergeSort vs QuickSort</h3>\n";
        ss << "      <p class=\"chart-subtitle\">Executed on actual student attendance datasets using custom C++ algorithmic implementations.</p>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "  <div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap:20px; margin-top:8px;\">\n";
        
        // Merge Sort Box
        ss << "    <div style=\"padding:18px; background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;\">\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "        <strong style=\"color:#2563eb; font-size:1.05rem;\">Merge Sort</strong>\n";
        ss << "        <span class=\"badge\" style=\"background:#eff6ff; color:#1d4ed8;\">O(n log n)</span>\n";
        ss << "      </div>\n";
        ss << "      <div style=\"display:flex; flex-direction:column; gap:8px; font-size:0.85rem;\">\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Records Sorted:</span> <strong>" << data.mergeSortMetrics.sorted.size() << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Key Comparisons:</span> <strong>" << data.mergeSortMetrics.comparisons << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Element Swaps / Copies:</span> <strong>" << data.mergeSortMetrics.swaps << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Execution Duration:</span> <strong style=\"color:#10b981;\">" << data.mergeSortMetrics.durationMicroseconds << " &mu;s</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Stability Guarantee:</span> <strong>Stable</strong></div>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";

        // Quick Sort Box
        ss << "    <div style=\"padding:18px; background:#f8fafc; border:1px solid #e2e8f0; border-radius:10px;\">\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "        <strong style=\"color:#8b5cf6; font-size:1.05rem;\">Quick Sort (Lomuto)</strong>\n";
        ss << "        <span class=\"badge\" style=\"background:#f5f3ff; color:#6d28d9;\">O(n log n)</span>\n";
        ss << "      </div>\n";
        ss << "      <div style=\"display:flex; flex-direction:column; gap:8px; font-size:0.85rem;\">\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Records Sorted:</span> <strong>" << data.quickSortMetrics.sorted.size() << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Key Comparisons:</span> <strong>" << data.quickSortMetrics.comparisons << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">In-Place Swaps:</span> <strong>" << data.quickSortMetrics.swaps << "</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Execution Duration:</span> <strong style=\"color:#10b981;\">" << data.quickSortMetrics.durationMicroseconds << " &mu;s</strong></div>\n";
        ss << "        <div style=\"display:flex; justify-content:space-between;\"><span style=\"color:#64748b;\">Space Complexity:</span> <strong>O(log n) stack</strong></div>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";

        ss << "  </div>\n";
        ss << "</div>\n";

        // Section 2: Searching Step-by-Step Traces (Linear vs Binary Search)
        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(420px, 1fr)); gap:24px; margin-bottom:28px;\">\n";

        // Linear Search Trace
        ss << "  <div class=\"card-table-wrapper\" style=\"margin-bottom:0;\">\n";
        ss << "    <div class=\"card-header-toolbar\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"table-title\">Linear Search Trace</h3>\n";
        ss << "        <span style=\"font-size:0.75rem; color:#64748b;\">Sequential Scan • O(n) Time</span>\n";
        ss << "      </div>\n";
        ss << "      <span class=\"badge\" style=\"background:#eff6ff; color:#2563eb;\">" << data.linearSearchMetrics.stepsCount << " Steps (" << data.linearSearchMetrics.timeMicroseconds << " &mu;s)</span>\n";
        ss << "    </div>\n";
        ss << "    <table class=\"data-table\" style=\"font-size:0.8rem;\">\n";
        ss << "      <thead><tr><th>Step</th><th>Index</th><th>Value Examined</th><th>Outcome</th></tr></thead>\n";
        ss << "      <tbody>\n";
        for (size_t i = 0; i < std::min(data.linearSearchMetrics.trace.size(), size_t(8)); i++) {
            const auto& t = data.linearSearchMetrics.trace[i];
            ss << "        <tr>\n";
            ss << "          <td>#" << (i + 1) << "</td>\n";
            ss << "          <td>Index " << t.index << "</td>\n";
            ss << "          <td><code>" << LayoutView::escapeHtml(t.valueExamined) << "</code></td>\n";
            ss << "          <td>" << (t.isMatch ? "<span style=\"color:#10b981; font-weight:700;\">Match Found!</span>" : "<span style=\"color:#64748b;\">Next</span>") << "</td>\n";
            ss << "        </tr>\n";
        }
        ss << "      </tbody>\n";
        ss << "    </table>\n";
        ss << "  </div>\n";

        // Binary Search Trace
        ss << "  <div class=\"card-table-wrapper\" style=\"margin-bottom:0;\">\n";
        ss << "    <div class=\"card-header-toolbar\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"table-title\">Binary Search Trace</h3>\n";
        ss << "        <span style=\"font-size:0.75rem; color:#64748b;\">Divide & Conquer • O(log n) Time</span>\n";
        ss << "      </div>\n";
        ss << "      <span class=\"badge\" style=\"background:#ecfdf5; color:#065f46;\">" << data.binarySearchMetrics.stepsCount << " Steps (" << data.binarySearchMetrics.timeMicroseconds << " &mu;s)</span>\n";
        ss << "    </div>\n";
        ss << "    <table class=\"data-table\" style=\"font-size:0.8rem;\">\n";
        ss << "      <thead><tr><th>Step</th><th>Range [L, Mid, R]</th><th>Value</th><th>Action Taken</th></tr></thead>\n";
        ss << "      <tbody>\n";
        for (size_t i = 0; i < data.binarySearchMetrics.trace.size(); i++) {
            const auto& t = data.binarySearchMetrics.trace[i];
            ss << "        <tr>\n";
            ss << "          <td>#" << (i + 1) << "</td>\n";
            ss << "          <td>[" << t.low << ", <strong>" << t.mid << "</strong>, " << t.high << "]</td>\n";
            ss << "          <td><code>" << LayoutView::escapeHtml(t.valueExamined) << "</code></td>\n";
            ss << "          <td style=\"font-size:0.75rem;\">" << LayoutView::escapeHtml(t.actionTaken) << "</td>\n";
            ss << "        </tr>\n";
        }
        ss << "      </tbody>\n";
        ss << "    </table>\n";
        ss << "  </div>\n";

        ss << "</div>\n";

        // Section 3: StudentHashMap Diagnostics & Graph Traversals
        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(420px, 1fr)); gap:24px; margin-bottom:28px;\">\n";

        // StudentHashMap Inspector
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <h3 class=\"chart-title\" style=\"margin-bottom:4px;\">Custom StudentHashMap&lt;K, V&gt; Diagnostics</h3>\n";
        ss << "    <p class=\"chart-subtitle\" style=\"margin-bottom:16px;\">Polynomial rolling hash (djb2) with separate chaining buckets.</p>\n";
        
        ss << "    <div style=\"display:flex; flex-direction:column; gap:10px; font-size:0.88rem;\">\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Hash Function:</span> <strong>djb2 (hash * 33 + char)</strong></div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Current Bucket Capacity:</span> <strong>" << data.hashMapCapacity << " buckets</strong></div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Total Stored Keys:</span> <strong>" << data.hashMapSize << " items</strong></div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Calculated Load Factor:</span> <strong>" << std::fixed << std::setprecision(3) << data.hashMapLoadFactor << " / 0.750 max</strong></div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Chaining Collisions Handled:</span> <strong>" << data.hashMapCollisions << "</strong></div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; padding:10px; background:#f8fafc; border-radius:6px;\">"
           << "<span style=\"color:#64748b;\">Auto-Rehash Resizing Threshold:</span> <strong>> 0.75 (Doubles capacity)</strong></div>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";

        // AttendanceGraph Traversal
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <h3 class=\"chart-title\" style=\"margin-bottom:4px;\">AttendanceGraph Relationship Traversal</h3>\n";
        ss << "    <p class=\"chart-subtitle\" style=\"margin-bottom:16px;\">Adjacency list connecting Faculty &rarr; Subjects &rarr; Students.</p>\n";
        
        ss << "    <div style=\"display:flex; flex-direction:column; gap:10px; font-size:0.85rem;\">\n";
        ss << "      <div style=\"padding:10px; background:#f8fafc; border-radius:6px;\">\n";
        ss << "        <div style=\"color:#64748b; font-size:0.75rem; font-weight:700; text-transform:uppercase;\">Breadth-First Search (BFS) Traversal Order:</div>\n";
        ss << "        <div style=\"margin-top:4px; font-weight:600; color:#0f172a;\">" << (data.bfsResult.order.empty() ? "None" : "") << " ";
        for (size_t i = 0; i < std::min(data.bfsResult.order.size(), size_t(6)); i++) {
            if (i > 0) ss << " &rarr; ";
            ss << LayoutView::escapeHtml(data.bfsResult.order[i]);
        }
        if (data.bfsResult.order.size() > 6) ss << " ... (" << data.bfsResult.nodesVisited << " nodes visited)";
        ss << "        </div>\n";
        ss << "      </div>\n";

        ss << "      <div style=\"padding:10px; background:#f8fafc; border-radius:6px;\">\n";
        ss << "        <div style=\"color:#64748b; font-size:0.75rem; font-weight:700; text-transform:uppercase;\">Depth-First Search (DFS) Traversal:</div>\n";
        ss << "        <div style=\"margin-top:4px; font-weight:600; color:#0f172a;\">" << (data.dfsResult.order.empty() ? "None" : "") << " ";
        for (size_t i = 0; i < std::min(data.dfsResult.order.size(), size_t(6)); i++) {
            if (i > 0) ss << " &rarr; ";
            ss << LayoutView::escapeHtml(data.dfsResult.order[i]);
        }
        if (data.dfsResult.order.size() > 6) ss << " ... (" << data.dfsResult.nodesVisited << " nodes visited)";
        ss << "        </div>\n";
        ss << "      </div>\n";

        ss << "      <div style=\"padding:10px; background:#eff6ff; border:1px solid #bfdbfe; border-radius:6px;\">\n";
        ss << "        <div style=\"color:#1e40af; font-size:0.75rem; font-weight:700; text-transform:uppercase;\">Shortest Path (" << LayoutView::escapeHtml(data.startNode) << " &rarr; " << LayoutView::escapeHtml(data.targetNode) << "):</div>\n";
        ss << "        <div style=\"margin-top:4px; font-weight:700; color:#1d4ed8;\">";
        if (data.shortestPath.empty()) {
            ss << "No direct path discovered";
        } else {
            for (size_t i = 0; i < data.shortestPath.size(); i++) {
                if (i > 0) ss << " &rarr; ";
                ss << LayoutView::escapeHtml(data.shortestPath[i]);
            }
        }
        ss << "        </div>\n";
        ss << "      </div>\n";

        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }
};

} // namespace attendance::views
