import urllib.request
import urllib.error
import json
import base64
import time
import sys

BASE_URL = "http://127.0.0.1:8080"

def make_req(path, method="GET", payload=None, token=None):
    url = f"{BASE_URL}{path}"
    data = json.dumps(payload).encode('utf-8') if payload is not None else None
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req) as resp:
            content = resp.read().decode('utf-8')
            return resp.status, json.loads(content) if content else {}
    except urllib.error.HTTPError as e:
        content = e.read().decode('utf-8')
        try:
            return e.code, json.loads(content) if content else {}
        except Exception:
            return e.code, {"error": content}
    except Exception as ex:
        return 500, {"error": str(ex)}

def run_tests():
    print("================================================================")
    print("🔒 RUNNING COMPREHENSIVE SECURITY & REST API VERIFICATION SUITE")
    print("================================================================")

    passed = 0
    failed = 0

    def assert_eq(test_name, actual, expected, detail=""):
        nonlocal passed, failed
        if actual == expected:
            print(f" [PASS] {test_name} {detail}")
            passed += 1
        else:
            print(f" [FAIL] {test_name}: Expected {expected}, got {actual} {detail}")
            failed += 1

    # 1. Health check
    status, body = make_req("/health")
    assert_eq("Health check returns 200", status, 200, f"-> {body.get('status')}")

    # 2. PRIORITY 1: Unauthenticated Data Exposure Audits (Anonymous Must Return 401)
    print("\n--- Auditing Anonymous Access Rejection (401) ---")
    protected_anonymous_endpoints = [
        ("GET", "/api/analytics", None),
        ("GET", "/api/subjects", None),
        ("GET", "/api/students", None),
        ("GET", "/api/students/stu_aryan_01", None),
        ("GET", "/api/attendance/stu_aryan_01", None),
        ("GET", "/api/risk/stu_aryan_01", None),
        ("POST", "/api/prediction", {"studentId": "stu_aryan_01", "attendNext": 5, "missNext": 0}),
    ]

    for meth, ep, pay in protected_anonymous_endpoints:
        st, _ = make_req(ep, method=meth, payload=pay, token=None)
        assert_eq(f"Anonymous {meth} {ep} returns 401 Unauthorized", st, 401)

    # 3. Authentication: Password Security Tests
    print("\n--- Auditing Authentication & Password Verification ---")
    status, body = make_req("/api/auth/login", method="POST", payload={
        "email": "admin@college.edu",
        "password": "wrongpassword123"
    })
    assert_eq("Login with wrong password rejected with 401", status, 401)

    status, body = make_req("/api/auth/login", method="POST", payload={
        "email": "admin@college.edu",
        "password": "faculty123"  # foreign password
    })
    assert_eq("Login with foreign account password rejected with 401", status, 401)

    # Valid Logins
    status, admin_auth = make_req("/api/auth/login", method="POST", payload={
        "email": "admin@college.edu",
        "password": "admin123"
    })
    assert_eq("Admin login successful", status, 200)
    admin_token = admin_auth.get("token")

    status, faculty_auth = make_req("/api/auth/login", method="POST", payload={
        "email": "prof.sharma@college.edu",
        "password": "faculty123"
    })
    assert_eq("Faculty login successful", status, 200)
    faculty_token = faculty_auth.get("token")
    faculty_user = faculty_auth.get("user")

    status, student_auth = make_req("/api/auth/login", method="POST", payload={
        "email": "aryan.sharma@college.edu",
        "password": "student123"
    })
    assert_eq("Student login successful", status, 200)
    student_token = student_auth.get("token")
    student_user = student_auth.get("user")
    aryan_student_id = student_user.get("studentId")

    # 4. JWT Security: Tampering, Missing Expiry, and Forgery Rejection
    print("\n--- Auditing JWT Cryptographic Validation ---")
    forged_token = faculty_token[:-5] + "XXXXX"
    status, _ = make_req("/api/auth/me", token=forged_token)
    assert_eq("Forged/Tampered JWT signature rejected with 401", status, 401)

    # Malformed base64url segments
    status, _ = make_req("/api/auth/me", token="invalid.token.format!!!")
    assert_eq("Malformed token rejected with 401", status, 401)

    # Fake unsigned token
    fake_header = base64.urlsafe_b64encode(b'{"alg":"HS256","typ":"JWT"}').decode().rstrip('=')
    fake_payload = base64.urlsafe_b64encode(b'{"sub":"usr_admin","role":"ADMIN","exp":9999999999}').decode().rstrip('=')
    fake_token = f"{fake_header}.{fake_payload}.fakesignature123"
    status, _ = make_req("/api/auth/me", token=fake_token)
    assert_eq("Unsigned forged token rejected with 401", status, 401)

    # 5. RBAC & Data Privacy Tests
    print("\n--- Auditing RBAC and Peer Data Privacy ---")
    # Student attempts to create student record -> 403 Forbidden
    status, _ = make_req("/api/students", method="POST", token=student_token, payload={
        "name": "Hacker Student", "email": "hacker@student.edu", "rollNumber": "HACK99"
    })
    assert_eq("Student cannot create students (403)", status, 403)

    # Student attempts to mark attendance -> 403 Forbidden
    status, _ = make_req("/api/attendance", method="POST", token=student_token, payload={
        "subjectId": "sub_dsa_301",
        "records": [{"studentId": aryan_student_id, "status": "PRESENT"}]
    })
    assert_eq("Student cannot mark attendance (403)", status, 403)

    # Find another student ID from student list via Admin token
    status, students_data = make_req("/api/students", token=admin_token)
    assert_eq("Admin can list students", status, 200)
    other_student = None
    for s in students_data.get("students", []):
        if s["studentId"] != aryan_student_id:
            other_student = s
            break

    if other_student:
        other_id = other_student["studentId"]
        # Student attempts to view other student's attendance records -> 403 Forbidden
        status, _ = make_req(f"/api/attendance/{other_id}", token=student_token)
        assert_eq("Student cannot view peer attendance records (403)", status, 403)

        # Student attempts to view other student's risk profile -> 403 Forbidden
        status, _ = make_req(f"/api/risk/{other_id}", token=student_token)
        assert_eq("Student cannot view peer risk profile (403)", status, 403)

        # Student views own attendance records -> 200 OK
        status, own_att = make_req(f"/api/attendance/{aryan_student_id}", token=student_token)
        assert_eq("Student can view own attendance records (200)", status, 200)

        # Student views own risk profile -> 200 OK
        status, own_risk = make_req(f"/api/risk/{aryan_student_id}", token=student_token)
        assert_eq("Student can view own risk profile (200)", status, 200)

    # 6. Faculty Subject Ownership & Cross-Subject Authorization
    print("\n--- Auditing Faculty Subject Authorization ---")
    status, subjects_data = make_req("/api/subjects", token=faculty_token)
    assert_eq("Faculty can view assigned subjects", status, 200)
    faculty_subjects = subjects_data.get("subjects", [])
    assert_eq("Faculty has assigned subjects", len(faculty_subjects) > 0, True)

    assigned_sub_id = faculty_subjects[0].get("id")

    # Attempt to mark attendance for unassigned subject ID -> 403 Forbidden
    status, _ = make_req("/api/attendance", method="POST", token=faculty_token, payload={
        "subjectId": "sub_unassigned_foreign_999",
        "records": [{"studentId": aryan_student_id, "status": "PRESENT"}]
    })
    assert_eq("Faculty cannot mark attendance for unassigned subject (403)", status, 403)

    # 7. Student Onboarding: Setup Token Flow
    print("\n--- Auditing Setup Token Student Onboarding ---")
    unique_roll = f"ROLL{int(time.time())}"
    unique_email = f"newstudent_{int(time.time())}@college.edu"
    status, new_stu_res = make_req("/api/students", method="POST", token=admin_token, payload={
        "name": "Onboarding Test Student",
        "email": unique_email,
        "rollNumber": unique_roll,
        "department": "Computer Science",
        "semester": 4
    })
    assert_eq("Admin creates student with setup token", status, 201)
    setup_token = new_stu_res.get("setupToken")
    assert_eq("Setup token returned to admin", bool(setup_token), True)

    # Attempt to log in before setup is completed -> 401 Unauthorized
    status, _ = make_req("/api/auth/login", method="POST", payload={
        "email": unique_email,
        "password": "student123"
    })
    assert_eq("Login on locked account rejected before setup (401)", status, 401)

    # Complete account setup using setup-password endpoint
    status, setup_res = make_req("/api/auth/setup-password", method="POST", payload={
        "email": unique_email,
        "setupToken": setup_token,
        "newPassword": "MySecureNewPassword@2026"
    })
    assert_eq("Account setup-password successful", status, 200)

    # Now login with the new password
    status, new_stu_auth = make_req("/api/auth/login", method="POST", payload={
        "email": unique_email,
        "password": "MySecureNewPassword@2026"
    })
    assert_eq("Login with newly established password successful", status, 200)

    # 8. Attendance Marking, Correction & Atomic Undo Lifecycle
    print("\n--- Auditing Attendance & Atomic Undo Lifecycle ---")
    status, mark_res = make_req("/api/attendance", method="POST", token=faculty_token, payload={
        "subjectId": assigned_sub_id,
        "date": time.strftime("%Y-%m-%d"),
        "remarks": "Regular Lecture",
        "records": [{"studentId": aryan_student_id, "status": "LATE", "remarks": "Bus delayed"}]
    })
    assert_eq("Faculty marks attendance (200)", status, 200)
    
    # Check undo stack
    status, undo_info = make_req("/api/attendance/undo", token=faculty_token)
    assert_eq("Undo stack available", status, 200)
    stack_size = undo_info.get("undoStackSize") or undo_info.get("stackSize") or len(undo_info.get("history", []))
    assert_eq("Undo stack has actions", stack_size > 0, True)

    # Execute undo
    status, undo_res = make_req("/api/attendance/undo", method="POST", token=faculty_token)
    assert_eq("Undo executes successfully", status, 200)

    # Student cannot execute undo
    status, _ = make_req("/api/attendance/undo", method="POST", token=student_token)
    assert_eq("Student cannot execute undo (403)", status, 403)

    # 9. ML Risk Prediction & What-if Simulation
    print("\n--- Auditing ML Risk Prediction & What-if Simulation ---")
    status, risk_data = make_req(f"/api/risk/{aryan_student_id}", token=student_token)
    assert_eq("Student can view own risk prediction", status, 200)
    has_methodology = "methodology" in risk_data.get("prediction", {}) or "methodology" in risk_data
    assert_eq("Risk prediction has transparent methodology", has_methodology, True)

    status, sim_data = make_req("/api/prediction", method="POST", token=student_token, payload={
        "studentId": aryan_student_id,
        "attendNext": 10,
        "missNext": 0,
        "threshold": 75.0
    })
    assert_eq("Simulation calculator executes", status, 200)
    assert_eq("Projected percentage calculated", "projectedPercentage" in sim_data.get("simulation", {}), True)

    # 10. DSA Benchmarks & Graph Traversal
    print("\n--- Auditing DSA Benchmarks & Graph Traversal ---")
    status, bench_data = make_req("/api/dsa/benchmark")
    assert_eq("DSA benchmarks endpoint returns 200", status, 200)
    assert_eq("Merge Sort metrics present", "mergeSort" in bench_data.get("sorting", {}), True)
    assert_eq("Quick Sort metrics present", "quickSort" in bench_data.get("sorting", {}), True)
    assert_eq("StudentHashMap lookup diagnostics present", "lookupBenchmark" in bench_data, True)

    status, graph_data = make_req("/api/dsa/graph")
    assert_eq("DSA graph endpoint returns 200", status, 200)
    assert_eq("Graph contains nodes and edges", len(graph_data.get("graph", {}).get("nodes", [])) > 0, True)
    assert_eq("BFS traversal returned", "bfs" in graph_data.get("traversals", {}), True)
    assert_eq("DFS traversal returned", "dfs" in graph_data.get("traversals", {}), True)

    print("\n================================================================")
    print(f"🏁 SUMMARY: {passed} PASSED, {failed} FAILED")
    print("================================================================")
    return failed == 0

if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
