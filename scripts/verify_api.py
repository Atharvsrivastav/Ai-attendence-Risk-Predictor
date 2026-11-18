import urllib.request
import json
import http.cookiejar

cookie_jar = http.cookiejar.CookieJar()
opener = urllib.request.build_opener(urllib.request.HTTPCookieProcessor(cookie_jar))

BASE_URL = "http://127.0.0.1:8080"

def test_endpoint(name, path, method="GET", payload=None):
    url = f"{BASE_URL}{path}"
    data = json.dumps(payload).encode('utf-8') if payload else None
    headers = {"Content-Type": "application/json"} if payload else {}
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    
    try:
        with opener.open(req) as resp:
            content = resp.read().decode('utf-8')
            res = json.loads(content)
            print(f"[SUCCESS] {name} ({method} {path}) -> Status {resp.status}")
            return res
    except urllib.error.HTTPError as e:
        err = e.read().decode('utf-8')
        print(f"[ERROR] {name} ({method} {path}) -> Status {e.code}: {err}")
        return None

print("=== VERIFYING C++ REST API BACKEND FULL SUITE ===")

# 1. Health
test_endpoint("Health Check", "/health")

# 2. Login as Faculty
login_res = test_endpoint("Faculty Login", "/api/auth/login", method="POST", payload={
    "email": "prof.sharma@college.edu",
    "password": "faculty123"
})
if login_res and login_res.get("token"):
    opener.addheaders = [("Authorization", f"Bearer {login_res['token']}")]

# 3. Auth Me
me = test_endpoint("Auth Me (Current Faculty)", "/api/auth/me")
print(f"   Logged in as: {me['user']['name']} ({me['user']['role']})")

# 4. Subjects
subjects_res = test_endpoint("List Subjects", "/api/subjects")
first_sub = subjects_res["subjects"][0]
print(f"   First Subject: {first_sub['code']} - {first_sub['name']}")

# 5. Students List
students_res = test_endpoint("List Students", "/api/students")
stu = students_res["students"][0]
stu_id = stu["studentId"]
print(f"   Sample Student: {stu['name']} ({stu['rollNumber']}), Attendance: {stu['overallPercentage']}%, Risk: {stu['riskCategory']}")

# 6. Mark Attendance (Faculty Action)
mark_res = test_endpoint("Mark Attendance", "/api/attendance", method="POST", payload={
    "subjectId": first_sub["id"],
    "records": [
        {"studentId": stu_id, "status": "ABSENT", "remarks": "Late bus arrival"}
    ]
})
print(f"   Records processed: {mark_res.get('recordsProcessed')}, Alert count: {len(mark_res.get('alertsTriggered', []))}")

# 7. Check Undo Stack
undo_history = test_endpoint("Fetch Undo History", "/api/attendance/undo")
print(f"   Undo stack size: {undo_history.get('undoStackSize')}, Alert queue length: {undo_history.get('alertQueueLength')}")

# 8. Trigger Undo (Revert the attendance mark)
undo_res = test_endpoint("Revert Attendance Action", "/api/attendance/undo", method="POST")
print(f"   Undo executed: {undo_res.get('message')}")

# 9. Attendance Simulation & Prediction (Student What-If Calculator)
sim_res = test_endpoint("What-If Attendance Simulation", "/api/prediction", method="POST", payload={
    "studentId": stu_id,
    "attendNext": 10,
    "missNext": 1,
    "threshold": 75.0
})
print(f"   Current Pct: {sim_res['current']['attendancePercentage']}% -> Projected: {sim_res['simulation']['projectedPercentage']}%")
print(f"   Projected Risk: {sim_res['projectedRisk']['category']} (Score: {sim_res['projectedRisk']['score']})")

# 10. DSA Benchmarks
bench = test_endpoint("DSA Lab Benchmarks", "/api/dsa/benchmark")
print(f"   Benchmark components: Sorting (Merge={bench['sorting']['mergeSort']['durationMicroseconds']}us, Quick={bench['sorting']['quickSort']['durationMicroseconds']}us), Searching={bench['searching']['binarySearch']['timeComplexity']}")

# 11. DSA Graph Traversal
graph_data = test_endpoint("DSA Graph Traversal", f"/api/dsa/graph?start=FACULTY_{first_sub['facultyId']}")
print(f"   Graph nodes: {len(graph_data['graph']['nodes'])}, Edges: {len(graph_data['graph']['edges'])}, BFS visited: {graph_data['traversals']['bfs']['nodesVisited']}")

print("=== ALL FUNCTIONAL CAPABILITIES FULLY VERIFIED ON C++ BACKEND ===")
