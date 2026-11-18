# 🎓 AI Attendance Risk Prediction System
### High-Performance C++20 Backend & React Browser Dashboard

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Crow](https://img.shields.io/badge/Crow-1.3.4-red.svg)](https://crowcpp.org/)
[![SQLite](https://img.shields.io/badge/SQLite-3.47-lightblue.svg)](https://www.sqlite.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-5.6-blue.svg)](https://www.typescriptlang.org/)
[![Next.js](https://img.shields.io/badge/Next.js-14.2-black.svg)](https://nextjs.org/)
[![Tailwind CSS](https://img.shields.io/badge/TailwindCSS-3.4-38bdf8.svg)](https://tailwindcss.com/)
[![DSA Core](https://img.shields.io/badge/DSA-Array%20|%20HashMap%20|%20Queue%20|%20Stack%20|%20Tree%20|%20Graph-indigo.svg)](#7-data-structures-used)
[![ML Engine](https://img.shields.io/badge/ML-Decision%20Tree%20|%20Logistic%20Regression-emerald.svg)](#11-aiml-methodology)

---

## 1. Project Overview

The **AI Attendance Risk Prediction System** is a full-stack, enterprise-grade attendance risk forecasting and academic analytics platform. 

The backend has been migrated to **modern C++20** using the **Crow HTTP web framework**, standalone **Asio**, and **embedded SQLite3**, delivering microsecond execution speeds for all algorithms, data structures, and ML predictions. The frontend is a responsive React/TypeScript browser dashboard built with Next.js 14, Tailwind CSS, and Recharts, connected via a zero-overhead reverse proxy.

---

## 2. Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    Browser Client Dashboard                  │
│       (Admin, Faculty, Student, DSA Interactive Lab)         │
└──────────────────────────────┬───────────────────────────────┘
                               │ HTTP / JSON (Port 3000)
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                  Next.js Gateway & Reverse Proxy             │
│        (Serves React UI & Rewrites /api/* to Port 8080)      │
└──────────────────────────────┬───────────────────────────────┘
                               │ REST API Calls (Port 8080)
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                 C++20 Crow REST API Server                   │
│                                                              │
│  ┌─────────────────────────┐   ┌───────────────────────────┐ │
│  │   Auth & Session Claims │   │ Custom Data Structures    │ │
│  │   (JWT + Bcrypt/SHA256) │   │ (StudentHashMap, Queues,  │ │
│  └─────────────────────────┘   │  Stack, Sets, Graphs)     │ │
│                                └───────────────────────────┘ │
│  ┌─────────────────────────┐   ┌───────────────────────────┐ │
│  │   Analytics & Service   │   │ ML Risk Predictor Engine  │ │
│  │   (Profile Aggregation, │   │ (Logistic Regression +    │ │
│  │    Threshold Auditing)  │   │  Decision Tree Ensemble)  │ │
│  └─────────────────────────┘   └───────────────────────────┘ │
│                                                              │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │       SQLite Thread-Safe Connection Pool & RAII         │ │
│  └─────────────────────────────────────────────────────────┘ │
└──────────────────────────────┬───────────────────────────────┘
                               │ SQL Queries
                               ▼
┌──────────────────────────────────────────────────────────────┐
│            SQLite3 Database (`database/attendance.db`)       │
│  (Users, Students, Faculty, Subjects, Records, Undo, Alerts) │
└──────────────────────────────────────────────────────────────┘
```

---

## 3. Technology Stack

### Backend (C++20)
- **Language:** C++20 (GCC 16.2.0 / Clang / MSVC)
- **Framework:** Crow C++ (v1.3.4 multi-threaded REST framework)
- **Networking:** Standalone Asio 1.30.2
- **Database:** SQLite 3.47.0 (with thread-safe RAII connection pooling)
- **JSON Engine:** nlohmann/json 3.12.0
- **Build System:** CMake 3.20+ with MinGW / Unix Makefiles

### Frontend (Browser Dashboard)
- **Framework:** React 18 / Next.js 14 (App Router)
- **Language:** TypeScript 5.6
- **Styling:** Tailwind CSS 3.4
- **Charts:** Recharts
- **Icons:** Lucide React

---

## 4. Quick Start & Execution

### Prerequisites
- GCC / G++ supporting C++20 (or MinGW-w64 on Windows)
- CMake 3.20+
- Node.js 18+ and npm

### 1. Build and Run the C++ Backend
```bash
# Configure CMake
cmake -B backend/build -S backend -G "MinGW Makefiles"

# Build the main server and test suite
cmake --build backend/build --target attendance_server
cmake --build backend/build --target attendance_tests

# Run the automated C++ unit tests (27/27 tests across 6 suites)
backend/build/attendance_tests

# Launch the C++ backend server (requires JWT_SECRET >= 256 bits)
backend/build/attendance_server
```

### 2. Run the Automated C++ Test Suite
```bash
./backend/build/attendance_tests
```
Output:
```
========================================================
 AI ATTENDANCE RISK PREDICTOR — REPEATABLE C++ TEST SUITE
========================================================
 [DSA] StudentHashMap: Insert, Get, Update, Collision djb2, and Auto-Resize ... PASSED
 [DSA] AttendanceQueue: FIFO Enqueue, Dequeue, Peek, Thread Safety ... PASSED
 [DSA] AttendanceStack: LIFO Push, Pop, Peek, removeIf, Max Depth Pruning ... PASSED
 [DSA] AttendanceSet: Add, Remove, Intersect, Unite, Difference, Subset ... PASSED
 [DSA] AttendanceGraph: Adjacency List, BFS, DFS, Shortest Path ... PASSED
 [Algorithms] Sorting: MergeSort and QuickSort with Swap and Comparison Counts ... PASSED
 [Algorithms] Searching: LinearSearch vs BinarySearch Step-by-Step Traces ... PASSED
 [Algorithms] Attendance Calculator: Rules (P=1, L=0.5, A=0, E=Excluded) ... PASSED
 [Algorithms] Consecutive Absences: Backward Streak and Excused Exemption ... PASSED
 [Algorithms] Sliding Window Analysis & OLS Linear Regression Trend ... PASSED
 [Algorithms] Recovery Calculator: Exact Needed Lectures & Safe Misses ... PASSED
 [Auth] Cryptographic OS RNG: Salt Uniqueness & Secure Token Generation ... PASSED
 [Auth] Password Verification: Bcrypt Verification & Rejection of Foreign/Legacy Passwords ... PASSED
 [Auth] Account Setup Token: Lockout Before Setup & Password Creation ... PASSED
 [Auth] JWT HS256: Secret Length Requirement (>= 256 bits / 32 chars) ... PASSED
 [Auth] JWT HS256: Claims Validation (iss, aud, iat, exp, signature, forgery) ... PASSED
 [Database] Integrity: Parameterized CRUD, Foreign Key Enforcement, Duplicate Rejection ... PASSED
 [Database] Transactions: Atomic Multi-Operation Rollback on Failure ... PASSED
 [Database] Transactions: Strict Commit Success Validation ... PASSED
 [Authorization] Faculty Subject Ownership & Enrollment Relationship Checks ... PASSED
 [Undo] Case A: Newly Created Record Deletion ... PASSED
 [Undo] Case B: Existing Record Correction Restoration ... PASSED
 [ML] Prediction: Low Risk (>85% Attendance, Positive Momentum) ... PASSED
 [ML] Prediction: 75% Boundary Condition (Borderline Medium Risk) ... PASSED
 [ML] Prediction: High Risk (70% Attendance with Downward Drop) ... PASSED
 [ML] Prediction: Critical Risk (<50% Attendance, Severe Streak) ... PASSED
 [ML] Prediction: Edge Case (0 Conducted Sessions) ... PASSED
========================================================
 FINAL RESULTS: 27 PASSED, 0 FAILED
========================================================
```

### 3. Run the Comprehensive Security & REST Verification Suite
```bash
python scripts/verify_security_and_api.py
```
Output:
```
================================================================
🔒 RUNNING COMPREHENSIVE SECURITY & REST API VERIFICATION SUITE
================================================================
 [PASS] Health check returns 200 -> healthy
 [PASS] Anonymous GET /api/analytics returns 401 Unauthorized
 [PASS] Anonymous GET /api/subjects returns 401 Unauthorized
 [PASS] Anonymous GET /api/students returns 401 Unauthorized
 [PASS] Anonymous GET /api/students/:id returns 401 Unauthorized
 [PASS] Anonymous GET /api/attendance/:studentId returns 401 Unauthorized
 [PASS] Anonymous GET /api/risk/:studentId returns 401 Unauthorized
 [PASS] Anonymous POST /api/prediction returns 401 Unauthorized
 [PASS] Login with wrong password rejected with 401
 [PASS] Login with foreign account password rejected with 401
 [PASS] Admin login successful
 [PASS] Faculty login successful
 [PASS] Student login successful
 [PASS] Forged/Tampered JWT signature rejected with 401
 [PASS] Malformed token rejected with 401
 [PASS] Unsigned forged token rejected with 401
 [PASS] Student cannot create students (403)
 [PASS] Student cannot mark attendance (403)
 [PASS] Admin can list students
 [PASS] Student cannot view peer attendance records (403)
 [PASS] Student cannot view peer risk profile (403)
 [PASS] Student can view own attendance records (200)
 [PASS] Student can view own risk profile (200)
 [PASS] Faculty can view assigned subjects
 [PASS] Faculty has assigned subjects
 [PASS] Faculty cannot mark attendance for unassigned subject (403)
 [PASS] Admin creates student with setup token
 [PASS] Setup token returned to admin
 [PASS] Login on locked account rejected before setup (401)
 [PASS] Account setup-password successful
 [PASS] Login with newly established password successful
 [PASS] Faculty marks attendance (200)
 [PASS] Undo stack available
 [PASS] Undo stack has actions
 [PASS] Undo executes successfully
 [PASS] Student cannot execute undo (403)
 [PASS] Student can view own risk prediction
 [PASS] Risk prediction has transparent methodology
 [PASS] Simulation calculator executes
 [PASS] Projected percentage calculated
 [PASS] DSA benchmarks endpoint returns 200
 [PASS] Merge Sort metrics present
 [PASS] Quick Sort metrics present
 [PASS] StudentHashMap lookup diagnostics present
 [PASS] DSA graph endpoint returns 200
 [PASS] Graph contains nodes and edges
 [PASS] BFS traversal returned
 [PASS] DFS traversal returned
================================================================
🏁 SUMMARY: 48 PASSED, 0 FAILED
================================================================
```

### 4. Launch the Frontend Dashboard
```bash
# Install frontend dependencies
npm install

# Start Next.js development server (port 3000)
npm run dev
```
Open **`http://localhost:3000`** in your browser. All API requests are automatically forwarded to the C++ Crow backend on port 8080 via the Next.js reverse proxy (`BACKEND_URL`).

---

## 5. Demo Credentials

| Role | Email | Password | Access Capabilities |
| :--- | :--- | :--- | :--- |
| **Admin** | `admin@college.edu` | `admin123` | Institutional analytics, student/faculty directories, system health |
| **Faculty** | `prof.sharma@college.edu` | `faculty123` | Class attendance marking, LIFO undo button, live DSA lab benchmarks |
| **Student** | `aryan.sharma@college.edu` | `student123` | Personal attendance %, XAI risk breakdown, What-If recovery calculator |

---

## 6. Implemented Data Structures in C++

1. **`StudentHashMap<K, V>`**:
   - Rolling polynomial hash (`djb2`) with separate chaining buckets.
   - Dynamic auto-resizing when load factor exceeds 0.75.
2. **`AttendanceQueue<T>`**:
   - Thread-safe FIFO queue with mutex synchronization.
   - Manages asynchronous attendance alerts.
3. **`AttendanceStack<T>`**:
   - Thread-safe LIFO stack with configurable maximum depth and pruning.
   - Powers the 1-click attendance rollback / undo operation.
4. **`AttendanceSet<T>`**:
   - Hash set with $O(\min(|A|, |B|))$ intersection, union, difference, and subset checking.
5. **`RiskDecisionTree`**:
   - Hierarchical binary classification tree evaluating critical threshold boundaries, momentum drops, and consecutive absences.
6. **`AttendanceGraph`**:
   - Adjacency list representation connecting Faculty, Subjects, and Students.
   - Implements Breadth-First Search (BFS) with level tracking, Depth-First Search (DFS), and shortest path discovery.

---

## 7. Algorithms & Machine Learning

- **Sorting:** Manual Merge Sort & Quick Sort with real-time comparison counts, swap tallies, and microsecond duration timers.
- **Searching:** Linear Search ($O(n)$) and Binary Search ($O(\log n)$) with step-by-step visual trace recording.
- **Attendance Rules:**
  - $\text{Present} = 1.0$
  - $\text{Late} = 0.5$
  - $\text{Absent} = 0.0$
  - $\text{Excused} =$ Excluded from conducted denominator
- **Recovery Formulas:**
  $$\text{Classes Needed} = \left\lceil \frac{T \times S - A}{1 - T} \right\rceil$$
  $$\text{Allowable Misses} = \left\lfloor \frac{A - T \times S}{T} \right\rfloor$$
- **Ensemble ML Risk Predictor:**
  - Extracts 7 key statistical features (overall %, recent %, consecutive absences, total absences, absence frequency, OLS trend slope, threshold gap).
  - Evaluates Logistic Regression logit and sigmoid probability.
  - Combines with Decision Tree node score (60% LR / 40% Tree) to classify students into **LOW**, **MEDIUM**, **HIGH**, and **CRITICAL** risk levels with explainable factor attributions.

---

## 8. Documentation

- [Architecture Guide](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/docs/architecture.md)
- [REST API Documentation](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/docs/api-documentation.md)
- [Migration & Parity Report](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/docs/migration-report.md)
