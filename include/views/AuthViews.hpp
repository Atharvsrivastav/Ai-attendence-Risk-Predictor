#pragma once

#include <string>
#include <sstream>
#include "views/LayoutView.hpp"

namespace attendance::views {

class AuthViews {
public:
    static std::string renderLoginPage(
        const std::string& errorMessage = "",
        const std::string& successMessage = "",
        const std::string& csrfToken = "") 
    {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n";
        ss << "<html lang=\"en\">\n";
        ss << "<head>\n";
        ss << "  <meta charset=\"UTF-8\" />\n";
        ss << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n";
        ss << "  <title>Sign In — AI Attendance Risk Predictor</title>\n";
        ss << "  <link rel=\"stylesheet\" href=\"/public/css/style.css\" />\n";
        ss << "</head>\n";
        ss << "<body>\n";
        ss << "  <div class=\"auth-page\">\n";
        ss << "    <div class=\"auth-card\">\n";
        
        // Brand
        ss << "      <div class=\"auth-brand\">\n";
        ss << "        <div class=\"auth-brand-icon\">AI</div>\n";
        ss << "        <h1 class=\"auth-title\">Attendance Risk Predictor</h1>\n";
        ss << "        <p class=\"auth-subtitle\">Sign in to your institutional account</p>\n";
        ss << "      </div>\n";

        // Errors or Alerts
        if (!errorMessage.empty()) {
            ss << "      <div class=\"alert-banner alert-danger\" style=\"margin-bottom:20px;\">"
               << LayoutView::escapeHtml(errorMessage) << "</div>\n";
        }
        if (!successMessage.empty()) {
            ss << "      <div class=\"alert-banner alert-success\" style=\"margin-bottom:20px;\">"
               << LayoutView::escapeHtml(successMessage) << "</div>\n";
        }

        // Form
        ss << "      <form action=\"/login\" method=\"POST\">\n";
        if (!csrfToken.empty()) {
            ss << "        <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(csrfToken) << "\" />\n";
        }
        ss << "        <div class=\"form-group\">\n";
        ss << "          <label class=\"form-label\" for=\"email\">Institutional Email</label>\n";
        ss << "          <input class=\"form-input\" type=\"email\" id=\"email\" name=\"email\" placeholder=\"e.g. admin@college.edu\" required autofocus />\n";
        ss << "        </div>\n";

        ss << "        <div class=\"form-group\">\n";
        ss << "          <label class=\"form-label\" for=\"password\">Password</label>\n";
        ss << "          <input class=\"form-input\" type=\"password\" id=\"password\" name=\"password\" placeholder=\"••••••••\" required />\n";
        ss << "        </div>\n";

        ss << "        <button type=\"submit\" class=\"btn btn-primary\" style=\"width:100%; padding:12px; margin-top:8px;\">Sign In</button>\n";
        ss << "      </form>\n";

        // First time account setup link
        ss << "      <div style=\"text-align:center; margin-top:16px; font-size:0.85rem;\">"
           << "<a href=\"/setup-password\" style=\"color:#2563eb; text-decoration:none; font-weight:600;\">First time student? Activate Account & Set Password &rarr;</a>"
           << "</div>\n";

        // Demo credentials box
        ss << "      <div class=\"demo-credentials-box\">\n";
        ss << "        <h4>Demo Credentials (Development & Testing)</h4>\n";
        ss << "        <table style=\"width:100%; border-collapse:collapse; font-size:0.75rem;\">\n";
        ss << "          <tr><td style=\"color:#64748b;\">Admin:</td><td><code>admin@college.edu</code></td><td><code>admin123</code></td></tr>\n";
        ss << "          <tr><td style=\"color:#64748b;\">Faculty:</td><td><code>prof.sharma@college.edu</code></td><td><code>faculty123</code></td></tr>\n";
        ss << "          <tr><td style=\"color:#64748b;\">Student:</td><td><code>aryan.sharma@college.edu</code></td><td><code>student123</code></td></tr>\n";
        ss << "        </table>\n";
        ss << "      </div>\n";

        ss << "    </div>\n";
        ss << "  </div>\n";
        ss << "</body>\n";
        ss << "</html>\n";
        return ss.str();
    }

    static std::string renderSetupPasswordPage(
        const std::string& token = "",
        const std::string& email = "",
        const std::string& errorMessage = "",
        const std::string& csrfToken = "") 
    {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n";
        ss << "<html lang=\"en\">\n";
        ss << "<head>\n";
        ss << "  <meta charset=\"UTF-8\" />\n";
        ss << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n";
        ss << "  <title>Activate Account — AI Attendance Risk Predictor</title>\n";
        ss << "  <link rel=\"stylesheet\" href=\"/public/css/style.css\" />\n";
        ss << "</head>\n";
        ss << "<body>\n";
        ss << "  <div class=\"auth-page\">\n";
        ss << "    <div class=\"auth-card\">\n";
        
        ss << "      <div class=\"auth-brand\">\n";
        ss << "        <div class=\"auth-brand-icon\">AI</div>\n";
        ss << "        <h1 class=\"auth-title\">Student Account Setup</h1>\n";
        ss << "        <p class=\"auth-subtitle\">Activate your account with your one-time setup token</p>\n";
        ss << "      </div>\n";

        if (!errorMessage.empty()) {
            ss << "      <div class=\"alert-banner alert-danger\" style=\"margin-bottom:20px;\">"
               << LayoutView::escapeHtml(errorMessage) << "</div>\n";
        }

        ss << "      <form action=\"/setup-password\" method=\"POST\">\n";
        if (!csrfToken.empty()) {
            ss << "        <input type=\"hidden\" name=\"csrf_token\" value=\"" << LayoutView::escapeHtml(csrfToken) << "\" />\n";
        }
        ss << "        <div class=\"form-group\">\n";
        ss << "          <label class=\"form-label\" for=\"token\">One-Time Setup Token (64-char Hex)</label>\n";
        ss << "          <input class=\"form-input\" type=\"text\" id=\"token\" name=\"token\" value=\"" << LayoutView::escapeHtml(token) << "\" placeholder=\"Paste setup token here\" required />\n";
        ss << "        </div>\n";

        ss << "        <div class=\"form-group\">\n";
        ss << "          <label class=\"form-label\" for=\"newPassword\">Create New Password (min 8 chars)</label>\n";
        ss << "          <input class=\"form-input\" type=\"password\" id=\"newPassword\" name=\"newPassword\" placeholder=\"Enter strong password\" minlength=\"8\" required />\n";
        ss << "        </div>\n";

        ss << "        <div class=\"form-group\">\n";
        ss << "          <label class=\"form-label\" for=\"confirmPassword\">Confirm New Password</label>\n";
        ss << "          <input class=\"form-input\" type=\"password\" id=\"confirmPassword\" name=\"confirmPassword\" placeholder=\"Confirm password\" minlength=\"8\" required />\n";
        ss << "        </div>\n";

        ss << "        <button type=\"submit\" class=\"btn btn-primary\" style=\"width:100%; padding:12px; margin-top:8px;\">Save Password & Activate</button>\n";
        ss << "      </form>\n";

        ss << "      <div style=\"text-align:center; margin-top:20px; font-size:0.85rem;\">"
           << "<a href=\"/login\" style=\"color:#64748b; text-decoration:none;\">&larr; Back to Sign In</a>"
           << "</div>\n";

        ss << "    </div>\n";
        ss << "  </div>\n";
        ss << "</body>\n";
        ss << "</html>\n";
        return ss.str();
    }
};

} // namespace attendance::views
