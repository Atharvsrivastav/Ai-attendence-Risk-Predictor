#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace attendance::views {

class SvgCharts {
public:
    // 1. Circular Attendance Donut Gauge
    static std::string renderAttendanceGauge(double percentage, double threshold = 75.0, int size = 200) {
        percentage = std::max(0.0, std::min(100.0, percentage));
        double radius = 70.0;
        double circumference = 2.0 * 3.141592653589793 * radius;
        double offset = circumference - (percentage / 100.0) * circumference;

        std::string strokeColor = "#10b981"; // Emerald
        std::string statusText = "ELIGIBLE FOR EXAMS";
        std::string statusColor = "#065f46";
        std::string bgStatus = "#ecfdf5";

        if (percentage < threshold) {
            if (percentage < 65.0) {
                strokeColor = "#ef4444"; // Red
                statusText = "CRITICAL RISK - SHORTAGE";
                statusColor = "#991b1b";
                bgStatus = "#fef2f2";
            } else {
                strokeColor = "#f59e0b"; // Amber
                statusText = "BORDERLINE RISK (<75%)";
                statusColor = "#92400e";
                bgStatus = "#fffbeb";
            }
        }

        std::stringstream ss;
        ss << "<div style=\"display:flex; flex-direction:column; align-items:center;\">";
        ss << "<svg width=\"" << size << "\" height=\"" << size << "\" viewBox=\"0 0 180 180\" class=\"gauge-svg\">";
        // Background Circle Track
        ss << "<circle cx=\"90\" cy=\"90\" r=\"" << radius << "\" fill=\"none\" stroke=\"#e2e8f0\" stroke-width=\"14\" />";
        // Value Circle
        ss << "<circle cx=\"90\" cy=\"90\" r=\"" << radius << "\" fill=\"none\" stroke=\"" << strokeColor << "\" stroke-width=\"14\" "
           << "stroke-dasharray=\"" << circumference << "\" stroke-dashoffset=\"" << offset << "\" "
           << "stroke-linecap=\"round\" transform=\"rotate(-90 90 90)\" style=\"transition: stroke-dashoffset 0.5s ease;\" />";
        // Center Text: Percentage
        ss << "<text x=\"90\" y=\"88\" text-anchor=\"middle\" dominant-baseline=\"central\" font-size=\"30\" font-weight=\"800\" fill=\"#0f172a\">"
           << std::fixed << std::setprecision(1) << percentage << "%</text>";
        // Center Subtitle: Requirement
        ss << "<text x=\"90\" y=\"110\" text-anchor=\"middle\" font-size=\"11\" font-weight=\"600\" fill=\"#64748b\">"
           << "Threshold: " << std::fixed << std::setprecision(0) << threshold << "%</text>";
        ss << "</svg>";
        // Pill status
        ss << "<span style=\"margin-top:10px; padding:4px 12px; border-radius:9999px; font-size:0.75rem; font-weight:700; background-color:" 
           << bgStatus << "; color:" << statusColor << "; border:1px solid " << strokeColor << "40;\">" 
           << statusText << "</span>";
        ss << "</div>";
        return ss.str();
    }

    // 2. Responsive Subject-wise Attendance Bar Chart
    static std::string renderSubjectBarChart(
        const std::vector<std::string>& labels,
        const std::vector<double>& values,
        double threshold = 75.0,
        int width = 540,
        int height = 240) 
    {
        if (labels.empty() || values.empty()) {
            return "<div class=\"empty-chart\">No subject attendance data available</div>";
        }

        int n = static_cast<int>(std::min(labels.size(), values.size()));
        int padLeft = 45;
        int padRight = 20;
        int padTop = 30;
        int padBottom = 40;

        int chartW = width - padLeft - padRight;
        int chartH = height - padTop - padBottom;

        std::stringstream ss;
        ss << "<svg width=\"100%\" height=\"" << height << "\" viewBox=\"0 0 " << width << " " << height << "\" style=\"overflow:visible;\">";

        // Grid lines (0, 25, 50, 75, 100)
        for (int pct = 0; pct <= 100; pct += 25) {
            double y = padTop + chartH - (pct / 100.0) * chartH;
            ss << "<line x1=\"" << padLeft << "\" y1=\"" << y << "\" x2=\"" << (width - padRight) << "\" y2=\"" << y << "\" stroke=\"#f1f5f9\" stroke-width=\"1\" />";
            ss << "<text x=\"" << (padLeft - 8) << "\" y=\"" << (y + 4) << "\" text-anchor=\"end\" font-size=\"10\" fill=\"#94a3b8\">" << pct << "%</text>";
        }

        // Threshold Reference Line (75%)
        double threshY = padTop + chartH - (threshold / 100.0) * chartH;
        ss << "<line x1=\"" << padLeft << "\" y1=\"" << threshY << "\" x2=\"" << (width - padRight) << "\" y2=\"" << threshY 
           << "\" stroke=\"#ef4444\" stroke-width=\"1.5\" stroke-dasharray=\"4 4\" />";
        ss << "<text x=\"" << (width - padRight) << "\" y=\"" << (threshY - 5) << "\" text-anchor=\"end\" font-size=\"10\" font-weight=\"600\" fill=\"#ef4444\">"
           << "Min " << std::fixed << std::setprecision(0) << threshold << "%</text>";

        // Bars
        double slotW = static_cast<double>(chartW) / n;
        double barW = std::min(42.0, slotW * 0.65);

        for (int i = 0; i < n; i++) {
            double val = std::max(0.0, std::min(100.0, values[i]));
            double bH = (val / 100.0) * chartH;
            double bX = padLeft + (i * slotW) + (slotW - barW) / 2.0;
            double bY = padTop + chartH - bH;

            std::string color = (val >= threshold) ? "#10b981" : (val >= 65.0 ? "#f59e0b" : "#ef4444");

            // Rounded bar rect
            ss << "<rect x=\"" << bX << "\" y=\"" << bY << "\" width=\"" << barW << "\" height=\"" << bH 
               << "\" rx=\"4\" fill=\"" << color << "\" />";

            // Value label above bar
            ss << "<text x=\"" << (bX + barW / 2.0) << "\" y=\"" << (bY - 6) << "\" text-anchor=\"middle\" font-size=\"11\" font-weight=\"700\" fill=\"#0f172a\">"
               << std::fixed << std::setprecision(0) << val << "%</text>";

            // Subject label below bar
            ss << "<text x=\"" << (bX + barW / 2.0) << "\" y=\"" << (padTop + chartH + 18) << "\" text-anchor=\"middle\" font-size=\"11\" font-weight=\"600\" fill=\"#475569\">"
               << labels[i] << "</text>";
        }

        ss << "</svg>";
        return ss.str();
    }

    // 3. Attendance Trend Smooth Line Chart
    static std::string renderAttendanceTrendChart(
        const std::vector<std::string>& dates,
        const std::vector<double>& percentages,
        double threshold = 75.0,
        int width = 540,
        int height = 240) 
    {
        if (percentages.empty()) {
            return "<div class=\"empty-chart\">Insufficient data for trend chart</div>";
        }

        int n = static_cast<int>(percentages.size());
        int padLeft = 45;
        int padRight = 20;
        int padTop = 30;
        int padBottom = 40;

        int chartW = width - padLeft - padRight;
        int chartH = height - padTop - padBottom;

        std::stringstream ss;
        ss << "<svg width=\"100%\" height=\"" << height << "\" viewBox=\"0 0 " << width << " " << height << "\">";
        ss << "<defs>";
        ss << "<linearGradient id=\"trendGrad\" x1=\"0%\" y1=\"0%\" x2=\"0%\" y2=\"100%\">";
        ss << "<stop offset=\"0%\" stop-color=\"#3b82f6\" stop-opacity=\"0.35\" />";
        ss << "<stop offset=\"100%\" stop-color=\"#3b82f6\" stop-opacity=\"0.0\" />";
        ss << "</linearGradient>";
        ss << "</defs>";

        // Grid lines (0, 25, 50, 75, 100)
        for (int pct = 0; pct <= 100; pct += 25) {
            double y = padTop + chartH - (pct / 100.0) * chartH;
            ss << "<line x1=\"" << padLeft << "\" y1=\"" << y << "\" x2=\"" << (width - padRight) << "\" y2=\"" << y << "\" stroke=\"#f1f5f9\" stroke-width=\"1\" />";
            ss << "<text x=\"" << (padLeft - 8) << "\" y=\"" << (y + 4) << "\" text-anchor=\"end\" font-size=\"10\" fill=\"#94a3b8\">" << pct << "%</text>";
        }

        // Threshold Reference Line (75%)
        double threshY = padTop + chartH - (threshold / 100.0) * chartH;
        ss << "<line x1=\"" << padLeft << "\" y1=\"" << threshY << "\" x2=\"" << (width - padRight) << "\" y2=\"" << threshY 
           << "\" stroke=\"#ef4444\" stroke-width=\"1.5\" stroke-dasharray=\"4 4\" />";

        std::vector<std::pair<double, double>> coords;
        for (int i = 0; i < n; i++) {
            double x = (n == 1) ? (padLeft + chartW / 2.0) : (padLeft + (static_cast<double>(i) / (n - 1)) * chartW);
            double val = std::max(0.0, std::min(100.0, percentages[i]));
            double y = padTop + chartH - (val / 100.0) * chartH;
            coords.push_back({x, y});
        }

        // Area fill
        if (coords.size() >= 2) {
            ss << "<path d=\"M " << coords[0].first << " " << (padTop + chartH) << " ";
            for (const auto& pt : coords) {
                ss << "L " << pt.first << " " << pt.second << " ";
            }
            ss << "L " << coords.back().first << " " << (padTop + chartH) << " Z\" fill=\"url(#trendGrad)\" />";
        }

        // Stroke line
        if (coords.size() >= 2) {
            ss << "<path d=\"M ";
            for (size_t i = 0; i < coords.size(); i++) {
                if (i > 0) ss << " L ";
                ss << coords[i].first << " " << coords[i].second;
            }
            ss << "\" fill=\"none\" stroke=\"#2563eb\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />";
        }

        // Data points & X labels
        for (size_t i = 0; i < coords.size(); i++) {
            ss << "<circle cx=\"" << coords[i].first << "\" cy=\"" << coords[i].second << "\" r=\"4.5\" fill=\"#ffffff\" stroke=\"#2563eb\" stroke-width=\"2.5\" />";
            if (i < dates.size() && (n <= 8 || i % (n / 6 + 1) == 0 || i == coords.size() - 1)) {
                ss << "<text x=\"" << coords[i].first << "\" y=\"" << (padTop + chartH + 18) << "\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">"
                   << dates[i] << "</text>";
            }
        }

        ss << "</svg>";
        return ss.str();
    }

    // 4. Institutional Risk Distribution Donut Chart
    static std::string renderRiskDistributionDonut(int low, int medium, int high, int critical, int size = 260) {
        int total = low + medium + high + critical;
        if (total == 0) total = 1;

        double pLow = static_cast<double>(low) / total;
        double pMed = static_cast<double>(medium) / total;
        double pHigh = static_cast<double>(high) / total;
        double pCrit = static_cast<double>(critical) / total;

        double radius = 64.0;
        double circumference = 2.0 * 3.141592653589793 * radius;

        double offLow = 0.0;
        double lenLow = pLow * circumference;

        double offMed = lenLow;
        double lenMed = pMed * circumference;

        double offHigh = lenLow + lenMed;
        double lenHigh = pHigh * circumference;

        double offCrit = lenLow + lenMed + lenHigh;
        double lenCrit = pCrit * circumference;

        std::stringstream ss;
        ss << "<div style=\"display:flex; align-items:center; justify-content:center; gap:24px; flex-wrap:wrap;\">";
        
        // Donut SVG
        ss << "<svg width=\"" << size << "\" height=\"" << size << "\" viewBox=\"0 0 160 160\">";
        ss << "<circle cx=\"80\" cy=\"80\" r=\"" << radius << "\" fill=\"none\" stroke=\"#f1f5f9\" stroke-width=\"22\" />";
        
        // Low Slice (Emerald)
        if (low > 0) {
            ss << "<circle cx=\"80\" cy=\"80\" r=\"" << radius << "\" fill=\"none\" stroke=\"#10b981\" stroke-width=\"22\" "
               << "stroke-dasharray=\"" << lenLow << " " << circumference << "\" stroke-dashoffset=\"" << -offLow << "\" "
               << "transform=\"rotate(-90 80 80)\" />";
        }
        // Medium Slice (Amber)
        if (medium > 0) {
            ss << "<circle cx=\"80\" cy=\"80\" r=\"" << radius << "\" fill=\"none\" stroke=\"#f59e0b\" stroke-width=\"22\" "
               << "stroke-dasharray=\"" << lenMed << " " << circumference << "\" stroke-dashoffset=\"" << -offMed << "\" "
               << "transform=\"rotate(-90 80 80)\" />";
        }
        // High Slice (Orange)
        if (high > 0) {
            ss << "<circle cx=\"80\" cy=\"80\" r=\"" << radius << "\" fill=\"none\" stroke=\"#f97316\" stroke-width=\"22\" "
               << "stroke-dasharray=\"" << lenHigh << " " << circumference << "\" stroke-dashoffset=\"" << -offHigh << "\" "
               << "transform=\"rotate(-90 80 80)\" />";
        }
        // Critical Slice (Red)
        if (critical > 0) {
            ss << "<circle cx=\"80\" cy=\"80\" r=\"" << radius << "\" fill=\"none\" stroke=\"#ef4444\" stroke-width=\"22\" "
               << "stroke-dasharray=\"" << lenCrit << " " << circumference << "\" stroke-dashoffset=\"" << -offCrit << "\" "
               << "transform=\"rotate(-90 80 80)\" />";
        }

        // Center Text
        ss << "<text x=\"80\" y=\"76\" text-anchor=\"middle\" dominant-baseline=\"central\" font-size=\"24\" font-weight=\"800\" fill=\"#0f172a\">"
           << (low + medium + high + critical) << "</text>";
        ss << "<text x=\"80\" y=\"94\" text-anchor=\"middle\" font-size=\"10\" font-weight=\"600\" fill=\"#64748b\">STUDENTS</text>";
        ss << "</svg>";

        // Legend Pills
        ss << "<div style=\"display:flex; flex-direction:column; gap:8px; min-width:140px;\">";
        ss << "<div style=\"display:flex; align-items:center; justify-content:space-between; font-size:0.82rem;\">"
           << "<span style=\"display:flex; align-items:center; gap:6px;\"><span style=\"width:10px; height:10px; border-radius:50%; background:#10b981;\"></span>Low</span>"
           << "<strong style=\"color:#0f172a;\">" << low << " (" << std::fixed << std::setprecision(0) << (pLow * 100.0) << "%)</strong></div>";
        ss << "<div style=\"display:flex; align-items:center; justify-content:space-between; font-size:0.82rem;\">"
           << "<span style=\"display:flex; align-items:center; gap:6px;\"><span style=\"width:10px; height:10px; border-radius:50%; background:#f59e0b;\"></span>Medium</span>"
           << "<strong style=\"color:#0f172a;\">" << medium << " (" << std::fixed << std::setprecision(0) << (pMed * 100.0) << "%)</strong></div>";
        ss << "<div style=\"display:flex; align-items:center; justify-content:space-between; font-size:0.82rem;\">"
           << "<span style=\"display:flex; align-items:center; gap:6px;\"><span style=\"width:10px; height:10px; border-radius:50%; background:#f97316;\"></span>High</span>"
           << "<strong style=\"color:#0f172a;\">" << high << " (" << std::fixed << std::setprecision(0) << (pHigh * 100.0) << "%)</strong></div>";
        ss << "<div style=\"display:flex; align-items:center; justify-content:space-between; font-size:0.82rem;\">"
           << "<span style=\"display:flex; align-items:center; gap:6px;\"><span style=\"width:10px; height:10px; border-radius:50%; background:#ef4444;\"></span>Critical</span>"
           << "<strong style=\"color:#0f172a;\">" << critical << " (" << std::fixed << std::setprecision(0) << (pCrit * 100.0) << "%)</strong></div>";
        ss << "</div>";

        ss << "</div>";
        return ss.str();
    }

    // 5. Mini Sparkline
    static std::string renderMiniSparkline(const std::vector<double>& points, const std::string& strokeColor = "#2563eb", int width = 70, int height = 24) {
        if (points.size() < 2) return "";

        double minV = *std::min_element(points.begin(), points.end());
        double maxV = *std::max_element(points.begin(), points.end());
        if (maxV == minV) maxV += 1.0;

        std::stringstream ss;
        ss << "<svg width=\"" << width << "\" height=\"" << height << "\" viewBox=\"0 0 " << width << " " << height << "\">";
        ss << "<polyline fill=\"none\" stroke=\"" << strokeColor << "\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" points=\"";
        
        for (size_t i = 0; i < points.size(); i++) {
            double x = (static_cast<double>(i) / (points.size() - 1)) * (width - 4) + 2;
            double y = height - 2 - ((points[i] - minV) / (maxV - minV)) * (height - 6);
            ss << x << "," << y << " ";
        }
        ss << "\" />";
        ss << "</svg>";
        return ss.str();
    }
};

} // namespace attendance::views
