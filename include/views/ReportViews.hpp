#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "views/LayoutView.hpp"
#include "models/Models.hpp"

namespace attendance::views {

class ReportViews {
public:
    static std::string renderReportsIndex(const PageContext& ctx) {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Reports & Data Export Center</h1>\n";
        ss << "    <p class=\"page-subtitle\">Generate official CSV exports and printable transcripts directly from the C++20 core engine.</p>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap:20px;\">\n";

        // Card 1: Student Attendance Summary CSV
        ss << "<div class=\"stat-card\">\n";
        ss << "  <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "    <h4 style=\"font-size:1.1rem; font-weight:700;\">Student Attendance Summary</h4>\n";
        ss << "    <span class=\"badge\" style=\"background:#ecfdf5; color:#065f46;\">CSV Export</span>\n";
        ss << "  </div>\n";
        ss << "  <p style=\"font-size:0.85rem; color:#64748b; margin-bottom:18px;\">Complete institutional roster including overall attendance %, conducted sessions, attended points, streak info, and risk tiers.</p>\n";
        ss << "  <a href=\"/reports/students.csv\" class=\"btn btn-primary\" style=\"margin-top:auto;\">Download Students CSV &rarr;</a>\n";
        ss << "</div>\n";

        // Card 2: Academic Risk & Intervention Report CSV
        ss << "<div class=\"stat-card\">\n";
        ss << "  <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "    <h4 style=\"font-size:1.1rem; font-weight:700;\">Shortage & Risk Report</h4>\n";
        ss << "    <span class=\"badge\" style=\"background:#fef2f2; color:#991b1b;\">High Risk CSV</span>\n";
        ss << "  </div>\n";
        ss << "  <p style=\"font-size:0.85rem; color:#64748b; margin-bottom:18px;\">Targeted export of students below 75% threshold or flagged as HIGH/CRITICAL risk by the ensemble ML predictor, including recovery targets.</p>\n";
        ss << "  <a href=\"/reports/risk.csv\" class=\"btn btn-danger\" style=\"margin-top:auto;\">Download Risk Report CSV &rarr;</a>\n";
        ss << "</div>\n";

        // Card 3: Course Subject Analytics CSV
        ss << "<div class=\"stat-card\">\n";
        ss << "  <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "    <h4 style=\"font-size:1.1rem; font-weight:700;\">Subject-wise Analytics</h4>\n";
        ss << "    <span class=\"badge\" style=\"background:#eff6ff; color:#1d4ed8;\">Curriculum CSV</span>\n";
        ss << "  </div>\n";
        ss << "  <p style=\"font-size:0.85rem; color:#64748b; margin-bottom:18px;\">Course-by-course breakdown: assigned faculty, enrollment tallies, sessions conducted, and average class attendance percentage.</p>\n";
        ss << "  <a href=\"/reports/subjects.csv\" class=\"btn btn-secondary\" style=\"margin-top:auto;\">Download Subjects CSV &rarr;</a>\n";
        ss << "</div>\n";

        // Card 4: Faculty Teaching Roster CSV
        ss << "<div class=\"stat-card\">\n";
        ss << "  <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;\">\n";
        ss << "    <h4 style=\"font-size:1.1rem; font-weight:700;\">Faculty Teaching Load</h4>\n";
        ss << "    <span class=\"badge\" style=\"background:#f5f3ff; color:#6d28d9;\">Staff CSV</span>\n";
        ss << "  </div>\n";
        ss << "  <p style=\"font-size:0.85rem; color:#64748b; margin-bottom:18px;\">Faculty directory with employee ID, department, assigned course subjects, and conducted lecture statistics.</p>\n";
        ss << "  <a href=\"/reports/faculty.csv\" class=\"btn btn-secondary\" style=\"margin-top:auto;\">Download Faculty CSV &rarr;</a>\n";
        ss << "</div>\n";

        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Printable Student Transcript View
    static std::string renderPrintableTranscript(
        const attendance::models::StudentAttendanceProfile& profile,
        const std::vector<attendance::models::AttendanceRecord>& history,
        double threshold = 75.0) 
    {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n";
        ss << "<html><head><meta charset=\"UTF-8\" />";
        ss << "<title>Official Attendance Transcript — " << LayoutView::escapeHtml(profile.name) << "</title>";
        ss << "<link rel=\"stylesheet\" href=\"/public/css/style.css\" />";
        ss << "<style>body{background:#fff; padding:40px; font-family:var(--font-sans); color:#000;}</style>";
        ss << "</head><body>";

        ss << "<div style=\"display:flex; justify-content:space-between; align-items:flex-start; border-bottom:2px solid #0f172a; padding-bottom:16px; margin-bottom:24px;\">";
        ss << "<div>";
        ss << "<h1 style=\"font-size:1.6rem; font-weight:800; text-transform:uppercase;\">National Institute of Technology</h1>";
        ss << "<h2 style=\"font-size:1.1rem; font-weight:600; color:#475569;\">Official Student Attendance Transcript & Risk Record</h2>";
        ss << "</div>";
        ss << "<div style=\"text-align:right; font-size:0.85rem; color:#64748b;\">";
        ss << "<div>Date of Issue: November 2026</div>";
        ss << "<div>Academic Term: Fall Semester 2026</div>";
        ss << "</div>";
        ss << "</div>";

        // Student Info
        ss << "<table style=\"width:100%; border-collapse:collapse; margin-bottom:24px; font-size:0.9rem;\">";
        ss << "<tr><td style=\"padding:6px 0; color:#64748b;\">Student Name:</td><td><strong>" << LayoutView::escapeHtml(profile.name) << "</strong></td>";
        ss << "<td style=\"padding:6px 0; color:#64748b;\">University Roll Number:</td><td><strong>" << LayoutView::escapeHtml(profile.rollNumber) << "</strong></td></tr>";
        ss << "<tr><td style=\"padding:6px 0; color:#64748b;\">Department:</td><td>" << LayoutView::escapeHtml(profile.department) << "</td>";
        ss << "<td style=\"padding:6px 0; color:#64748b;\">Current Semester:</td><td>Semester " << profile.semester << "</td></tr>";
        ss << "<tr><td style=\"padding:6px 0; color:#64748b;\">Overall Attendance:</td><td><strong style=\"font-size:1.1rem;\">" << std::fixed << std::setprecision(1) << profile.overallPercentage << "%</strong></td>";
        ss << "<td style=\"padding:6px 0; color:#64748b;\">Eligibility Status:</td><td><strong>" << (profile.overallPercentage >= threshold ? "ELIGIBLE FOR SEMESTER EXAMINATIONS" : "ATTENDANCE DEFICIT SHORTAGE") << "</strong></td></tr>";
        ss << "</table>";

        // Subject Breakdown
        ss << "<h3 style=\"font-size:1rem; font-weight:700; border-bottom:1px solid #e2e8f0; padding-bottom:6px; margin-bottom:12px;\">Course Subject Performance</h3>";
        ss << "<table style=\"width:100%; border-collapse:collapse; font-size:0.85rem; margin-bottom:28px;\">";
        ss << "<thead><tr style=\"background:#f8fafc; border-bottom:1px solid #cbd5e1;\">";
        ss << "<th style=\"padding:8px; text-align:left;\">Code</th><th style=\"padding:8px; text-align:left;\">Subject Name</th>";
        ss << "<th style=\"padding:8px; text-align:center;\">Conducted</th><th style=\"padding:8px; text-align:center;\">Attended</th><th style=\"padding:8px; text-align:center;\">Percentage</th>";
        ss << "</tr></thead><tbody>";

        for (const auto& [id, sub] : profile.subjectWise) {
            ss << "<tr style=\"border-bottom:1px solid #e2e8f0;\">";
            ss << "<td style=\"padding:8px;\"><strong>" << LayoutView::escapeHtml(sub.subjectCode) << "</strong></td>";
            ss << "<td style=\"padding:8px;\">" << LayoutView::escapeHtml(sub.subjectName) << "</td>";
            ss << "<td style=\"padding:8px; text-align:center;\">" << sub.totalConducted << "</td>";
            ss << "<td style=\"padding:8px; text-align:center;\">" << sub.presentCount << "</td>";
            ss << "<td style=\"padding:8px; text-align:center;\"><strong>" << std::fixed << std::setprecision(1) << sub.percentage << "%</strong></td>";
            ss << "</tr>";
        }
        ss << "</tbody></table>";

        // Signature Bar
        ss << "<div style=\"display:flex; justify-content:space-between; margin-top:60px; font-size:0.85rem;\">";
        ss << "<div style=\"text-align:center; width:200px; border-top:1px solid #000; padding-top:6px;\">Academic Dean Signature</div>";
        ss << "<div style=\"text-align:center; width:200px; border-top:1px solid #000; padding-top:6px;\">Registrar & University Seal</div>";
        ss << "</div>";

        ss << "</body></html>";
        return ss.str();
    }
};

} // namespace attendance::views
