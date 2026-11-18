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

# Run the automated C++ unit tests (15/15 tests)
backend/build/attendance_tests

# Launch the C++ backend server (listens on port 8080)
backend/build/attendance_server
```

### 2. Run the Automated C++ Test Suite
```bash
./backend/build/attendance_tests
```
Output:
```
========================================================
    AI ATTENDANCE RISK PREDICTOR - C++ TEST SUITE       
========================================================
[RUNNING] StudentHashMap: Insert, Get, Update, Remove, and Auto-Resize ... PASSED
[RUNNING] AttendanceQueue: FIFO Enqueue, Dequeue, Peek, and Thread Safety ... PASSED
[RUNNING] AttendanceStack: LIFO Push, Pop, Peek, and Depth Pruning ... PASSED
[RUNNING] AttendanceSet: Add, Remove, Intersect, Union, and Difference ... PASSED
[RUNNING] RiskDecisionTree: Hierarchical Classification of Risk Levels ... PASSED
[RUNNING] AttendanceGraph: BFS, DFS, and Shortest Path Traversal ... PASSED
[RUNNING] Algorithms: MergeSort and QuickSort with Metrics ... PASSED
[RUNNING] Algorithms: LinearSearch and BinarySearch with Step Traces ... PASSED
[RUNNING] Attendance Calculations: Present=1, Late=0.5, Absent=0, Excused=Excluded ... PASSED
[RUNNING] Consecutive Absences: Active Backward Streak & Max Semester Streak ... PASSED
[RUNNING] Sliding Window Analysis & Ordinary Least Squares Trend ... PASSED
[RUNNING] Recovery Calculator: Formulas & Simulation ... PASSED
[RUNNING] Machine Learning: Logistic Regression & Ensemble Risk Classification ... PASSED
[RUNNING] AuthService: Hashing, Verification, and Signed Tokens ... PASSED
[RUNNING] Database & Repositories: SQLite Persistence, Users, Students, Sessions, Undo ... PASSED
========================================================
 TEST RESULTS: 15 PASSED, 0 FAILED
========================================================
```

### 3. Launch the Frontend Dashboard
```bash
# Install frontend dependencies
npm install

# Start Next.js development server (port 3000)
npm run dev
```
Open **`http://localhost:3000`** in your browser. All API requests are automatically forwarded to the C++ Crow backend on port 8080.

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
