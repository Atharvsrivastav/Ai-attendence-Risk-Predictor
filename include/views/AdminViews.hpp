#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include "models/Models.hpp"
#include "views/LayoutView.hpp"
#include "views/SvgCharts.hpp"

namespace attendance::views {

struct AdminOverviewData {
    int totalStudents{0};
    int totalFaculty{0};
    int totalSubjects{0};
    int totalSessions{0};
    double averageAttendance{0.0};
    int belowThresholdCount{0};
    int highRiskCount{0};
    int criticalRiskCount{0};
    int lowRiskCount{0};
    int mediumRiskCount{0};
    std::vector<std::string> subjectCodes;
    std::vector<double> subjectAverages;
    std::vector<attendance::models::StudentAttendanceProfile> atRiskProfiles;
};

struct AdminAnalyticsData {
    int totalStudents{0};
    int totalFaculty{0};
    int totalSubjects{0};
    int totalSessions{0};
    double averageAttendance{0.0};
    int belowThresholdCount{0};
    int highRiskCount{0};
    int criticalRiskCount{0};
    int lowRiskCount{0};
    int mediumRiskCount{0};
    std::vector<std::string> subjectCodes;
    std::vector<double> subjectAverages;
    std::vector<std::string> timelineLabels;
    std::vector<double> timelineAverages;
    std::vector<attendance::models::StudentAttendanceProfile> profiles;
    std::vector<attendance::models::Subject> subjects;
    nlohmann::json alerts;
};

class AdminViews {
public:
    static std::string renderOverview(
        const AdminOverviewData& data,
        const PageContext& ctx,
        double threshold = 75.0) 
    {
        std::stringstream ss;

        // Page Header
        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Institutional Executive Overview</h1>\n";
        ss << "    <p class=\"page-subtitle\">Real-time academic attendance health & predictive risk distribution.</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/admin/analytics\" class=\"btn btn-secondary\">Deep Analytics &rarr;</a>\n";
        ss << "    <a href=\"/admin/students/new\" class=\"btn btn-primary\">+ Add Student</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Top 6 Metrics Grid
        ss << "<div class=\"metrics-grid\">\n";
        ss << LayoutView::renderStatCard("Total Students", std::to_string(data.totalStudents), "Active enrollment", "#2563eb", "", "/admin/students");
        ss << LayoutView::renderStatCard("Faculty Staff", std::to_string(data.totalFaculty), "Teaching professors", "#10b981", "", "/admin/faculty");
        ss << LayoutView::renderStatCard("Course Subjects", std::to_string(data.totalSubjects), "Active curriculum", "#8b5cf6", "", "/admin/subjects");
        ss << LayoutView::renderStatCard("Sessions Held", std::to_string(data.totalSessions), "Conducted lectures", "#06b6d4");
        
        std::stringstream ssAvg;
        ssAvg << std::fixed << std::setprecision(1) << data.averageAttendance << "%";
        ss << LayoutView::renderStatCard("Institution Average", ssAvg.str(), "Across all courses", (data.averageAttendance >= threshold ? "#10b981" : "#f59e0b"), "", "/admin/analytics");

        ss << LayoutView::renderStatCard("Shortage Deficit", std::to_string(data.belowThresholdCount), "Students below " + std::to_string(static_cast<int>(threshold)) + "%", "#ef4444", "", "/reports");
        ss << "</div>\n";

        // Charts Grid: Subject-wise Bar Chart + Risk Donut Chart
        ss << "<div class=\"charts-grid\">\n";
        
        // Subject Bar Chart
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <div class=\"chart-header\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"chart-title\">Subject-wise Attendance Distribution</h3>\n";
        ss << "        <p class=\"chart-subtitle\">Average student attendance compared to " << threshold << "% threshold</p>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";
        ss << "    <div class=\"chart-container\">\n";
        ss << SvgCharts::renderSubjectBarChart(data.subjectCodes, data.subjectAverages, threshold, 520, 240);
        ss << "    </div>\n";
        ss << "  </div>\n";

        // Risk Donut Chart
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <div class=\"chart-header\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"chart-title\">Student Risk Classification</h3>\n";
        ss << "        <p class=\"chart-subtitle\">Ensemble ML breakdown across student body</p>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";
        ss << "    <div class=\"chart-container\">\n";
        ss << SvgCharts::renderRiskDistributionDonut(data.lowRiskCount, data.mediumRiskCount, data.highRiskCount, data.criticalRiskCount, 220);
        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "</div>\n";

        // Urgent Action Table: Students below threshold requiring intervention
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">Students Requiring Academic Intervention (< " << threshold << "% Attendance)</h3>\n";
        ss << "    <a href=\"/admin/students\" class=\"btn btn-secondary btn-sm\">View All Students</a>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Roll Number</th>\n";
        ss << "        <th>Student Name</th>\n";
        ss << "        <th>Department</th>\n";
        ss << "        <th>Overall %</th>\n";
        ss << "        <th>Risk Tier</th>\n";
        ss << "        <th>Streak</th>\n";
        ss << "        <th>Recovery Plan</th>\n";
        ss << "        <th>Actions</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (data.atRiskProfiles.empty()) {
            ss << "      <tr><td colspan=\"8\" style=\"text-align:center; padding:24px; color:#10b981; font-weight:600;\">✓ Excellent! All students are currently above the " << threshold << "% threshold.</td></tr>\n";
        } else {
            for (const auto& prof : data.atRiskProfiles) {
                models::RiskLevel rLevel = models::stringToRisk(prof.riskCategory);
                ss << "      <tr>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(prof.rollNumber) << "</strong></td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(prof.name) << "</td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(prof.department) << "</td>\n";
                ss << "        <td><strong style=\"color:#dc2626;\">" << std::fixed << std::setprecision(1) << prof.overallPercentage << "%</strong></td>\n";
                ss << "        <td>" << LayoutView::renderRiskBadge(rLevel) << "</td>\n";
                ss << "        <td>" << (prof.currentConsecutiveAbsences > 0 ? ("<span style=\"color:#ef4444; font-weight:700;\">" + std::to_string(prof.currentConsecutiveAbsences) + " missed</span>") : "<span style=\"color:#10b981;\">0</span>") << "</td>\n";
                ss << "        <td><span style=\"color:#b91c1c; font-weight:600; font-size:0.8rem;\">Attend " << prof.classesNeededToRecover << " sessions</span></td>\n";
                ss << "        <td><a href=\"/admin/students/" << prof.studentId << "\" class=\"btn btn-secondary btn-sm\">Profile</a></td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Student Directory List
    static std::string renderStudentList(
        const std::vector<attendance::models::StudentAttendanceProfile>& students,
        const std::string& searchQuery,
        const std::string& selectedDept,
        int selectedSem,
        const PageContext& ctx,
        double threshold = 75.0) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Student Academic Directory</h1>\n";
        ss << "    <p class=\"page-subtitle\">Manage student profiles, monitor attendance health, and inspect risk predictions.</p>\n";
        ss << "  </div>\n";
        ss << "  <a href=\"/admin/students/new\" class=\"btn btn-primary\">+ Add New Student</a>\n";
        ss << "</div>\n";

        // Filter & Search Toolbar Card
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <form method=\"GET\" action=\"/admin/students\" class=\"filter-bar\" style=\"width:100%;\">\n";
        ss << "      <input type=\"text\" name=\"q\" class=\"form-input\" style=\"width:240px;\" placeholder=\"Search name or roll...\" value=\"" << LayoutView::escapeHtml(searchQuery) << "\" />\n";
        
        ss << "      <select name=\"dept\" class=\"form-select\" style=\"width:180px;\">\n";
        ss << "        <option value=\"\">All Departments</option>\n";
        ss << "        <option value=\"Computer Science\"" << (selectedDept == "Computer Science" ? " selected" : "") << ">Computer Science</option>\n";
        ss << "        <option value=\"Information Technology\"" << (selectedDept == "Information Technology" ? " selected" : "") << ">Information Technology</option>\n";
        ss << "        <option value=\"Electronics\"" << (selectedDept == "Electronics" ? " selected" : "") << ">Electronics</option>\n";
        ss << "      </select>\n";

        ss << "      <select name=\"sem\" class=\"form-select\" style=\"width:140px;\">\n";
        ss << "        <option value=\"0\">All Semesters</option>\n";
        for (int s = 1; s <= 8; s++) {
            ss << "        <option value=\"" << s << "\"" << (selectedSem == s ? " selected" : "") << ">Semester " << s << "</option>\n";
        }
        ss << "      </select>\n";

        ss << "      <button type=\"submit\" class=\"btn btn-secondary btn-sm\">Apply Filter</button>\n";
        if (!searchQuery.empty() || !selectedDept.empty() || selectedSem > 0) {
            ss << "      <a href=\"/admin/students\" class=\"btn btn-sm\" style=\"color:#64748b;\">Clear</a>\n";
        }
        ss << "    </form>\n";
        ss << "  </div>\n";

        // Table
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Roll No</th>\n";
        ss << "        <th>Name</th>\n";
        ss << "        <th>Email</th>\n";
        ss << "        <th>Department</th>\n";
        ss << "        <th>Sem</th>\n";
        ss << "        <th>Attendance %</th>\n";
        ss << "        <th>Risk Tier</th>\n";
        ss << "        <th>Actions</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (students.empty()) {
            ss << "      <tr><td colspan=\"8\" style=\"text-align:center; padding:32px; color:#94a3b8;\">No students match the criteria</td></tr>\n";
        } else {
            for (const auto& stu : students) {
                models::RiskLevel rLevel = models::stringToRisk(stu.riskCategory);
                ss << "      <tr>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(stu.rollNumber) << "</strong></td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(stu.name) << "</td>\n";
                ss << "        <td style=\"color:#64748b; font-size:0.82rem;\">" << LayoutView::escapeHtml(stu.email) << "</td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(stu.department) << "</td>\n";
                ss << "        <td>Sem " << stu.semester << "</td>\n";
                
                std::string col = (stu.overallPercentage >= threshold) ? "#065f46" : (stu.overallPercentage >= 65.0 ? "#92400e" : "#991b1b");
                ss << "        <td><strong style=\"color:" << col << ";\">" << std::fixed << std::setprecision(1) << stu.overallPercentage << "%</strong></td>\n";
                ss << "        <td>" << LayoutView::renderRiskBadge(rLevel) << "</td>\n";
                ss << "        <td><a href=\"/admin/students/" << stu.studentId << "\" class=\"btn btn-secondary btn-sm\">Profile</a></td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Add Student Form View (creates setup token!)
    static std::string renderAddStudentForm(
        const std::string& setupTokenResult,
        const std::string& createdStudentEmail,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Register New Student</h1>\n";
        ss << "    <p class=\"page-subtitle\">Generate a cryptographically locked student profile and activation setup token.</p>\n";
        ss << "  </div>\n";
        ss << "  <a href=\"/admin/students\" class=\"btn btn-secondary\">&larr; Back to Directory</a>\n";
        ss << "</div>\n";

        // Display Setup Token Banner if just generated
        if (!setupTokenResult.empty()) {
            ss << "<div class=\"alert-banner alert-success\" style=\"flex-direction:column; align-items:flex-start; gap:8px;\">\n";
            ss << "  <div style=\"font-weight:700; font-size:1rem;\">✓ Student Profile Successfully Registered!</div>\n";
            ss << "  <div>The account for <strong>" << LayoutView::escapeHtml(createdStudentEmail) << "</strong> is locked until password creation.</div>\n";
            ss << "  <div style=\"width:100%; margin-top:6px; padding:12px; background:#ffffff; border:1px solid #a7f3d0; border-radius:6px;\">\n";
            ss << "    <div style=\"font-size:0.75rem; text-transform:uppercase; color:#065f46; font-weight:700;\">Single-Use Setup Token (Deliver to Student):</div>\n";
            ss << "    <code style=\"display:block; word-break:break-all; font-size:0.9rem; color:#0f172a; margin-top:4px;\">" << setupTokenResult << "</code>\n";
            ss << "  </div>\n";
            ss << "  <div style=\"font-size:0.8rem; margin-top:4px;\">The student activates their account at: <code>/setup-password</code></div>\n";
            ss << "</div>\n";
        }

        ss << "<div class=\"form-card\">\n";
        ss << "  <form action=\"/admin/students/new\" method=\"POST\">\n";
        if (!ctx.csrfToken.empty()) {
            ss << "    <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
        }

        ss << "    <div class=\"form-row\">\n";
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"name\">Full Name</label>\n";
        ss << "        <input type=\"text\" class=\"form-input\" id=\"name\" name=\"name\" placeholder=\"e.g. Rahul Verma\" required />\n";
        ss << "      </div>\n";
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"email\">Institutional Email</label>\n";
        ss << "        <input type=\"email\" class=\"form-input\" id=\"email\" name=\"email\" placeholder=\"rahul.verma@college.edu\" required />\n";
        ss << "      </div>\n";
        ss << "    </div>\n";

        ss << "    <div class=\"form-row\" style=\"margin-top:16px;\">\n";
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"rollNumber\">University Roll Number</label>\n";
        ss << "        <input type=\"text\" class=\"form-input\" id=\"rollNumber\" name=\"rollNumber\" placeholder=\"e.g. 2026CS150\" required />\n";
        ss << "      </div>\n";
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"department\">Department</label>\n";
        ss << "        <select class=\"form-select\" id=\"department\" name=\"department\" required>\n";
        ss << "          <option value=\"Computer Science\">Computer Science</option>\n";
        ss << "          <option value=\"Information Technology\">Information Technology</option>\n";
        ss << "          <option value=\"Electronics\">Electronics</option>\n";
        ss << "        </select>\n";
        ss << "      </div>\n";
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"semester\">Semester</label>\n";
        ss << "        <input type=\"number\" class=\"form-input\" id=\"semester\" name=\"semester\" value=\"5\" min=\"1\" max=\"8\" required />\n";
        ss << "      </div>\n";
        ss << "    </div>\n";

        ss << "    <div style=\"display:flex; justify-content:flex-end; gap:12px; margin-top:24px;\">\n";
        ss << "      <a href=\"/admin/students\" class=\"btn btn-secondary\">Cancel</a>\n";
        ss << "      <button type=\"submit\" class=\"btn btn-primary\">Create Student & Generate Token</button>\n";
        ss << "    </div>\n";
        ss << "  </form>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // System Config & Audit Logs View
    static std::string renderSystemConfig(
        double currentThreshold,
        const std::vector<attendance::models::AuditLog>& auditLogs,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">System Health & Institutional Audit Logs</h1>\n";
        ss << "    <p class=\"page-subtitle\">Configure global attendance thresholds and monitor system security actions.</p>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Threshold Configuration Form Card
        ss << "<div class=\"form-card\">\n";
        ss << "  <h3 class=\"chart-title\" style=\"margin-bottom:6px;\">Academic Attendance Threshold Configuration</h3>\n";
        ss << "  <p style=\"font-size:0.85rem; color:#64748b; margin-bottom:18px;\">Mandatory minimum attendance percentage required for semester exam eligibility.</p>\n";
        
        ss << "  <form action=\"/admin/system/threshold\" method=\"POST\" style=\"display:flex; gap:12px; align-items:flex-end; max-width:400px;\">\n";
        if (!ctx.csrfToken.empty()) {
            ss << "    <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
        }
        ss << "    <div style=\"flex:1;\">\n";
        ss << "      <label class=\"form-label\" for=\"threshold\">Minimum Requirement (%)</label>\n";
        ss << "      <input type=\"number\" step=\"0.5\" min=\"50.0\" max=\"95.0\" class=\"form-input\" id=\"threshold\" name=\"threshold\" value=\"" << currentThreshold << "\" required />\n";
        ss << "    </div>\n";
        ss << "    <button type=\"submit\" class=\"btn btn-primary\">Update Threshold</button>\n";
        ss << "  </form>\n";
        ss << "</div>\n";

        // Audit Logs Table
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">System Audit Trail (Recent Actions)</h3>\n";
        ss << "    <span style=\"font-size:0.8rem; color:#64748b;\">" << auditLogs.size() << " logged actions</span>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Timestamp</th>\n";
        ss << "        <th>Action</th>\n";
        ss << "        <th>Actor Email</th>\n";
        ss << "        <th>Target Entity</th>\n";
        ss << "        <th>Details</th>\n";
        ss << "        <th>IP Address</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (auditLogs.empty()) {
            ss << "      <tr><td colspan=\"6\" style=\"text-align:center; padding:24px; color:#94a3b8;\">No audit logs recorded yet</td></tr>\n";
        } else {
            for (const auto& log : auditLogs) {
                ss << "      <tr>\n";
                ss << "        <td style=\"font-size:0.8rem; color:#64748b;\">" << LayoutView::escapeHtml(log.createdAt) << "</td>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(log.action) << "</strong></td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(log.userEmail) << "</td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(log.entityType) << "</td>\n";
                ss << "        <td style=\"font-size:0.82rem; color:#475569;\">" << LayoutView::escapeHtml(log.details) << "</td>\n";
                ss << "        <td style=\"font-size:0.75rem; color:#94a3b8;\">" << LayoutView::escapeHtml(log.ipAddress) << "</td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Faculty Staff Directory View
    static std::string renderFacultyList(
        const std::vector<attendance::models::Faculty>& facultyList,
        const std::vector<attendance::models::Subject>& subjectList,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Faculty Staff Directory</h1>\n";
        ss << "    <p class=\"page-subtitle\">Institutional teaching faculty, department appointments, and course workload assignments.</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/admin/subjects\" class=\"btn btn-secondary\">View Courses &rarr;</a>\n";
        ss << "    <a href=\"/admin/students\" class=\"btn btn-secondary\">Student Directory &rarr;</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Summary Stats Grid
        std::unordered_set<std::string> depts;
        for (const auto& f : facultyList) {
            if (!f.department.empty()) depts.insert(f.department);
        }
        int totalTeachingSessions = 0;
        for (const auto& s : subjectList) {
            totalTeachingSessions += s.totalSessions;
        }

        ss << "<div class=\"metrics-grid\">\n";
        ss << LayoutView::renderStatCard("Faculty Members", std::to_string(facultyList.size()), "Active academic staff", "#10b981");
        ss << LayoutView::renderStatCard("Departments", std::to_string(depts.empty() ? 1 : depts.size()), "Academic divisions", "#6366f1");
        ss << LayoutView::renderStatCard("Course Allocations", std::to_string(subjectList.size()), "Assigned subject modules", "#8b5cf6", "", "/admin/subjects");
        ss << LayoutView::renderStatCard("Lectures Delivered", std::to_string(totalTeachingSessions), "Conducted sessions", "#06b6d4");
        ss << "</div>\n";

        // Faculty Directory Table Card
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <div>\n";
        ss << "      <h3 class=\"table-title\">Institutional Faculty Roster</h3>\n";
        ss << "      <p style=\"font-size:0.8rem; color:#64748b; margin-top:2px;\">Showing " << facultyList.size() << " registered faculty instructors</p>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Faculty Member</th>\n";
        ss << "        <th>Employee / Faculty ID</th>\n";
        ss << "        <th>Department</th>\n";
        ss << "        <th>Assigned Courses</th>\n";
        ss << "        <th>Workload</th>\n";
        ss << "        <th>Status</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (facultyList.empty()) {
            ss << "      <tr><td colspan=\"6\" style=\"text-align:center; padding:32px; color:#94a3b8;\">No faculty members found</td></tr>\n";
        } else {
            for (const auto& f : facultyList) {
                // Find subjects assigned to this faculty
                std::vector<std::string> assignedCourses;
                int facultySessions = 0;
                int facultyStudents = 0;
                for (const auto& s : subjectList) {
                    if (s.facultyId == f.id || s.facultyEmail == f.email || s.facultyId == f.userId) {
                        assignedCourses.push_back(s.code + " (" + s.name + ")");
                        facultySessions += s.totalSessions;
                        facultyStudents += s.enrolledStudents;
                    }
                }

                std::string initial = f.name.empty() ? "F" : f.name.substr(0, 1);

                ss << "      <tr>\n";
                ss << "        <td>\n";
                ss << "          <div style=\"display:flex; align-items:center; gap:10px;\">\n";
                ss << "            <div style=\"width:32px; height:32px; border-radius:50%; background:#10b98115; color:#10b981; display:flex; align-items:center; justify-content:center; font-weight:700; font-size:0.85rem;\">"
                   << LayoutView::escapeHtml(initial) << "</div>\n";
                ss << "            <div>\n";
                ss << "              <div style=\"font-weight:600; color:#0f172a;\">" << LayoutView::escapeHtml(f.name) << "</div>\n";
                ss << "              <div style=\"font-size:0.75rem; color:#64748b;\">" << LayoutView::escapeHtml(f.email) << "</div>\n";
                ss << "            </div>\n";
                ss << "          </div>\n";
                ss << "        </td>\n";
                ss << "        <td><span style=\"font-family:monospace; font-size:0.82rem; background:#f1f5f9; padding:2px 8px; border-radius:4px;\">" 
                   << LayoutView::escapeHtml(f.employeeId.empty() ? f.id : f.employeeId) << "</span></td>\n";
                ss << "        <td><span class=\"badge\" style=\"background:#e0e7ff; color:#3730a3;\">" << LayoutView::escapeHtml(f.department.empty() ? "Computer Science" : f.department) << "</span></td>\n";
                ss << "        <td>\n";
                if (assignedCourses.empty()) {
                    ss << "          <span style=\"font-size:0.8rem; color:#94a3b8;\">No current courses</span>\n";
                } else {
                    ss << "          <div style=\"display:flex; flex-wrap:wrap; gap:4px;\">\n";
                    for (const auto& c : assignedCourses) {
                        ss << "            <span class=\"badge\" style=\"background:#f8fafc; border:1px solid #e2e8f0; color:#334155; font-size:0.75rem;\">" << LayoutView::escapeHtml(c) << "</span>\n";
                    }
                    ss << "          </div>\n";
                }
                ss << "        </td>\n";
                ss << "        <td>\n";
                ss << "          <div style=\"font-size:0.82rem;\"><strong>" << facultySessions << "</strong> sessions</div>\n";
                ss << "          <div style=\"font-size:0.75rem; color:#64748b;\">" << facultyStudents << " enrolled seats</div>\n";
                ss << "        </td>\n";
                ss << "        <td><span class=\"badge badge-present\">Active Faculty</span></td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Course Curriculum & Subjects Catalog View
    static std::string renderSubjectList(
        const std::vector<attendance::models::Subject>& subjectList,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Course Curriculum & Subjects</h1>\n";
        ss << "    <p class=\"page-subtitle\">Academic subject catalog, teaching mentor allocations, credit values, and session attendance tracking.</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/admin/faculty\" class=\"btn btn-secondary\">Faculty Staff &rarr;</a>\n";
        ss << "    <a href=\"/admin/students\" class=\"btn btn-secondary\">Student Directory &rarr;</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        int totalEnrolledSeats = 0;
        int totalLecturesConducted = 0;
        int totalCredits = 0;
        for (const auto& sub : subjectList) {
            totalEnrolledSeats += sub.enrolledStudents;
            totalLecturesConducted += sub.totalSessions;
            totalCredits += sub.credits;
        }

        // Summary Stats Grid
        ss << "<div class=\"metrics-grid\">\n";
        ss << LayoutView::renderStatCard("Total Courses", std::to_string(subjectList.size()), "Accredited curriculum", "#8b5cf6");
        ss << LayoutView::renderStatCard("Enrolled Seats", std::to_string(totalEnrolledSeats), "Course student enrollments", "#2563eb", "", "/admin/students");
        ss << LayoutView::renderStatCard("Lectures Conducted", std::to_string(totalLecturesConducted), "Total classroom sessions", "#06b6d4");
        ss << LayoutView::renderStatCard("Curriculum Credits", std::to_string(totalCredits), "Total semester credits", "#10b981");
        ss << "</div>\n";

        // Subjects Table Card
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <div>\n";
        ss << "      <h3 class=\"table-title\">Institutional Subject Catalog</h3>\n";
        ss << "      <p style=\"font-size:0.8rem; color:#64748b; margin-top:2px;\">Listing all " << subjectList.size() << " active course subjects</p>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Code</th>\n";
        ss << "        <th>Course Title</th>\n";
        ss << "        <th>Credits</th>\n";
        ss << "        <th>Instructor / Faculty Mentor</th>\n";
        ss << "        <th>Enrollments</th>\n";
        ss << "        <th>Conducted Sessions</th>\n";
        ss << "        <th>Curriculum Status</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (subjectList.empty()) {
            ss << "      <tr><td colspan=\"7\" style=\"text-align:center; padding:32px; color:#94a3b8;\">No course subjects found</td></tr>\n";
        } else {
            for (const auto& sub : subjectList) {
                ss << "      <tr>\n";
                ss << "        <td><span style=\"font-family:monospace; font-weight:700; color:#2563eb; background:#eff6ff; padding:3px 8px; border-radius:4px; border:1px solid #bfdbfe;\">" 
                   << LayoutView::escapeHtml(sub.code) << "</span></td>\n";
                ss << "        <td>\n";
                ss << "          <div style=\"font-weight:600; color:#0f172a;\">" << LayoutView::escapeHtml(sub.name) << "</div>\n";
                ss << "          <div style=\"font-size:0.75rem; color:#64748b;\">ID: " << LayoutView::escapeHtml(sub.id) << "</div>\n";
                ss << "        </td>\n";
                ss << "        <td><span class=\"badge\" style=\"background:#f1f5f9; color:#475569;\">" << sub.credits << " Credits</span></td>\n";
                ss << "        <td>\n";
                if (!sub.facultyName.empty() && sub.facultyName != "Unassigned") {
                    ss << "          <div style=\"font-weight:500; color:#1e293b;\">" << LayoutView::escapeHtml(sub.facultyName) << "</div>\n";
                    if (!sub.facultyEmail.empty()) {
                        ss << "          <div style=\"font-size:0.75rem; color:#64748b;\">" << LayoutView::escapeHtml(sub.facultyEmail) << "</div>\n";
                    }
                } else {
                    ss << "          <span class=\"badge badge-absent\">Unassigned</span>\n";
                }
                ss << "        </td>\n";
                ss << "        <td><strong>" << sub.enrolledStudents << "</strong> students</td>\n";
                ss << "        <td><strong>" << sub.totalSessions << "</strong> sessions</td>\n";
                ss << "        <td><span class=\"badge badge-present\">Active Course</span></td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Deep Institutional Analytics & Risk Telemetry View
    static std::string renderDeepAnalytics(
        const AdminAnalyticsData& data,
        const PageContext& ctx,
        double threshold = 75.0) 
    {
        std::stringstream ss;

        // Page Header
        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Institutional Deep Analytics & Risk Telemetry</h1>\n";
        ss << "    <p class=\"page-subtitle\">Predictive enrollment risk distribution, subject engagement patterns, and cohort trajectory modeling.</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/admin\" class=\"btn btn-secondary\">&larr; Overview</a>\n";
        ss << "    <a href=\"/admin/students\" class=\"btn btn-secondary\">Student Directory</a>\n";
        ss << "    <a href=\"/reports\" class=\"btn btn-primary\">Export Telemetry</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Metrics Grid (6 cards)
        ss << "<div class=\"metrics-grid\">\n";
        std::stringstream ssAvg;
        ssAvg << std::fixed << std::setprecision(1) << data.averageAttendance << "%";
        ss << LayoutView::renderStatCard("Cohort Average", ssAvg.str(), "Across all curriculum", (data.averageAttendance >= threshold ? "#10b981" : "#f59e0b"));

        double deficitPct = data.totalStudents > 0 ? (data.belowThresholdCount * 100.0 / data.totalStudents) : 0.0;
        std::stringstream ssDef;
        ssDef << data.belowThresholdCount << " (" << std::fixed << std::setprecision(1) << deficitPct << "%)";
        ss << LayoutView::renderStatCard("Shortage Deficit", ssDef.str(), "Below " + std::to_string(static_cast<int>(threshold)) + "% minimum", "#ef4444", "", "/reports");

        ss << LayoutView::renderStatCard("Critical / High Risk", std::to_string(data.criticalRiskCount + data.highRiskCount), "Immediate intervention target", "#dc2626");
        ss << LayoutView::renderStatCard("Safe Cohort", std::to_string(data.lowRiskCount), "Meeting attendance benchmarks", "#10b981");
        ss << LayoutView::renderStatCard("Curriculum Modules", std::to_string(data.totalSubjects), "Active academic courses", "#8b5cf6", "", "/admin/subjects");
        ss << LayoutView::renderStatCard("Classroom Sessions", std::to_string(data.totalSessions), "Conducted lectures", "#06b6d4");
        ss << "</div>\n";

        // Visual Charts Grid: 3 Charts
        // Row 1: Subject Bar Chart + Risk Donut Chart
        ss << "<div class=\"charts-grid\">\n";
        
        // Subject Performance Bar Chart
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <div class=\"chart-header\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"chart-title\">Subject-wise Attendance Distribution</h3>\n";
        ss << "        <p class=\"chart-subtitle\">Average student attendance benchmarked against institutional " << threshold << "% threshold</p>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";
        ss << "    <div class=\"chart-container\">\n";
        ss << SvgCharts::renderSubjectBarChart(data.subjectCodes, data.subjectAverages, threshold, 520, 240);
        ss << "    </div>\n";
        ss << "  </div>\n";

        // Risk Donut Chart
        ss << "  <div class=\"chart-card\">\n";
        ss << "    <div class=\"chart-header\">\n";
        ss << "      <div>\n";
        ss << "        <h3 class=\"chart-title\">Ensemble ML Risk Classification</h3>\n";
        ss << "        <p class=\"chart-subtitle\">Predictive distribution across entire active student body</p>\n";
        ss << "      </div>\n";
        ss << "    </div>\n";
        ss << "    <div class=\"chart-container\">\n";
        ss << SvgCharts::renderRiskDistributionDonut(data.lowRiskCount, data.mediumRiskCount, data.highRiskCount, data.criticalRiskCount, 220);
        ss << "    </div>\n";
        ss << "  </div>\n";

        ss << "</div>\n";

        // Row 2: Attendance Progression Trend Chart
        if (!data.timelineLabels.empty() && !data.timelineAverages.empty()) {
            ss << "<div class=\"chart-card\" style=\"margin-bottom:28px;\">\n";
            ss << "  <div class=\"chart-header\">\n";
            ss << "    <div>\n";
            ss << "      <h3 class=\"chart-title\">Semester Session Progression & Engagement Velocity</h3>\n";
            ss << "      <p class=\"chart-subtitle\">Temporal tracking of attendance average across lecture blocks</p>\n";
            ss << "    </div>\n";
            ss << "  </div>\n";
            ss << "  <div class=\"chart-container\">\n";
            ss << SvgCharts::renderAttendanceTrendChart(data.timelineLabels, data.timelineAverages, threshold, 820, 220);
            ss << "  </div>\n";
            ss << "</div>\n";
        }

        // Section 1: Detailed Subject-Wise Analytics Table
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <div>\n";
        ss << "      <h3 class=\"table-title\">Subject Performance Matrix & Risk Exposure</h3>\n";
        ss << "      <p style=\"font-size:0.8rem; color:#64748b; margin-top:2px;\">Granular attendance statistics, student counts, and deficit exposure per course</p>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Course Code</th>\n";
        ss << "        <th>Subject Title</th>\n";
        ss << "        <th>Instructor</th>\n";
        ss << "        <th>Enrollments</th>\n";
        ss << "        <th>Sessions</th>\n";
        ss << "        <th>Average Attendance</th>\n";
        ss << "        <th>At-Risk in Subject</th>\n";
        ss << "        <th>Course Health</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        for (const auto& sub : data.subjects) {
            double sumSubPct = 0.0;
            int subStudentCount = 0;
            int subAtRiskCount = 0;
            for (const auto& p : data.profiles) {
                auto it = p.subjectWise.find(sub.id);
                if (it != p.subjectWise.end()) {
                    sumSubPct += it->second.percentage;
                    subStudentCount++;
                    if (it->second.percentage < threshold) {
                        subAtRiskCount++;
                    }
                }
            }
            double subAvg = (subStudentCount > 0) ? (sumSubPct / subStudentCount) : 0.0;

            ss << "      <tr>\n";
            ss << "        <td><span style=\"font-family:monospace; font-weight:700; color:#2563eb; background:#eff6ff; padding:3px 8px; border-radius:4px;\">" 
               << LayoutView::escapeHtml(sub.code) << "</span></td>\n";
            ss << "        <td><strong>" << LayoutView::escapeHtml(sub.name) << "</strong></td>\n";
            ss << "        <td>" << LayoutView::escapeHtml(sub.facultyName.empty() ? "Unassigned" : sub.facultyName) << "</td>\n";
            ss << "        <td>" << subStudentCount << " students</td>\n";
            ss << "        <td>" << sub.totalSessions << " sessions</td>\n";
            ss << "        <td>\n";
            ss << "          <div style=\"display:flex; align-items:center; gap:8px;\">\n";
            ss << "            <span style=\"font-weight:700; min-width:48px; color:" << (subAvg >= threshold ? "#10b981" : "#ef4444") << ";\">"
               << std::fixed << std::setprecision(1) << subAvg << "%</span>\n";
            ss << "            <div style=\"flex:1; max-width:80px; height:6px; background:#e2e8f0; border-radius:3px; overflow:hidden;\">\n";
            ss << "              <div style=\"height:100%; width:" << std::min(100.0, subAvg) << "%; background:" << (subAvg >= threshold ? "#10b981" : "#ef4444") << ";\"></div>\n";
            ss << "            </div>\n";
            ss << "          </div>\n";
            ss << "        </td>\n";
            ss << "        <td>\n";
            if (subAtRiskCount > 0) {
                ss << "          <span style=\"color:#ef4444; font-weight:600;\">" << subAtRiskCount << " students</span>\n";
            } else {
                ss << "          <span style=\"color:#10b981;\">0 students</span>\n";
            }
            ss << "        </td>\n";
            ss << "        <td>\n";
            if (subAvg >= threshold) {
                ss << "          <span class=\"badge badge-present\">Optimal</span>\n";
            } else if (subAvg >= threshold - 10.0) {
                ss << "          <span class=\"badge badge-medium\">Warning</span>\n";
            } else {
                ss << "          <span class=\"badge badge-critical\">Deficit</span>\n";
            }
            ss << "        </td>\n";
            ss << "      </tr>\n";
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        // Section 2: Department-wise Breakdown & Risk Stratification
        std::unordered_map<std::string, std::vector<attendance::models::StudentAttendanceProfile>> deptMap;
        for (const auto& p : data.profiles) {
            std::string d = p.department.empty() ? "General Academic" : p.department;
            deptMap[d].push_back(p);
        }

        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <div>\n";
        ss << "      <h3 class=\"table-title\">Departmental Engagement & Cohort Risk Clustering</h3>\n";
        ss << "      <p style=\"font-size:0.8rem; color:#64748b; margin-top:2px;\">Aggregated attendance and shortage exposure segmented by academic division</p>\n";
        ss << "    </div>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Department</th>\n";
        ss << "        <th>Cohort Size</th>\n";
        ss << "        <th>Department Average</th>\n";
        ss << "        <th>Below Threshold</th>\n";
        ss << "        <th>Critical Risk</th>\n";
        ss << "        <th>High Risk</th>\n";
        ss << "        <th>Safe Standing</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        for (const auto& [dept, pList] : deptMap) {
            double sumDeptPct = 0.0;
            int deptDeficit = 0;
            int deptCrit = 0, deptHigh = 0, deptLow = 0;
            for (const auto& p : pList) {
                sumDeptPct += p.overallPercentage;
                if (p.overallPercentage < threshold) deptDeficit++;
                if (p.riskCategory == "CRITICAL") deptCrit++;
                else if (p.riskCategory == "HIGH") deptHigh++;
                else deptLow++;
            }
            double deptAvg = pList.empty() ? 0.0 : (sumDeptPct / pList.size());

            ss << "      <tr>\n";
            ss << "        <td><strong>" << LayoutView::escapeHtml(dept) << "</strong></td>\n";
            ss << "        <td>" << pList.size() << " students</td>\n";
            ss << "        <td><strong style=\"color:" << (deptAvg >= threshold ? "#10b981" : "#ef4444") << ";\">" 
               << std::fixed << std::setprecision(1) << deptAvg << "%</strong></td>\n";
            ss << "        <td><span style=\"color:#ef4444; font-weight:600;\">" << deptDeficit << " students</span></td>\n";
            ss << "        <td>" << (deptCrit > 0 ? "<span class=\"badge badge-critical\">" + std::to_string(deptCrit) + " Critical</span>" : "<span style=\"color:#64748b;\">0</span>") << "</td>\n";
            ss << "        <td>" << (deptHigh > 0 ? "<span class=\"badge badge-high\">" + std::to_string(deptHigh) + " High</span>" : "<span style=\"color:#64748b;\">0</span>") << "</td>\n";
            ss << "        <td><span class=\"badge badge-present\">" << deptLow << " Safe</span></td>\n";
            ss << "      </tr>\n";
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        // Section 3: Risk Tier Stratification Explainer Cards
        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap:18px; margin-bottom:28px;\">\n";
        
        // Critical
        ss << "  <div style=\"padding:18px; background:#fff; border-left:4px solid #ef4444; border-radius:8px; box-shadow:0 1px 3px rgba(0,0,0,0.06);\">\n";
        ss << "    <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;\">\n";
        ss << "      <strong style=\"color:#ef4444; font-size:0.95rem;\">Critical Risk Tier</strong>\n";
        ss << "      <span class=\"badge badge-critical\">" << data.criticalRiskCount << " Students</span>\n";
        ss << "    </div>\n";
        ss << "    <p style=\"font-size:0.8rem; color:#64748b; line-height:1.4;\">Risk score &ge; 0.70. Chronic absenteeism with 3+ consecutive missed sessions. Ineligible for semester examinations without immediate administrative override.</p>\n";
        ss << "  </div>\n";

        // High
        ss << "  <div style=\"padding:18px; background:#fff; border-left:4px solid #f97316; border-radius:8px; box-shadow:0 1px 3px rgba(0,0,0,0.06);\">\n";
        ss << "    <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;\">\n";
        ss << "      <strong style=\"color:#f97316; font-size:0.95rem;\">High Risk Tier</strong>\n";
        ss << "      <span class=\"badge badge-high\">" << data.highRiskCount << " Students</span>\n";
        ss << "    </div>\n";
        ss << "    <p style=\"font-size:0.8rem; color:#64748b; line-height:1.4;\">Risk score 0.50 &ndash; 0.69. Attendance trending downward via linear regression. Requires faculty advisor consultation and targeted remediation.</p>\n";
        ss << "  </div>\n";

        // Medium
        ss << "  <div style=\"padding:18px; background:#fff; border-left:4px solid #eab308; border-radius:8px; box-shadow:0 1px 3px rgba(0,0,0,0.06);\">\n";
        ss << "    <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;\">\n";
        ss << "      <strong style=\"color:#ca8a04; font-size:0.95rem;\">Medium Risk Tier</strong>\n";
        ss << "      <span class=\"badge badge-medium\">" << data.mediumRiskCount << " Students</span>\n";
        ss << "    </div>\n";
        ss << "    <p style=\"font-size:0.8rem; color:#64748b; line-height:1.4;\">Risk score 0.30 &ndash; 0.49. Borderline standing within 2 misses of deficit threshold. Active attendance monitoring enabled.</p>\n";
        ss << "  </div>\n";

        // Low
        ss << "  <div style=\"padding:18px; background:#fff; border-left:4px solid #10b981; border-radius:8px; box-shadow:0 1px 3px rgba(0,0,0,0.06);\">\n";
        ss << "    <div style=\"display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;\">\n";
        ss << "      <strong style=\"color:#10b981; font-size:0.95rem;\">Safe Standing Tier</strong>\n";
        ss << "      <span class=\"badge badge-low\">" << data.lowRiskCount << " Students</span>\n";
        ss << "    </div>\n";
        ss << "    <p style=\"font-size:0.8rem; color:#64748b; line-height:1.4;\">Risk score &lt; 0.30. Consistently above institutional minimums with robust presence across all enrolled subjects.</p>\n";
        ss << "  </div>\n";

        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }
};

} // namespace attendance::views
