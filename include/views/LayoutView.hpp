#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "models/Models.hpp"

namespace attendance::views {

struct BreadcrumbItem {
    std::string label;
    std::string url;
};

struct PageContext {
    std::string pageTitle{"Dashboard"};
    std::string activeNav{"overview"};
    std::string userName{"User"};
    std::string userEmail;
    attendance::models::UserRole userRole{attendance::models::UserRole::STUDENT};
    std::string studentId;
    std::string facultyId;
    std::vector<BreadcrumbItem> breadcrumbs;
    std::string flashSuccess;
    std::string flashError;
    std::string flashWarning;
    std::string flashInfo;
    std::string csrfToken;
};

class LayoutView {
public:
    static std::string escapeHtml(const std::string& input) {
        std::stringstream ss;
        for (char c : input) {
            switch (c) {
                case '&':  ss << "&amp;"; break;
                case '<':  ss << "&lt;"; break;
                case '>':  ss << "&gt;"; break;
                case '"':  ss << "&quot;"; break;
                case '\'': ss << "&#39;"; break;
                default:   ss << c; break;
            }
        }
        return ss.str();
    }

    static std::string renderStatusBadge(attendance::models::AttendanceStatus status) {
        switch (status) {
            case attendance::models::AttendanceStatus::PRESENT:
                return "<span class=\"badge badge-present\">Present</span>";
            case attendance::models::AttendanceStatus::ABSENT:
                return "<span class=\"badge badge-absent\">Absent</span>";
            case attendance::models::AttendanceStatus::LATE:
                return "<span class=\"badge badge-late\">Late</span>";
            case attendance::models::AttendanceStatus::EXCUSED:
                return "<span class=\"badge badge-excused\">Excused</span>";
            case attendance::models::AttendanceStatus::NONE:
                return "<span class=\"badge\" style=\"background:#e2e8f0; color:#475569;\">None</span>";
        }
        return "<span class=\"badge badge-present\">Present</span>";
    }

    static std::string renderRiskBadge(attendance::models::RiskLevel risk) {
        switch (risk) {
            case attendance::models::RiskLevel::LOW:
                return "<span class=\"badge badge-low\">Low Risk</span>";
            case attendance::models::RiskLevel::MEDIUM:
                return "<span class=\"badge badge-medium\">Medium Risk</span>";
            case attendance::models::RiskLevel::HIGH:
                return "<span class=\"badge badge-high\">High Risk</span>";
            case attendance::models::RiskLevel::CRITICAL:
                return "<span class=\"badge badge-critical\">Critical Risk</span>";
        }
        return "<span class=\"badge badge-low\">Low Risk</span>";
    }

    static std::string renderStatCard(
        const std::string& title,
        const std::string& value,
        const std::string& subtext,
        const std::string& iconColor = "#2563eb",
        const std::string& sparklineSvg = "") 
    {
        std::stringstream ss;
        ss << "<div class=\"stat-card\">";
        ss << "<div class=\"stat-header\">";
        ss << "<span class=\"stat-title\">" << escapeHtml(title) << "</span>";
        if (!sparklineSvg.empty()) {
            ss << sparklineSvg;
        } else {
            ss << "<div class=\"stat-icon\" style=\"color:" << iconColor << "; background:" << iconColor << "15;\">";
            ss << "<svg width=\"20\" height=\"20\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\">"
               << "<path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z\"></path></svg>";
            ss << "</div>";
        }
        ss << "</div>";
        ss << "<div class=\"stat-value\">" << escapeHtml(value) << "</div>";
        if (!subtext.empty()) {
            ss << "<div class=\"stat-subtext\">" << escapeHtml(subtext) << "</div>";
        }
        ss << "</div>";
        return ss.str();
    }

    static std::string renderSidebar(const PageContext& ctx) {
        std::stringstream ss;
        ss << "<aside class=\"sidebar\">";
        
        // Brand Header
        ss << "<div class=\"sidebar-header\">";
        ss << "<div class=\"sidebar-brand-icon\">AI</div>";
        ss << "<div>";
        ss << "<div class=\"sidebar-brand-title\">Attendance Risk</div>";
        ss << "<div class=\"sidebar-brand-subtitle\">Enterprise Predictor</div>";
        ss << "</div>";
        ss << "</div>";

        // Navigation Links
        ss << "<nav class=\"sidebar-nav\">";

        if (ctx.userRole == attendance::models::UserRole::ADMIN) {
            ss << "<div class=\"nav-section-title\">Institutional Analytics</div>";
            ss << "<a href=\"/admin\" class=\"nav-item " << (ctx.activeNav == "admin-overview" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6\"/></svg>"
               << "Admin Overview</a>";
            ss << "<a href=\"/admin/analytics\" class=\"nav-item " << (ctx.activeNav == "admin-analytics" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z\"/></svg>"
               << "Deep Analytics</a>";

            ss << "<div class=\"nav-section-title\">Management</div>";
            ss << "<a href=\"/admin/students\" class=\"nav-item " << (ctx.activeNav == "admin-students" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 4.354a4 4 0 110 5.292M15 21H3v-1a6 6 0 0112 0v1zm0 0h6v-1a6 6 0 00-9-5.197M13 7a4 4 0 11-8 0 4 4 0 018 0z\"/></svg>"
               << "Student Directory</a>";
            ss << "<a href=\"/admin/faculty\" class=\"nav-item " << (ctx.activeNav == "admin-faculty" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M21 13.255A23.931 23.931 0 0112 15c-3.183 0-6.22-.62-9-1.745M16 6V4a2 2 0 00-2-2h-4a2 2 0 00-2 2v2m4 6h.01M5 20h14a2 2 0 002-2V8a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z\"/></svg>"
               << "Faculty Staff</a>";
            ss << "<a href=\"/admin/subjects\" class=\"nav-item " << (ctx.activeNav == "admin-subjects" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253\"/></svg>"
               << "Course Subjects</a>";
            ss << "<a href=\"/admin/system\" class=\"nav-item " << (ctx.activeNav == "admin-system" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z\"/><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M15 12a3 3 0 11-6 0 3 3 0 016 0z\"/></svg>"
               << "System & Audit</a>";
        } else if (ctx.userRole == attendance::models::UserRole::FACULTY) {
            ss << "<div class=\"nav-section-title\">Faculty Portal</div>";
            ss << "<a href=\"/faculty\" class=\"nav-item " << (ctx.activeNav == "faculty-overview" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6\"/></svg>"
               << "Assigned Classes</a>";
            ss << "<a href=\"/faculty/mark\" class=\"nav-item " << (ctx.activeNav == "faculty-mark" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2m-6 9l2 2 4-4\"/></svg>"
               << "Mark Attendance</a>";
            ss << "<a href=\"/faculty/sessions\" class=\"nav-item " << (ctx.activeNav == "faculty-sessions" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z\"/></svg>"
               << "Session History</a>";
        } else {
            ss << "<div class=\"nav-section-title\">Academic Portal</div>";
            ss << "<a href=\"/student\" class=\"nav-item " << (ctx.activeNav == "student-dashboard" ? "active" : "") << "\">"
               << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z\"/></svg>"
               << "My Attendance Profile</a>";
        }

        // Shared Labs & Tools
        ss << "<div class=\"nav-section-title\">Tools & Benchmarks</div>";
        ss << "<a href=\"/dsa-lab\" class=\"nav-item " << (ctx.activeNav == "dsa-lab" ? "active" : "") << "\">"
           << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M19.428 15.428a2 2 0 00-1.022-.547l-2.387-.477a6 6 0 00-3.86.517l-.318.158a6 6 0 01-3.86.517L6.05 15.21a2 2 0 00-1.806.547M8 4h8l-1 1v5.172a2 2 0 00.586 1.414l5 5c1.26 1.26.367 3.414-1.415 3.414H4.828c-1.782 0-2.674-2.154-1.414-3.414l5-5A2 2 0 009 10.172V5L8 4z\"/></svg>"
           << "DSA Visualizer Lab</a>";
        ss << "<a href=\"/reports\" class=\"nav-item " << (ctx.activeNav == "reports" ? "active" : "") << "\">"
           << "<svg fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M9 17v-2m3 2v-4m3 4v-6m2 10H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z\"/></svg>"
           << "Export Reports</a>";

        ss << "</nav>";

        // Footer User Profile & Logout
        std::string roleLabel = (ctx.userRole == attendance::models::UserRole::ADMIN ? "Administrator" : 
                                (ctx.userRole == attendance::models::UserRole::FACULTY ? "Faculty Professor" : "Student"));
        std::string initial = ctx.userName.empty() ? "U" : ctx.userName.substr(0, 1);

        ss << "<div class=\"sidebar-footer\">";
        ss << "<div class=\"user-profile-badge\">";
        ss << "<div class=\"user-avatar\">" << escapeHtml(initial) << "</div>";
        ss << "<div class=\"user-meta\">";
        ss << "<span class=\"user-name\">" << escapeHtml(ctx.userName) << "</span>";
        ss << "<span class=\"user-role-tag\">" << roleLabel << "</span>";
        ss << "</div>";
        ss << "</div>";
        ss << "<a href=\"/logout\" class=\"logout-btn\" title=\"Sign out\">";
        ss << "<svg width=\"18\" height=\"18\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M17 16l4-4m0 0l-4-4m4 4H7m6 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h4a3 3 0 013 3v1\"/></svg>";
        ss << "</a>";
        ss << "</div>";

        ss << "</aside>";
        return ss.str();
    }

    static std::string renderHeader(const PageContext& ctx) {
        std::stringstream ss;
        ss << "<header class=\"top-header\">";
        
        // Breadcrumbs
        ss << "<div class=\"breadcrumbs\">";
        ss << "<a href=\"/\">Home</a>";
        for (const auto& item : ctx.breadcrumbs) {
            ss << "<span class=\"separator\">/</span>";
            if (!item.url.empty()) {
                ss << "<a href=\"" << item.url << "\">" << escapeHtml(item.label) << "</a>";
            } else {
                ss << "<span class=\"current\">" << escapeHtml(item.label) << "</span>";
            }
        }
        ss << "</div>";

        // Institutional Badge & Quick Links
        ss << "<div class=\"header-actions\">";
        ss << "<span style=\"display:inline-flex; align-items:center; gap:6px; font-size:0.8rem; font-weight:600; color:#475569; background:#f1f5f9; padding:4px 10px; border-radius:9999px;\">"
           << "<span style=\"width:8px; height:8px; border-radius:50%; background:#10b981;\"></span> C++20 Core Active</span>";
        ss << "<a href=\"/reports\" class=\"btn btn-secondary btn-sm\" style=\"gap:6px;\">"
           << "<svg width=\"14\" height=\"14\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4\"/></svg>"
           << "Export Data</a>";
        ss << "</div>";

        ss << "</header>";
        return ss.str();
    }

    static std::string renderAlerts(const PageContext& ctx) {
        std::stringstream ss;
        if (!ctx.flashSuccess.empty()) {
            ss << "<div class=\"alert-banner alert-success\">"
               << "<svg width=\"18\" height=\"18\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z\"/></svg>"
               << "<span>" << escapeHtml(ctx.flashSuccess) << "</span></div>";
        }
        if (!ctx.flashError.empty()) {
            ss << "<div class=\"alert-banner alert-danger\">"
               << "<svg width=\"18\" height=\"18\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"/></svg>"
               << "<span>" << escapeHtml(ctx.flashError) << "</span></div>";
        }
        if (!ctx.flashWarning.empty()) {
            ss << "<div class=\"alert-banner alert-warning\">"
               << "<svg width=\"18\" height=\"18\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z\"/></svg>"
               << "<span>" << escapeHtml(ctx.flashWarning) << "</span></div>";
        }
        if (!ctx.flashInfo.empty()) {
            ss << "<div class=\"alert-banner alert-info\">"
               << "<svg width=\"18\" height=\"18\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" viewBox=\"0 0 24 24\"><path stroke-linecap=\"round\" stroke-linejoin=\"round\" d=\"M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z\"/></svg>"
               << "<span>" << escapeHtml(ctx.flashInfo) << "</span></div>";
        }
        return ss.str();
    }

    static std::string renderLayout(const PageContext& ctx, const std::string& bodyHtml) {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n";
        ss << "<html lang=\"en\">\n";
        ss << "<head>\n";
        ss << "  <meta charset=\"UTF-8\" />\n";
        ss << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n";
        ss << "  <title>" << escapeHtml(ctx.pageTitle) << " — AI Attendance Risk Predictor</title>\n";
        ss << "  <link rel=\"stylesheet\" href=\"/public/css/style.css\" />\n";
        ss << "</head>\n";
        ss << "<body>\n";
        ss << "  <div class=\"app-container\">\n";
        
        // Sidebar
        ss << renderSidebar(ctx);

        // Main Wrapper
        ss << "    <div class=\"main-wrapper\">\n";
        ss << renderHeader(ctx);
        ss << "      <main class=\"main-content\">\n";
        ss << renderAlerts(ctx);
        ss << bodyHtml;
        ss << "      </main>\n";
        ss << "    </div>\n";

        ss << "  </div>\n";
        ss << "</body>\n";
        ss << "</html>\n";
        return ss.str();
    }
};

} // namespace attendance::views
