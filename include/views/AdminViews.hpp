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
};

} // namespace attendance::views
