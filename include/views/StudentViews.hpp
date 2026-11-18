#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "models/Models.hpp"
#include "views/LayoutView.hpp"
#include "views/SvgCharts.hpp"
#include "algorithms/RecoveryCalculator.hpp"

namespace attendance::views {

class StudentViews {
public:
    static std::string renderStudentDashboard(
        const attendance::models::StudentAttendanceProfile& profile,
        const std::vector<attendance::models::AttendanceRecord>& history,
        const attendance::algorithms::SimulationResult* simulationResult,
        int simulatedAttendNext,
        int simulatedMissNext,
        const PageContext& ctx,
        double threshold = 75.0) 
    {
        std::stringstream ss;

        // Page Header
        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Student Attendance & Risk Analytics</h1>\n";
        ss << "    <p class=\"page-subtitle\">" << LayoutView::escapeHtml(profile.name) 
           << " • Roll No: <strong>" << LayoutView::escapeHtml(profile.rollNumber) << "</strong>"
           << " • " << LayoutView::escapeHtml(profile.department) << " (Sem " << profile.semester << ")</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/reports/print/student\" target=\"_blank\" class=\"btn btn-secondary\">\n";
        ss << "      <svg width=\"16\" height=\"16\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M17 17h2a2 2 0 002-2v-4a2 2 0 00-2-2H5a2 2 0 00-2 2v4a2 2 0 002 2h2m2 4h6a2 2 0 002-2v-4a2 2 0 00-2-2H9a2 2 0 00-2 2v4a2 2 0 002 2zm8-12V5a2 2 0 00-2-2H9a2 2 0 00-2 2v4h10z\"/></svg>\n";
        ss << "      Print Transcript</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Prominent 75% Threshold Alert Warning if below threshold
        if (profile.overallPercentage < threshold) {
            double deficit = threshold - profile.overallPercentage;
            ss << "<div class=\"alert-banner alert-danger\">\n";
            ss << "  <svg width=\"24\" height=\"24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z\"/></svg>\n";
            ss << "  <div>\n";
            ss << "    <strong>Warning: Attendance Deficit (" << std::fixed << std::setprecision(1) << profile.overallPercentage << "% < " << threshold << "%)</strong>\n";
            ss << "    <p style=\"margin-top:2px; font-size:0.85rem;\">You are currently " << std::fixed << std::setprecision(1) << deficit << "% below the mandatory university threshold. You must attend the next <strong>" 
               << profile.classesNeededToRecover << " consecutive classes</strong> to become eligible for semester examinations.</p>\n";
            ss << "  </div>\n";
            ss << "</div>\n";
        }

        // Consecutive Absence Streak Alert
        if (profile.currentConsecutiveAbsences >= 2) {
            ss << "<div class=\"alert-banner alert-warning\">\n";
            ss << "  <svg width=\"20\" height=\"20\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M13 10V3L4 14h7v7l9-11h-7z\"/></svg>\n";
            ss << "  <div>\n";
            ss << "    <strong>Active Consecutive Absence Streak Detected: " << profile.currentConsecutiveAbsences << " Sessions Missed</strong>\n";
            ss << "    <p style=\"margin-top:2px; font-size:0.85rem;\">Repeated unexcused absences trigger automated academic counseling alerts.</p>\n";
            ss << "  </div>\n";
            ss << "</div>\n";
        }

        // Top Row: Circular Attendance Gauge + 4 Metric Cards
        ss << "<div style=\"display:grid; grid-template-columns: 280px 1fr; gap:24px; margin-bottom:28px;\">\n";
        
        // Gauge Card
        ss << "  <div class=\"chart-card\" style=\"align-items:center; justify-content:center; text-align:center;\">\n";
        ss << "    <h3 class=\"chart-title\" style=\"margin-bottom:14px;\">Overall Attendance</h3>\n";
        ss << SvgCharts::renderAttendanceGauge(profile.overallPercentage, threshold, 190);
        ss << "  </div>\n";

        // Metrics Grid (4 cards)
        ss << "  <div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap:16px;\">\n";
        ss << LayoutView::renderStatCard("Total Conducted", std::to_string(profile.totalSessionsConducted), "Lectures held to date", "#2563eb");
        ss << LayoutView::renderStatCard("Attended Points", std::to_string(profile.totalSessionsAttended), "P=1.0, Late=0.5", "#10b981");
        ss << LayoutView::renderStatCard("Classes Missed", std::to_string(profile.totalSessionsAbsent), "Absent sessions", "#ef4444");
        
        if (profile.overallPercentage >= threshold) {
            ss << LayoutView::renderStatCard("Safe Miss Allowance", std::to_string(profile.classesCanBeMissed), "Can miss without falling below 75%", "#10b981");
        } else {
            ss << LayoutView::renderStatCard("Classes to Recover", std::to_string(profile.classesNeededToRecover), "Required to hit 75% requirement", "#f59e0b");
        }
        ss << "  </div>\n";
        ss << "</div>\n";

        // Middle Row: Subject-wise Breakdown Table
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">Subject-wise Attendance Breakdown</h3>\n";
        ss << "    <span style=\"font-size:0.8rem; color:#64748b;\">Threshold: " << threshold << "%</span>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Subject Code</th>\n";
        ss << "        <th>Course Name</th>\n";
        ss << "        <th>Conducted</th>\n";
        ss << "        <th>Attended</th>\n";
        ss << "        <th>Percentage</th>\n";
        ss << "        <th>Status</th>\n";
        ss << "        <th>Streak</th>\n";
        ss << "        <th>Recovery Plan</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (profile.subjectWise.empty()) {
            ss << "      <tr><td colspan=\"8\" style=\"text-align:center; padding:24px; color:#94a3b8;\">No course enrollments found</td></tr>\n";
        } else {
            for (const auto& [subId, sub] : profile.subjectWise) {
                ss << "      <tr>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(sub.subjectCode) << "</strong></td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(sub.subjectName) << "</td>\n";
                ss << "        <td>" << sub.totalConducted << "</td>\n";
                ss << "        <td>" << sub.presentCount << " <span style=\"font-size:0.75rem; color:#64748b;\">(+" << sub.lateCount << " L)</span></td>\n";
                
                std::string pctColor = (sub.percentage >= threshold) ? "#065f46" : (sub.percentage >= 65.0 ? "#92400e" : "#991b1b");
                ss << "        <td><strong style=\"color:" << pctColor << "; font-size:0.95rem;\">" << std::fixed << std::setprecision(1) << sub.percentage << "%</strong></td>\n";
                
                ss << "        <td>";
                if (sub.percentage >= threshold) {
                    ss << "<span class=\"badge badge-present\">Eligible</span>";
                } else if (sub.percentage >= 65.0) {
                    ss << "<span class=\"badge badge-late\">Warning</span>";
                } else {
                    ss << "<span class=\"badge badge-absent\">Deficit</span>";
                }
                ss << "</td>\n";

                ss << "        <td>" << (sub.consecutiveAbsences > 0 ? ("<span style=\"color:#ef4444; font-weight:700;\">" + std::to_string(sub.consecutiveAbsences) + " absent</span>") : "<span style=\"color:#10b981;\">Clear</span>") << "</td>\n";
                
                ss << "        <td>";
                if (sub.percentage >= threshold) {
                    ss << "<span style=\"color:#10b981; font-size:0.8rem;\">Can miss " << sub.classesCanBeMissed << " lectures</span>";
                } else {
                    ss << "<span style=\"color:#ef4444; font-weight:600; font-size:0.8rem;\">Must attend " << sub.classesNeededToRecover << " in a row</span>";
                }
                ss << "</td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        // Two Column: What-If Attendance Recovery Calculator + Explainable AI Risk Evaluation
        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(420px, 1fr)); gap:24px; margin-bottom:28px;\">\n";

        // What-If Calculator Card
        ss << "  <div class=\"form-card\" style=\"margin-bottom:0;\">\n";
        ss << "    <h3 class=\"chart-title\" style=\"margin-bottom:4px;\">What-If Attendance Recovery Calculator</h3>\n";
        ss << "    <p style=\"font-size:0.82rem; color:#64748b; margin-bottom:18px;\">Simulate future attendance momentum to project semester eligibility status.</p>\n";
        
        ss << "    <form action=\"/student/simulate\" method=\"POST\">\n";
        if (!ctx.csrfToken.empty()) {
            ss << "      <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
        }
        ss << "      <div class=\"form-row\">\n";
        ss << "        <div class=\"form-col\">\n";
        ss << "          <label class=\"form-label\" for=\"attendNext\">Next Classes to Attend</label>\n";
        ss << "          <input type=\"number\" class=\"form-input\" id=\"attendNext\" name=\"attendNext\" min=\"0\" max=\"50\" value=\"" << simulatedAttendNext << "\" required />\n";
        ss << "        </div>\n";
        ss << "        <div class=\"form-col\">\n";
        ss << "          <label class=\"form-label\" for=\"missNext\">Next Classes to Miss</label>\n";
        ss << "          <input type=\"number\" class=\"form-input\" id=\"missNext\" name=\"missNext\" min=\"0\" max=\"50\" value=\"" << simulatedMissNext << "\" required />\n";
        ss << "        </div>\n";
        ss << "      </div>\n";
        ss << "      <button type=\"submit\" class=\"btn btn-primary\" style=\"margin-top:14px; width:100%;\">Run Simulation</button>\n";
        ss << "    </form>\n";

        // If simulation was run, display projected results
        if (simulationResult != nullptr) {
            ss << "    <div style=\"margin-top:20px; padding:16px; background:#f8fafc; border:1px solid #e2e8f0; border-radius:8px;\">\n";
            ss << "      <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;\">\n";
            ss << "        <span style=\"font-size:0.85rem; font-weight:600; color:#475569;\">Projected Attendance:</span>\n";
            ss << "        <strong style=\"font-size:1.25rem; color:" << (simulationResult->meetsThreshold ? "#10b981" : "#ef4444") << ";\">"
               << std::fixed << std::setprecision(1) << simulationResult->projectedPercentage << "%</strong>\n";
            ss << "      </div>\n";
            ss << "      <div style=\"font-size:0.82rem; color:#475569;\">\n";
            if (simulationResult->meetsThreshold) {
                ss << "      <span style=\"color:#065f46; font-weight:600;\">✓ Goal Satisfied:</span> This schedule will restore your attendance above the " << threshold << "% eligibility threshold.";
            } else {
                ss << "      <span style=\"color:#991b1b; font-weight:600;\">✗ Deficit Remains:</span> You will still be " 
                   << std::fixed << std::setprecision(1) << (threshold - simulationResult->projectedPercentage) 
                   << "% below threshold. Attend more consecutive sessions to recover.";
            }
            ss << "      </div>\n";
            ss << "    </div>\n";
        }

        ss << "  </div>\n";

        // Explainable AI Risk Prediction Card
        ss << "  <div class=\"chart-card\" style=\"margin-bottom:0;\">\n";
        ss << "    <div style=\"display:flex; justify-content:space-between; align-items:flex-start;\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"chart-title\">AI Attendance Risk Assessment</h3>\n";
        ss << "        <p class=\"chart-subtitle\">Ensemble Logistic Scoring + Decision Tree</p>\n";
        ss << "      </div>\n";
        
        models::RiskLevel rLevel = models::stringToRisk(profile.riskCategory);
        ss << LayoutView::renderRiskBadge(rLevel);
        ss << "    </div>\n";

        ss << "    <div style=\"margin:16px 0; padding:12px; background:#f8fafc; border-radius:8px;\">\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; font-size:0.85rem; margin-bottom:6px;\">\n";
        ss << "        <span style=\"color:#64748b;\">Estimated Risk Score:</span>\n";
        ss << "        <strong>" << std::fixed << std::setprecision(2) << profile.riskScore << " / 1.00</strong>\n";
        ss << "      </div>\n";
        ss << "      <div style=\"display:flex; justify-content:space-between; font-size:0.85rem;\">\n";
        ss << "        <span style=\"color:#64748b;\">Attendance Momentum:</span>\n";
        ss << "        <strong>" << LayoutView::escapeHtml(profile.trend) << " (" << std::fixed << std::setprecision(3) << profile.trendSlope << ")</strong>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";

        ss << "    <h4 style=\"font-size:0.82rem; text-transform:uppercase; color:#64748b; font-weight:700; margin-bottom:8px;\">Key Contributing Factors</h4>\n";
        ss << "    <ul style=\"margin-left:18px; font-size:0.82rem; color:#334155; margin-bottom:14px; line-height:1.6;\">\n";
        if (profile.contributingFactors.empty()) {
            ss << "      <li>Attendance consistently satisfies university requirements.</li>\n";
        } else {
            for (const auto& factor : profile.contributingFactors) {
                ss << "      <li>" << LayoutView::escapeHtml(factor) << "</li>\n";
            }
        }
        ss << "    </ul>\n";

        if (!profile.recommendation.empty()) {
            ss << "    <div style=\"padding:10px 14px; background:#eff6ff; border-left:3px solid #2563eb; border-radius:4px; font-size:0.82rem; color:#1e40af;\">\n";
            ss << "      <strong>Recommendation:</strong> " << LayoutView::escapeHtml(profile.recommendation) << "\n";
            ss << "    </div>\n";
        }

        ss << "  </div>\n";
        ss << "</div>\n";

        // Bottom Row: Chronological Attendance History Table
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">Chronological Lecture Session History</h3>\n";
        ss << "    <span style=\"font-size:0.8rem; color:#64748b;\">Showing " << history.size() << " lectures</span>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Date</th>\n";
        ss << "        <th>Subject</th>\n";
        ss << "        <th>Lecture Topic</th>\n";
        ss << "        <th>Faculty</th>\n";
        ss << "        <th>Status</th>\n";
        ss << "        <th>Remarks</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (history.empty()) {
            ss << "      <tr><td colspan=\"6\" style=\"text-align:center; padding:24px; color:#94a3b8;\">No lecture records logged yet</td></tr>\n";
        } else {
            for (const auto& rec : history) {
                ss << "      <tr>\n";
                ss << "        <td>" << LayoutView::escapeHtml(rec.sessionDate.substr(0, 10)) << "</td>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(rec.subjectCode) << "</strong></td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(rec.topic.empty() ? "Regular Lecture" : rec.topic) << "</td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(rec.facultyName) << "</td>\n";
                ss << "        <td>" << LayoutView::renderStatusBadge(rec.status) << "</td>\n";
                ss << "        <td style=\"color:#64748b; font-size:0.82rem;\">" << LayoutView::escapeHtml(rec.remarks) << "</td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }
};

} // namespace attendance::views
