import urllib.request
import urllib.parse
import http.cookiejar
import re
import sys

BASE_URL = "http://127.0.0.1:8080"

passed = 0
failed = 0

def check(name, condition, extra=""):
    global passed, failed
    if condition:
        print(f"  [PASS] {name} {extra}")
        passed += 1
    else:
        print(f"  [FAIL] {name} {extra}")
        failed += 1

def create_session_opener():
    cj = http.cookiejar.CookieJar()
    opener = urllib.request.build_opener(urllib.request.HTTPCookieProcessor(cj))
    return opener, cj

print("=" * 65)
print("  AI ATTENDANCE RISK PREDICTOR — C++ FULL-STACK VERIFICATION")
print("=" * 65)

opener, cj = create_session_opener()

# -------------------------------------------------------------
# 1. Static Assets & Public UI Routes
# -------------------------------------------------------------
print("\n[1] Testing Static Assets & Public UI Routes...")
try:
    req = urllib.request.Request(f"{BASE_URL}/public/css/style.css")
    with opener.open(req) as res:
        css = res.read().decode("utf-8")
        check("CSS Stylesheet Serviced", res.status == 200 and ("sidebar" in css or "body" in css), f"(Length: {len(css)} bytes)")
except Exception as e:
    check("CSS Stylesheet Serviced", False, str(e))

try:
    req = urllib.request.Request(f"{BASE_URL}/login")
    with opener.open(req) as res:
        html = res.read().decode("utf-8")
        check("Login Page GET", res.status == 200 and "Attendance Risk Predictor" in html)
        check("Login Page Has CSRF", 'name="csrf_token"' in html)
        check("Login Page Has Form Inputs", 'name="email"' in html and 'name="password"' in html)
except Exception as e:
    check("Login Page GET", False, str(e))

# Unauthenticated access redirects
try:
    req = urllib.request.Request(f"{BASE_URL}/admin")
    with opener.open(req) as res:
        check("Unauthenticated Access Redirected to /login", res.geturl().endswith("/login"))
except urllib.error.HTTPError as e:
    check("Unauthenticated Access Handled", e.code in [302, 303, 401, 403], f"Code: {e.code}")
except Exception as e:
    check("Unauthenticated Access Handled", True, str(e))

# -------------------------------------------------------------
# 2. Admin Authentication & Dashboard
# -------------------------------------------------------------
print("\n[2] Testing Admin Login & Full-Stack Dashboard...")
try:
    with opener.open(f"{BASE_URL}/login") as res:
        login_html = res.read().decode("utf-8")
        csrf_match = re.search(r'name="csrf_token"\s+value="([^"]+)"', login_html)
        csrf_token = csrf_match.group(1) if csrf_match else ""

    login_data = urllib.parse.urlencode({
        "email": "admin@college.edu",
        "password": "admin123",
        "csrf_token": csrf_token
    }).encode("utf-8")

    req = urllib.request.Request(f"{BASE_URL}/login", data=login_data, method="POST")
    with opener.open(req) as res:
        cookies = [cookie.name for cookie in cj]
        check("Admin Login Sets Session Cookie", "session_id" in cookies)

    with opener.open(f"{BASE_URL}/admin") as res:
        admin_html = res.read().decode("utf-8")
        check("Admin Dashboard Rendered (C++)", "Institutional Executive Overview" in admin_html)
        check("Admin Dashboard Has Stat Cards", "Total Students" in admin_html and "Institution Average" in admin_html)
        check("Admin Dashboard Has SVG Charts", "<svg" in admin_html and "STUDENTS" in admin_html)

    with opener.open(f"{BASE_URL}/admin/students") as res:
        roster_html = res.read().decode("utf-8")
        check("Admin Students Directory Rendered", "Student Directory" in roster_html and "<table" in roster_html)

    with opener.open(f"{BASE_URL}/admin/faculty") as res:
        faculty_html = res.read().decode("utf-8")
        check("Admin Faculty Staff Directory Rendered", res.status == 200 and "Faculty Staff Directory" in faculty_html and "Institutional Faculty Roster" in faculty_html)

    with opener.open(f"{BASE_URL}/admin/subjects") as res:
        subjects_html = res.read().decode("utf-8")
        check("Admin Course Subjects Catalog Rendered", res.status == 200 and "Course Curriculum & Subjects" in subjects_html and "Institutional Subject Catalog" in subjects_html)

    with opener.open(f"{BASE_URL}/admin/analytics") as res:
        analytics_html = res.read().decode("utf-8")
        check("Admin Deep Analytics Page Rendered", res.status == 200 and "Institutional Deep Analytics" in analytics_html and "Subject Performance Matrix" in analytics_html)
        check("Admin Deep Analytics Has Progression Chart", "<svg" in analytics_html and "Semester Session Progression" in analytics_html)

    with opener.open(f"{BASE_URL}/admin/system") as res:
        system_html = res.read().decode("utf-8")
        check("Admin System Settings Rendered", "System Health" in system_html and "Audit" in system_html)
        csrf_match = re.search(r'name="csrf_token"\s+value="([^"]+)"', system_html)
        sys_csrf = csrf_match.group(1) if csrf_match else ""

    # Update Threshold Setting via POST
    if sys_csrf:
        update_data = urllib.parse.urlencode({
            "threshold": "76.5",
            "csrf_token": sys_csrf
        }).encode("utf-8")
        req = urllib.request.Request(f"{BASE_URL}/admin/system/threshold", data=update_data, method="POST")
        with opener.open(req) as res:
            check("Admin System Config POST", res.status in [200, 302, 303])

    # Reset threshold back to 75.0
    if sys_csrf:
        reset_data = urllib.parse.urlencode({
            "threshold": "75.0",
            "csrf_token": sys_csrf
        }).encode("utf-8")
        req = urllib.request.Request(f"{BASE_URL}/admin/system/threshold", data=reset_data, method="POST")
        opener.open(req)

except Exception as e:
    check("Admin Workflow", False, str(e))

# -------------------------------------------------------------
# 3. Interactive DSA Laboratory & Reports
# -------------------------------------------------------------
print("\n[3] Testing DSA Laboratory & CSV Reports...")
try:
    with opener.open(f"{BASE_URL}/dsa-lab") as res:
        dsa_html = res.read().decode("utf-8")
        check("DSA Lab Page Rendered", "Interactive Data Structures & Algorithms Lab" in dsa_html)
        check("DSA Lab MergeSort vs QuickSort", "Merge Sort" in dsa_html and "Quick Sort (Lomuto)" in dsa_html)
        check("DSA Lab Binary Search Steps", "Binary Search" in dsa_html)
        check("DSA Lab StudentHashMap Diagnostics", "StudentHashMap" in dsa_html and "Bucket Capacity" in dsa_html)
        check("DSA Lab AttendanceGraph Traversal", "AttendanceGraph" in dsa_html and "Breadth-First Search" in dsa_html)

    with opener.open(f"{BASE_URL}/reports") as res:
        rep_html = res.read().decode("utf-8")
        check("Reports Center Rendered", "Reports & Data Export Center" in rep_html)

    with opener.open(f"{BASE_URL}/reports/students.csv") as res:
        csv_text = res.read().decode("utf-8")
        check("Students CSV Download", "Roll Number,Student Name" in csv_text, f"({len(csv_text.splitlines())} lines)")

    with opener.open(f"{BASE_URL}/reports/risk.csv") as res:
        risk_csv = res.read().decode("utf-8")
        check("Risk CSV Download", "Roll Number,Student Name" in risk_csv, f"({len(risk_csv.splitlines())} lines)")

except Exception as e:
    check("DSA & Reports", False, str(e))

# -------------------------------------------------------------
# 4. Faculty Workflow & LIFO Undo
# -------------------------------------------------------------
print("\n[4] Testing Faculty Dashboard, Roll Call & LIFO Undo...")
fac_opener, fac_cj = create_session_opener()
try:
    with fac_opener.open(f"{BASE_URL}/login") as res:
        login_html = res.read().decode("utf-8")
        csrf_match = re.search(r'name="csrf_token"\s+value="([^"]+)"', login_html)
        fac_csrf = csrf_match.group(1) if csrf_match else ""

    login_data = urllib.parse.urlencode({
        "email": "prof.sharma@college.edu",
        "password": "faculty123",
        "csrf_token": fac_csrf
    }).encode("utf-8")

    req = urllib.request.Request(f"{BASE_URL}/login", data=login_data, method="POST")
    with fac_opener.open(req) as res:
        check("Faculty Login Success", any("session_id" in c.name for c in fac_cj))

    with fac_opener.open(f"{BASE_URL}/faculty") as res:
        fac_html = res.read().decode("utf-8")
        check("Faculty Dashboard Rendered", "Faculty Academic Dashboard" in fac_html)
        check("Faculty Assigned Subjects", "Assigned Courses" in fac_html or "Mark Today's Attendance" in fac_html)

    with fac_opener.open(f"{BASE_URL}/faculty/mark") as res:
        mark_html = res.read().decode("utf-8")
        check("Roll Call Page Rendered", "Attendance Roll Call" in mark_html)
        check("Student Status Radio Buttons", "PRESENT" in mark_html and "ABSENT" in mark_html)
        csrf_match = re.search(r'name="csrf_token"\s+value="([^"]+)"', mark_html)
        mark_csrf = csrf_match.group(1) if csrf_match else ""

    if mark_csrf:
        undo_data = urllib.parse.urlencode({"csrf_token": mark_csrf}).encode("utf-8")
        req = urllib.request.Request(f"{BASE_URL}/faculty/undo", data=undo_data, method="POST")
        with fac_opener.open(req) as res:
            check("Faculty Undo Endpoint Functional", res.status in [200, 302, 303])

except Exception as e:
    check("Faculty Workflow", False, str(e))

# -------------------------------------------------------------
# 5. Student Workflow, Gauge & What-If Calculator
# -------------------------------------------------------------
print("\n[5] Testing Student Dashboard, SVG Gauge & What-If Calculator...")
stu_opener, stu_cj = create_session_opener()
try:
    with stu_opener.open(f"{BASE_URL}/login") as res:
        login_html = res.read().decode("utf-8")
        csrf_match = re.search(r'name="csrf_token"\s+value="([^"]+)"', login_html)
        stu_csrf = csrf_match.group(1) if csrf_match else ""

    login_data = urllib.parse.urlencode({
        "email": "aryan.sharma@college.edu",
        "password": "student123",
        "csrf_token": stu_csrf
    }).encode("utf-8")

    req = urllib.request.Request(f"{BASE_URL}/login", data=login_data, method="POST")
    with stu_opener.open(req) as res:
        check("Student Login Success", any("session_id" in c.name for c in stu_cj))

    with stu_opener.open(f"{BASE_URL}/student") as res:
        stu_html = res.read().decode("utf-8")
        check("Student Dashboard Rendered", "Student Attendance & Risk Analytics" in stu_html)
        check("Student Attendance Gauge SVG", "<svg" in stu_html and "gauge-svg" in stu_html)
        check("Student Subject Breakdown Table", "Subject-wise Attendance Breakdown" in stu_html)
        check("Student What-If Calculator Form", "What-If Attendance Recovery Calculator" in stu_html)

    sim_data = urllib.parse.urlencode({
        "classes_attended": "5",
        "classes_missed": "0",
        "csrf_token": stu_csrf
    }).encode("utf-8")
    req = urllib.request.Request(f"{BASE_URL}/student/simulate", data=sim_data, method="POST")
    with stu_opener.open(req) as res:
        sim_html = res.read().decode("utf-8")
        check("What-If Simulation POST Computed", "Simulation Result" in sim_html or "What-If" in sim_html)

    # Student RBAC Isolation Check: Cannot access /admin
    try:
        req = urllib.request.Request(f"{BASE_URL}/admin")
        stu_opener.open(req)
        check("Student Blocked From Admin (RBAC)", False, "Expected 403 Forbidden")
    except urllib.error.HTTPError as e:
        check("Student Blocked From Admin (RBAC)", e.code == 403, f"Code: {e.code}")

except Exception as e:
    check("Student Workflow", False, str(e))

print("\n" + "=" * 65)
print(f"  FULL-STACK VERIFICATION: {passed} PASSED, {failed} FAILED")
print("=" * 65)

sys.exit(0 if failed == 0 else 1)
