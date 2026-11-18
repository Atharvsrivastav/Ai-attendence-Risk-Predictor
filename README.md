# 🎓 AI Attendance Risk Prediction System
### Complete C++20 Full-Stack Web Application

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Crow](https://img.shields.io/badge/Crow-1.3.4-red.svg)](https://crowcpp.org/)
[![SQLite](https://img.shields.io/badge/SQLite-3.47-lightblue.svg)](https://www.sqlite.org/)
[![Argon2id](https://img.shields.io/badge/Argon2id-RFC%209106-emerald.svg)](https://monocypher.org/)
[![DSA Core](https://img.shields.io/badge/DSA-Array%20|%20HashMap%20|%20Queue%20|%20Stack%20|%20Tree%20|%20Graph-indigo.svg)](#6-custom-data-structures-and-algorithms)
[![ML Engine](https://img.shields.io/badge/ML-Decision%20Tree%20|%20Logistic%20Regression-emerald.svg)](#7-machine-learning-risk-predictor)
[![Automated Tests](https://img.shields.io/badge/Tests-105%20Passing-success.svg)](#8-testing--verification)

---

## 1. Project Overview

The **AI Attendance Risk Prediction System** is a pure, self-contained **C++20 full-stack web application** engineered for enterprise university academic analytics and proactive dropout/exam-shortage prevention.

**Zero External Framework Overhead:**
- **Primary Language:** C++20
- **Web & Routing Framework:** Crow C++ (Fast, multithreaded HTTP micro-framework)
- **Embedded Database:** SQLite3 with RAII prepared statement wrappers, foreign keys, and atomic scoped transactions
- **Authentication:** Opaque server-side sessions stored in SQLite with SHA-256 token hashing; Argon2id (RFC 9106) password hashing via Monocypher (with legacy bcrypt backward compatibility); Cryptographic OS RNG
- **Dynamic Presentation:** Server-rendered HTML and CSS generated dynamically by C++ (no Node.js, Next.js, React, or JavaScript required in application runtime)
- **Data Visualizations:** Pure C++ dynamic vector graphics (SVG donut gauges, subject bar charts, trend charts, risk distributions)
- **Reporting Engine:** Streaming CSV exports for students, academic risk alerts, subjects, and faculty rosters

---

## 2. Architecture Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Web Browser Client                              │
│       (Admin, Faculty, Student Portals & Interactive DSA Lab)          │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ HTTP / HTML / CSS / SVG (Port 8080)
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                 C++20 Full-Stack Crow Web Server                       │
│                                                                        │
│  ┌──────────────────────────────┐   ┌────────────────────────────────┐ │
│  │   Server-Side Web Routing    │   │ Pure C++ View Generators       │ │
│  │   - Session & Cookie Mgmt    │   │ - Admin, Faculty, Student Views│ │
│  │   - CSRF & Rate Limiting     │   │ - Dynamic SVG Chart Engine     │ │
│  │   - Role-Based Access Control│   │ - CSV Export Generators        │ │
│  └──────────────────────────────┘   └────────────────────────────────┘ │
│                                                                        │
│  ┌──────────────────────────────┐   ┌────────────────────────────────┐ │
│  │   Custom Data Structures     │   │ Machine Learning Risk Engine   │ │
│  │   - StudentHashMap (djb2)    │   │ - Logistic Regression Model    │ │
│  │   - AttendanceQueue (FIFO)   │   │ - Explainable Decision Tree    │ │
│  │   - AttendanceStack (Undo)   │   │ - OLS Trend Linear Regression  │ │
│  │   - AttendanceGraph (BFS/DFS)│   │ - What-If Recovery Calculator  │ │
│  └──────────────────────────────┘   └────────────────────────────────┘ │
│                                                                        │
│  ┌──────────────────────────────┐   ┌────────────────────────────────┐ │
│  │   Security & Cryptography    │   │ Embedded SQLite Database Layer │ │
│  │   - Monocypher Argon2id      │   │ - RAII Scoped Transactions     │ │
│  │   - Bcrypt Legacy Verify     │   │ - Parameterized Query Defense  │ │
│  │   - Cryptographic OS RNG     │   │ - WAL Mode & Foreign Keys ON   │ │
│  └──────────────────────────────┘   └────────────────────────────────┘ │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Direct C API
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                        SQLite Database (attendance.db)                 │
│         (14 Tables: users, students, faculty, subjects, sessions,      │
│          records, enrollments, user_sessions, audit_logs, config)      │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Technology Stack

| Layer | Implementation | Details |
|---|---|---|
| **Language** | C++20 | ISO standard C++20 compiled with GCC 16.2 / Clang |
| **Web Server** | Crow C++ 1.3.4 | Multithreaded HTTP server with routing and cookie management |
| **Database** | SQLite 3.47 (Embedded) | Thread-safe, WAL journal mode, enforced foreign key constraints |
| **Password Hashing** | Argon2id (Monocypher) | RFC 9106 memory-hard KDF, salt generated via OS RNG |
| **Session Security** | Opaque Server-Side Sessions | SHA-256 token hashing in SQLite, `HttpOnly`, `SameSite=Lax` |
| **Data Interchange** | nlohmann/json | Modern C++ JSON parsing and serialization |
| **Frontend Presentation** | C++ Server-Rendered HTML & CSS | Modern, responsive SaaS design system with CSS custom properties |
| **Visual Charts** | Dynamic SVG generated by C++ | Zero external chart libraries; clean, vector-sharp graphics |
| **Build System** | CMake 3.20+ | Multi-platform build for Linux, macOS, and Windows |

---

## 4. Key Functional Modules

### A. Authentication & Session Management
- **Role-Based Access Control (RBAC):** Strict partition among `ADMIN`, `FACULTY`, and `STUDENT` roles.
- **Opaque Sessions:** 32-byte cryptographically random session tokens stored as SHA-256 hashes in `user_sessions`.
- **First-Time Student Activation:** Secure 24-hour setup tokens (`/setup-password`) generated by admins for credential activation.
- **Dual Authentication Bridge:** Supports web browser cookies (`session_id`) and Bearer JWT for automated API verification.

### B. Institutional Admin Dashboard
- **Executive Analytics:** Active student enrollments, faculty staff, course curricula, overall institutional attendance, and shortage deficits.
- **Student Roster Management:** Search by student name or roll number, filter by department and semester.
- **Configurable University Threshold:** Live dynamic attendance threshold adjustment (e.g. 75.0%) with automatic institutional audit logging.
- **Audit Trail:** Immutable records of security actions, user logins, configuration changes, and attendance overrides.

### C. Faculty Academic Portal
- **Course Assignment Ownership:** Strict authorization preventing faculty from accessing or modifying unassigned courses.
- **Roll Call Marking:** Fast class-wide attendance marking with custom status pills (`PRESENT`, `ABSENT`, `LATE` [0.5 credit], `EXCUSED`).
- **Atomic LIFO Undo Stack:** 1-click reversal of erroneous roll-call submissions or corrections with case-specific restoration.

### D. Personalized Student Portal
- **Real-Time Attendance Gauge:** Dynamic SVG circular gauge color-coded by standing (Emerald: $\ge 75\%$, Amber: $65\text{--}74\%$, Red: $< 65\%$).
- **Explainable Risk Evaluation:** Breakdown of top contributing risk factors (momentum decay, streak absence, volatility).
- **What-If Attendance Recovery Calculator:** Real-time simulation showing projected attendance percentage and consecutive lectures required to regain semester exam eligibility.

### E. Interactive DSA Laboratory (`/dsa-lab`)
- **Sorting Engine Benchmark:** Live side-by-side comparison of **MergeSort** vs **QuickSort** (Lomuto partition) measuring key comparisons, element swaps, and duration ($\mu\text{s}$).
- **Search Trace Inspector:** Step-by-step trace of **BinarySearch** ($O(\log n)$) vs **LinearSearch** ($O(n)$) showing bounds and mid-point evaluations.
- **StudentHashMap Diagnostics:** Memory layout, bucket capacity, occupied buckets, load factor, and chaining collision counters.
- **AttendanceGraph Traversal:** Full BFS and DFS traversal order demonstrations through institutional relationships.

### F. Reports & CSV Export Center (`/reports`)
- **Students CSV:** Complete roster with attendance percentages, streaks, and risk categories.
- **Academic Risk CSV:** Filtered report of students below threshold requiring immediate academic intervention.
- **Printable Student Transcript:** Official printable record formatted with dedicated `@media print` CSS.

---

## 5. Seed Credentials

| Role | Email | Password | Access Portal |
|---|---|---|---|
| **Admin** | `admin@college.edu` | `admin123` | `/admin` |
| **Faculty** | `prof.sharma@college.edu` | `faculty123` | `/faculty` |
| **Student** | `aryan.sharma@college.edu` | `student123` | `/student` |

---

## 6. Custom Data Structures and Algorithms

All algorithmic operations are custom-implemented in modern C++20 templates without external runtime dependencies:

1. **`StudentHashMap<K, V>`**: Hash map using the polynomial rolling hash function (djb2: `hash * 33 + c`), separate chaining for collision resolution, and automatic $2\times$ rehashing when load factor $\alpha > 0.75$.
2. **`AttendanceQueue<T>`**: Thread-safe FIFO queue with mutex guards used for institutional risk and alert event queuing.
3. **`AttendanceStack<T>`**: LIFO stack with bounded depth and custom `removeIf` pruning, powering atomic faculty undo.
4. **`AttendanceSet<T>`**: Mathematical set implementation supporting union, intersection, difference, and subset checks.
5. **`AttendanceGraph`**: Graph with adjacency lists representing academic entities (`FACULTY` $\to$ `SUBJECT` $\to$ `STUDENT`), supporting BFS, DFS, and Dijkstra/BFS shortest-path calculations.
6. **`MergeSort` & `QuickSort`**: Generic sorting algorithms instrumented with detailed comparison, swap, and microsecond telemetry.
7. **`BinarySearch` & `LinearSearch`**: Search algorithms providing step-by-step trace steps for educational inspection.

---

## 7. Machine Learning Risk Predictor

The risk engine employs an ensemble architecture combining **Logistic Regression** and an **Explainable Decision Tree**:

$$\text{RiskScore} = \sigma\left(\beta_0 + \beta_1(1 - \text{AttPct}) + \beta_2 \cdot \text{Streak} + \beta_3 \cdot \text{RecentDrop} + \beta_4(1 - \text{TrendSlope})\right)$$

- **Attendance Deficit:** Weight $0.40$
- **Consecutive Absences:** Weight $0.25$
- **Recent Window Drop (Last 5 Sessions):** Weight $0.20$
- **OLS Trend Momentum:** Weight $0.15$
- **Classification Categories:**
  - `LOW`: Score $< 0.35$
  - `MEDIUM`: Score $0.35 \text{--} 0.55$
  - `HIGH`: Score $0.55 \text{--} 0.75$
  - `CRITICAL`: Score $\ge 0.75$

---

## 8. Testing & Verification

The project includes 3 distinct, repeatable automated verification suites:

### Suite 1: Native C++ Unit, Integration & Security Tests (`attendance_tests`)
```powershell
cmake --build build --target attendance_tests
./build/attendance_tests
```
- **Result:** **24 PASSED, 0 FAILED** (DSA structures, algorithms, ML boundaries, Argon2id, sessions, transactions, undo)

### Suite 2: Full-Stack Web App UI & Session Verification (`verify_fullstack_cpp.py`)
```powershell
python scripts/verify_fullstack_cpp.py
```
- **Result:** **33 PASSED, 0 FAILED** (CSS, login, session cookies, SVG charts, CSV exports, roll-call, what-if simulator, RBAC 403)

### Suite 3: Comprehensive Security & REST API Verification (`verify_security_and_api.py`)
```powershell
python scripts/verify_security_and_api.py
```
- **Result:** **48 PASSED, 0 FAILED** (Anonymous 401 audits, JWT cryptography, peer privacy isolation, setup token activation)

**Total Test Coverage:** **105 automated checks passing with 0 failures.**

---

## 9. Building and Running

### Prerequisites
- CMake 3.20 or newer
- C++20 compliant compiler (GCC 12+, Clang 15+, MSVC 19.34+, or MinGW-w64)
- Python 3.9+ (for running automated verification scripts)

### Build Instructions
```bash
# 1. Configure CMake
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 2. Build the server and test suite
cmake --build build --config Release

# 3. Run automated C++ test suite
./build/attendance_tests

# 4. Start the Full-Stack Web Server
export JWT_SECRET="your_cryptographic_secret_key_minimum_256_bits_length"
./build/attendance_server
```

Open your browser to:
`http://localhost:8080/login`

---

## 10. License

Educational and Institutional Software License. Developed for academic excellence and predictive student retention.
