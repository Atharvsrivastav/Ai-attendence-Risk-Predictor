#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "models/Models.hpp"
#include "views/LayoutView.hpp"
#include "views/SvgCharts.hpp"

namespace attendance::views {

struct FacultyDashboardData {
    std::string facultyName;
    std::string employeeId;
    std::string department;
    std::vector<attendance::models::Subject> assignedSubjects;
    int totalAssignedStudents{0};
    int totalSessionsConducted{0};
    double averageAttendance{0.0};
    int atRiskStudentsCount{0};
    std::vector<attendance::models::AttendanceSession> recentSessions;
    std::optional<attendance::models::UndoAction> latestUndoAction;
};

class FacultyViews {
public:
    static std::string renderFacultyDashboard(
        const FacultyDashboardData& data,
        const PageContext& ctx,
        double threshold = 75.0) 
    {
        std::stringstream ss;

        // Page Header
        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Faculty Academic Dashboard</h1>\n";
        ss << "    <p class=\"page-subtitle\">" << LayoutView::escapeHtml(data.facultyName) 
           << " • ID: <strong>" << LayoutView::escapeHtml(data.employeeId) << "</strong>"
           << " • Dept: " << LayoutView::escapeHtml(data.department) << "</p>\n";
        ss << "  </div>\n";
        ss << "  <div style=\"display:flex; gap:10px;\">\n";
        ss << "    <a href=\"/faculty/mark\" class=\"btn btn-primary\">\n";
        ss << "      <svg width=\"16\" height=\"16\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 4v16m8-8H4\"/></svg>\n";
        ss << "      Mark Today's Attendance</a>\n";
        ss << "  </div>\n";
        ss << "</div>\n";

        // Persistent Undo Action Banner (if active action exists in stack)
        if (data.latestUndoAction.has_value()) {
            const auto& undo = *data.latestUndoAction;
            ss << "<div style=\"background:#ffffff; border:1px solid #cbd5e1; border-left:4px solid #2563eb; border-radius:8px; padding:16px 20px; margin-bottom:24px; display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:12px; box-shadow:0 1px 3px rgba(0,0,0,0.05);\">\n";
            ss << "  <div>\n";
            ss << "    <div style=\"font-size:0.85rem; font-weight:700; color:#1e40af; text-transform:uppercase; letter-spacing:0.04em;\">LIFO Undo Stack Active</div>\n";
            ss << "    <div style=\"font-size:0.9rem; color:#0f172a; margin-top:2px;\">\n";
            if (undo.previousStatus == attendance::models::AttendanceStatus::NONE) {
                ss << "Recent Action: Recorded attendance for <strong>" << LayoutView::escapeHtml(undo.studentName) << "</strong> in <strong>" << LayoutView::escapeHtml(undo.subjectCode) << "</strong> (Case A: Newly Inserted)";
            } else {
                ss << "Recent Action: Corrected <strong>" << LayoutView::escapeHtml(undo.studentName) << "</strong> (" << LayoutView::escapeHtml(undo.subjectCode) << ") from " 
                   << LayoutView::renderStatusBadge(undo.previousStatus) << " &rarr; " << LayoutView::renderStatusBadge(undo.newStatus) << " (Case B: Correction)";
            }
            ss << "    </div>\n";
            ss << "  </div>\n";
            ss << "  <form action=\"/faculty/undo\" method=\"POST\" style=\"margin:0;\">\n";
            if (!ctx.csrfToken.empty()) {
                ss << "    <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
            }
            ss << "    <button type=\"submit\" class=\"btn btn-secondary\" style=\"gap:6px; border-color:#93c5fd; color:#1d4ed8;\">\n";
            ss << "      <svg width=\"15\" height=\"15\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M3 10h10a8 8 0 018 8v2M3 10l6 6m-6-6l6-6\"/></svg>\n";
            ss << "      Undo Last Action</button>\n";
            ss << "  </form>\n";
            ss << "</div>\n";
        }

        // Top Metrics Grid
        ss << "<div class=\"metrics-grid\">\n";
        ss << LayoutView::renderStatCard("Assigned Courses", std::to_string(data.assignedSubjects.size()), "Teaching subjects this semester", "#2563eb");
        ss << LayoutView::renderStatCard("Total Students", std::to_string(data.totalAssignedStudents), "Enrolled in your classes", "#10b981");
        ss << LayoutView::renderStatCard("Sessions Conducted", std::to_string(data.totalSessionsConducted), "Lectures held to date", "#8b5cf6");
        
        std::stringstream ssAvg;
        ssAvg << std::fixed << std::setprecision(1) << data.averageAttendance << "%";
        ss << LayoutView::renderStatCard("Class Average", ssAvg.str(), "Across all assigned subjects", "#06b6d4");
        ss << "</div>\n";

        // Assigned Subjects Cards
        ss << "<h3 class=\"chart-title\" style=\"margin-bottom:14px;\">Assigned Courses & Subject Health</h3>\n";
        ss << "<div style=\"display:grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap:20px; margin-bottom:28px;\">\n";

        if (data.assignedSubjects.empty()) {
            ss << "<div class=\"stat-card\" style=\"grid-column: 1 / -1; text-align:center; padding:32px; color:#94a3b8;\">No course subjects currently assigned to your faculty profile. Contact administrator.</div>\n";
        } else {
            for (const auto& sub : data.assignedSubjects) {
                ss << "<div class=\"stat-card\">\n";
                ss << "  <div style=\"display:flex; justify-content:space-between; align-items:flex-start;\">\n";
                ss << "    <div>\n";
                ss << "      <span style=\"font-size:0.75rem; font-weight:700; color:#2563eb; text-transform:uppercase;\">" << LayoutView::escapeHtml(sub.code) << "</span>\n";
                ss << "      <h4 style=\"font-size:1.1rem; font-weight:700; color:#0f172a; margin-top:2px;\">" << LayoutView::escapeHtml(sub.name) << "</h4>\n";
                ss << "    </div>\n";
                ss << "    <span class=\"badge\" style=\"background:#eff6ff; color:#1d4ed8;\">" << sub.credits << " Credits</span>\n";
                ss << "  </div>\n";

                ss << "  <div style=\"margin:16px 0; display:flex; justify-content:space-between; font-size:0.85rem;\">\n";
                ss << "    <div><span style=\"color:#64748b;\">Enrolled:</span> <strong>" << sub.enrolledStudents << " students</strong></div>\n";
                ss << "    <div><span style=\"color:#64748b;\">Lectures:</span> <strong>" << sub.totalSessions << " conducted</strong></div>\n";
                ss << "  </div>\n";

                ss << "  <div style=\"display:flex; gap:8px; margin-top:auto;\">\n";
                ss << "    <a href=\"/faculty/mark?subjectId=" << sub.id << "\" class=\"btn btn-primary btn-sm\" style=\"flex:1;\">Mark Attendance</a>\n";
                ss << "    <a href=\"/faculty/sessions?subjectId=" << sub.id << "\" class=\"btn btn-secondary btn-sm\">History</a>\n";
                ss << "  </div>\n";
                ss << "</div>\n";
            }
        }
        ss << "</div>\n";

        // Recent Sessions Table
        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">Recent Attendance Sessions Conducted</h3>\n";
        ss << "    <a href=\"/faculty/sessions\" class=\"btn btn-secondary btn-sm\">View All Sessions</a>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Date</th>\n";
        ss << "        <th>Course</th>\n";
        ss << "        <th>Lecture Topic</th>\n";
        ss << "        <th>Actions</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        if (data.recentSessions.empty()) {
            ss << "      <tr><td colspan=\"4\" style=\"text-align:center; padding:24px; color:#94a3b8;\">No attendance sessions recorded yet</td></tr>\n";
        } else {
            for (const auto& ses : data.recentSessions) {
                ss << "      <tr>\n";
                ss << "        <td>" << LayoutView::escapeHtml(ses.sessionDate.substr(0, 10)) << "</td>\n";
                ss << "        <td><strong>" << LayoutView::escapeHtml(ses.subjectCode) << "</strong> - " << LayoutView::escapeHtml(ses.subjectName) << "</td>\n";
                ss << "        <td>" << LayoutView::escapeHtml(ses.topic.empty() ? "Regular Lecture" : ses.topic) << "</td>\n";
                ss << "        <td><a href=\"/faculty/sessions/" << ses.id << "\" class=\"btn btn-secondary btn-sm\">Inspect & Correct</a></td>\n";
                ss << "      </tr>\n";
            }
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Attendance Marking Form
    static std::string renderAttendanceMarkingForm(
        const std::vector<attendance::models::Subject>& assignedSubjects,
        const std::string& selectedSubjectId,
        const std::string& sessionDate,
        const std::string& topic,
        const std::vector<attendance::models::Student>& enrolledStudents,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Attendance Roll Call</h1>\n";
        ss << "    <p class=\"page-subtitle\">Select course subject, session date, and mark status for enrolled students.</p>\n";
        ss << "  </div>\n";
        ss << "  <a href=\"/faculty\" class=\"btn btn-secondary\">&larr; Back to Dashboard</a>\n";
        ss << "</div>\n";

        // Step 1: Select Subject & Date Filter Form
        ss << "<div class=\"form-card\">\n";
        ss << "  <form method=\"GET\" action=\"/faculty/mark\">\n";
        ss << "    <div class=\"form-row\">\n";
        
        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"subjectId\">Select Assigned Course</label>\n";
        ss << "        <select class=\"form-select\" id=\"subjectId\" name=\"subjectId\" onchange=\"this.form.submit()\">\n";
        for (const auto& sub : assignedSubjects) {
            bool selected = (sub.id == selectedSubjectId);
            ss << "          <option value=\"" << sub.id << "\"" << (selected ? " selected" : "") << ">"
               << LayoutView::escapeHtml(sub.code) << " — " << LayoutView::escapeHtml(sub.name) << "</option>\n";
        }
        ss << "        </select>\n";
        ss << "      </div>\n";

        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"sessionDate\">Session Date</label>\n";
        ss << "        <input type=\"date\" class=\"form-input\" id=\"sessionDate\" name=\"sessionDate\" value=\"" << LayoutView::escapeHtml(sessionDate) << "\" />\n";
        ss << "      </div>\n";

        ss << "      <div class=\"form-col\">\n";
        ss << "        <label class=\"form-label\" for=\"topic\">Lecture Topic / Module</label>\n";
        ss << "        <input type=\"text\" class=\"form-input\" id=\"topic\" name=\"topic\" value=\"" << LayoutView::escapeHtml(topic) << "\" placeholder=\"e.g. Graph Traversal Algorithms\" />\n";
        ss << "      </div>\n";

        ss << "    </div>\n";
        ss << "  </form>\n";
        ss << "</div>\n";

        // Step 2: Student Roster Form
        if (enrolledStudents.empty()) {
            ss << "<div class=\"stat-card\" style=\"text-align:center; padding:40px; color:#94a3b8;\">No enrolled students found for the selected subject.</div>\n";
        } else {
            ss << "<form action=\"/faculty/mark\" method=\"POST\">\n";
            if (!ctx.csrfToken.empty()) {
                ss << "  <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
            }
            ss << "  <input type=\"hidden\" name=\"subjectId\" value=\"" << LayoutView::escapeHtml(selectedSubjectId) << "\" />\n";
            ss << "  <input type=\"hidden\" name=\"sessionDate\" value=\"" << LayoutView::escapeHtml(sessionDate) << "\" />\n";
            ss << "  <input type=\"hidden\" name=\"topic\" value=\"" << LayoutView::escapeHtml(topic) << "\" />\n";

            ss << "  <div class=\"card-table-wrapper\">\n";
            ss << "    <div class=\"card-header-toolbar\">\n";
            ss << "      <h3 class=\"table-title\">Enrolled Student Roster (" << enrolledStudents.size() << " students)</h3>\n";
            ss << "      <div style=\"font-size:0.8rem; color:#64748b;\">Default status: <strong>PRESENT</strong></div>\n";
            ss << "    </div>\n";
            ss << "    <table class=\"data-table\">\n";
            ss << "      <thead>\n";
            ss << "        <tr>\n";
            ss << "          <th>Roll No</th>\n";
            ss << "          <th>Student Name</th>\n";
            ss << "          <th>Department</th>\n";
            ss << "          <th>Attendance Status</th>\n";
            ss << "          <th>Remarks</th>\n";
            ss << "        </tr>\n";
            ss << "      </thead>\n";
            ss << "      <tbody>\n";

            for (const auto& stu : enrolledStudents) {
                ss << "        <tr>\n";
                ss << "          <td><strong>" << LayoutView::escapeHtml(stu.rollNumber) << "</strong></td>\n";
                ss << "          <td>" << LayoutView::escapeHtml(stu.name) << "</td>\n";
                ss << "          <td>" << LayoutView::escapeHtml(stu.department) << "</td>\n";
                
                // Radio status pill options
                ss << "          <td>\n";
                ss << "            <div class=\"radio-status-group\">\n";
                ss << "              <label class=\"radio-status-label present\">\n";
                ss << "                <input type=\"radio\" name=\"status_" << stu.id << "\" value=\"PRESENT\" checked /> Present\n";
                ss << "              </label>\n";
                ss << "              <label class=\"radio-status-label absent\">\n";
                ss << "                <input type=\"radio\" name=\"status_" << stu.id << "\" value=\"ABSENT\" /> Absent\n";
                ss << "              </label>\n";
                ss << "              <label class=\"radio-status-label late\">\n";
                ss << "                <input type=\"radio\" name=\"status_" << stu.id << "\" value=\"LATE\" /> Late\n";
                ss << "              </label>\n";
                ss << "              <label class=\"radio-status-label excused\">\n";
                ss << "                <input type=\"radio\" name=\"status_" << stu.id << "\" value=\"EXCUSED\" /> Excused\n";
                ss << "              </label>\n";
                ss << "            </div>\n";
                ss << "          </td>\n";

                ss << "          <td>\n";
                ss << "            <input type=\"text\" class=\"form-input\" name=\"remarks_" << stu.id << "\" placeholder=\"Optional note\" style=\"font-size:0.8rem; padding:6px 10px;\" />\n";
                ss << "          </td>\n";

                ss << "        </tr>\n";
            }

            ss << "      </tbody>\n";
            ss << "    </table>\n";
            ss << "  </div>\n";

            // Submit Bar
            ss << "  <div style=\"display:flex; justify-content:flex-end; gap:12px; margin-top:20px;\">\n";
            ss << "    <a href=\"/faculty\" class=\"btn btn-secondary\">Cancel</a>\n";
            ss << "    <button type=\"submit\" class=\"btn btn-primary\" style=\"padding:12px 24px; font-size:1rem;\">Submit & Record Attendance</button>\n";
            ss << "  </div>\n";
            ss << "</form>\n";
        }

        return LayoutView::renderLayout(ctx, ss.str());
    }

    // Session Detail & Correction View
    static std::string renderSessionDetail(
        const attendance::models::AttendanceSession& session,
        const std::vector<attendance::models::AttendanceRecord>& records,
        const PageContext& ctx) 
    {
        std::stringstream ss;

        ss << "<div class=\"page-header\">\n";
        ss << "  <div>\n";
        ss << "    <h1 class=\"page-title\">Session Details & Correction</h1>\n";
        ss << "    <p class=\"page-subtitle\">" << LayoutView::escapeHtml(session.subjectCode) << " • Date: " << session.sessionDate.substr(0, 10) 
           << " • Topic: " << LayoutView::escapeHtml(session.topic.empty() ? "Regular Lecture" : session.topic) << "</p>\n";
        ss << "  </div>\n";
        ss << "  <a href=\"/faculty/sessions\" class=\"btn btn-secondary\">&larr; Back to Sessions</a>\n";
        ss << "</div>\n";

        ss << "<div class=\"card-table-wrapper\">\n";
        ss << "  <div class=\"card-header-toolbar\">\n";
        ss << "    <h3 class=\"table-title\">Recorded Student Attendance</h3>\n";
        ss << "    <span style=\"font-size:0.8rem; color:#64748b;\">" << records.size() << " student records</span>\n";
        ss << "  </div>\n";
        ss << "  <table class=\"data-table\">\n";
        ss << "    <thead>\n";
        ss << "      <tr>\n";
        ss << "        <th>Student Name</th>\n";
        ss << "        <th>Current Status</th>\n";
        ss << "        <th>Remarks</th>\n";
        ss << "        <th>Correction Action</th>\n";
        ss << "      </tr>\n";
        ss << "    </thead>\n";
        ss << "    <tbody>\n";

        for (const auto& rec : records) {
            ss << "      <tr>\n";
            ss << "        <td><strong>" << LayoutView::escapeHtml(rec.studentId) << "</strong></td>\n";
            ss << "        <td>" << LayoutView::renderStatusBadge(rec.status) << "</td>\n";
            ss << "        <td style=\"color:#64748b;\">" << LayoutView::escapeHtml(rec.remarks) << "</td>\n";
            
            // Inline Correction Form
            ss << "        <td>\n";
            ss << "          <form action=\"/faculty/sessions/" << session.id << "/update\" method=\"POST\" style=\"display:flex; gap:6px; margin:0; align-items:center;\">\n";
            if (!ctx.csrfToken.empty()) {
                ss << "            <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(ctx.csrfToken) << "\" />\n";
            }
            ss << "            <input type=\"hidden\" name=\"recordId\" value=\"" << rec.id << "\" />\n";
            ss << "            <select name=\"status\" class=\"form-select\" style=\"padding:4px 8px; font-size:0.8rem; width:110px;\">\n";
            ss << "              <option value=\"PRESENT\"" << (rec.status == attendance::models::AttendanceStatus::PRESENT ? " selected" : "") << ">Present</option>\n";
            ss << "              <option value=\"ABSENT\"" << (rec.status == attendance::models::AttendanceStatus::ABSENT ? " selected" : "") << ">Absent</option>\n";
            ss << "              <option value=\"LATE\"" << (rec.status == attendance::models::AttendanceStatus::LATE ? " selected" : "") << ">Late</option>\n";
            ss << "              <option value=\"EXCUSED\"" << (rec.status == attendance::models::AttendanceStatus::EXCUSED ? " selected" : "") << ">Excused</option>\n";
            ss << "            </select>\n";
            ss << "            <button type=\"submit\" class=\"btn btn-secondary btn-sm\">Save</button>\n";
            ss << "          </form>\n";
            ss << "        </td>\n";

            ss << "      </tr>\n";
        }

        ss << "    </tbody>\n";
        ss << "  </table>\n";
        ss << "</div>\n";

        return LayoutView::renderLayout(ctx, ss.str());
    }
};

} // namespace attendance::views
