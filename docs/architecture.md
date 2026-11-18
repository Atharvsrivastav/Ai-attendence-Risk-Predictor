# System Architecture: C++ Backend & React Frontend

## 1. High-Level Architecture Overview

The **AI Attendance Risk Predictor** is architected as a high-performance, decoupled full-stack platform:
- **Core Backend Engine**: Built in modern **C++20** using the **Crow** lightweight multi-threaded HTTP framework, standalone **Asio**, and embedded **SQLite3**.
- **Data Persistence**: Relational SQLite database (`database/attendance.db`) with an enterprise-ready normalized schema that maps directly to PostgreSQL for production scaling.
- **Frontend Dashboard**: Responsive React 18, Next.js 14 App Router, TypeScript, Tailwind CSS, Lucide icons, and Recharts. All API requests (`/api/*`) are transparently proxied to the C++ Crow server on port 8080.

```
┌──────────────────────────────────────────────────────────────┐
│                    Browser Client Dashboard                  │
│       (Admin, Faculty, Student, DSA Interactive Lab)         │
└──────────────────────────────┬───────────────────────────────┘
                               │ HTTP / JSON (Port 3000)
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                  Next.js Gateway & Reverse Proxy             │
│        (Serves React SSR/CSR UI & Rewrites /api/* to 8080)   │
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

## 2. C++ Backend Layer Breakdown

### A. HTTP & Controller Layer (`backend/src/main.cpp`)
- Built on top of **Crow v1.3.4** and **Asio 1.30.2**.
- Configured with 8 multi-threaded worker pools for asynchronous request handling.
- Implements CORS headers (`Access-Control-Allow-*`) and parses JWT tokens from both HTTP Authorization Bearer headers and `auth_token` cookies.

### B. Core Custom Data Structures (`backend/include/dsa/`)
1. **`StudentHashMap<K, V>`**:
   - Custom hash table using the **djb2 polynomial rolling hash algorithm** with separate chaining buckets.
   - Automatically re-hashes and doubles capacity when load factor exceeds **0.75**.
   - Provides $O(1)$ amortized lookup for student profiles by ID and roll number.
2. **`AttendanceQueue<T>`**:
   - Thread-safe singly-linked FIFO queue with mutex synchronization.
   - Manages real-time attendance alert notifications and processing order.
3. **`AttendanceStack<T>`**:
   - Thread-safe LIFO stack with bounded depth and automatic bottom-pruning.
   - Manages instant single-click undo and revert operations for faculty members.
4. **`AttendanceSet<T>`**:
   - Custom set structure providing efficient set union, intersection, difference, and subset validation.
5. **`AttendanceGraph`**:
   - Graph modeled via adjacency lists connecting Faculty, Subjects, and Students.
   - Supports Breadth-First Search (BFS) with level tracking, Depth-First Search (DFS), and shortest path discovery.
6. **`RiskDecisionTree`**:
   - Hierarchical binary classification tree evaluating critical threshold bounds, momentum drops, and consecutive absences.

### C. Algorithmic Modules (`backend/include/algorithms/`)
- **`Sorting.hpp`**: Manual implementations of Merge Sort and Quick Sort with real-time comparison counts, swap metrics, and microsecond timers.
- **`Searching.hpp`**: Linear Search ($O(n)$) and Binary Search ($O(\log n)$) with step-by-step visual search traces.
- **`AttendanceCalculator.hpp`**: Strict attendance calculation:
  $$\text{Attendance \%} = \frac{\text{Present} + 0.5 \times \text{Late}}{\text{Conducted Sessions (Excl. Excused)}} \times 100$$
- **`ConsecutiveAbsences.hpp`**: Real-time backward scanning for active consecutive absences and historical maximum semester streaks.
- **`RecentWindowAnalysis.hpp`**: 5-session sliding window detecting sudden drops of $>15\%$.
- **`TrendAnalysis.hpp`**: Ordinary Least Squares (OLS) linear regression computing slope and directional momentum.
- **`RecoveryCalculator.hpp`**:
  $$\text{Classes Needed} = \left\lceil \frac{T \times S - A}{1 - T} \right\rceil$$
  $$\text{Allowable Misses} = \left\lfloor \frac{A - T \times S}{T} \right\rfloor$$

### D. Machine Learning Engine (`backend/include/ml/`)
- **`FeatureEngineering.hpp`**: Extracts 7 key operational features:
  1. Overall Attendance %
  2. Recent 5-session Attendance %
  3. Active Consecutive Absences
  4. Total Absences
  5. Recent Absence Frequency
  6. OLS Linear Trend Slope
  7. Threshold Gap ($75\% - \text{Current \%}$)
- **`LogisticRegressionModel.hpp`**: Evaluates feature weights and bias using logit transformation and sigmoid function:
  $$P(\text{Risk}) = \frac{1}{1 + e^{-z}}$$
- **`RiskPredictor.hpp`**: Ensembles Logistic Regression probability (60% weight) with Decision Tree score (40% weight) to classify students into **LOW**, **MEDIUM**, **HIGH**, and **CRITICAL** risk levels with explainable factor attributions.

---

## 3. Database Architecture & Persistence
- SQLite3 file database at `database/attendance.db`.
- Managed using RAII prepared statements with parameter binding to prevent SQL injection.
- Tables:
  - `users` (credentials, role-based access)
  - `faculty` (faculty profiles and departments)
  - `students` (student roll numbers, semesters, departments)
  - `subjects` (course catalog and assigned faculty)
  - `enrollments` (many-to-many relationship)
  - `attendance_sessions` (lecture dates, topics, faculty)
  - `attendance_records` (individual attendance statuses: PRESENT, LATE, ABSENT, EXCUSED)
  - `undo_records` (persistent undo journal for revert operations)
  - `alert_queue_items` (persistent alert log for automated risk notifications)

---

## 4. Security Architecture & Cryptographic Core

### A. Password Security (Bcrypt Blowfish)
- Stored using production-grade Bcrypt hashes (`$2a$10$...`) powered by Solar Designer's public domain `crypt_blowfish`.
- Direct Bcrypt string comparison protects against foreign password substitution and password bypasses.
- No plaintext fallback passwords or insecure shortcuts.

### B. Cryptographically Signed JWT Tokens (HS256)
- Signed using genuine RFC 2104 HMAC-SHA256 (`backend/third_party/sha256.c`).
- Tokens contain `userId`, `email`, `role`, `studentId`, `facultyId`, `issuedAt`, and `expiresAt` (`exp = 7 days`).
- The server recalculates and verifies the HMAC-SHA256 signature on every authenticated request before inspecting claims.
- Forged, tampered, expired, or malformed tokens are rejected immediately with HTTP 401 Unauthorized.
- Secret key is read from `JWT_SECRET` environment variable with fail-safe fallback for local development.

### C. Role-Based Access Control (RBAC) & Data Privacy
- **ADMIN**: Can manage institution-wide data, view all students, faculty, subjects, and analytics.
- **FACULTY**: Can conduct roll calls, correct attendance, undo recent submissions, and inspect assigned student profiles.
- **STUDENT**: Privacy-restricted role. Students can only view their own attendance records (`/api/attendance/{studentId}`), their own profile (`/api/students/{id}`), and their own risk prediction. Accessing peer records returns HTTP 403 Forbidden. Students are prohibited from creating users or mutating attendance.

---

## 5. Transaction Safety & RAII ScopedTransaction
- All database state transitions (roll call submissions, status corrections, and undo operations) are wrapped in `attendance::db::Database::ScopedTransaction`.
- If an exception or database error occurs mid-operation, `ScopedTransaction` automatically triggers `ROLLBACK` in its destructor, preserving SQLite data consistency.
- SQLite is configured with `PRAGMA foreign_keys = ON;` and `PRAGMA journal_mode = WAL;` for concurrent read/write transactions.
- Exact previous statuses are stored in `undo_records` (e.g., LATE -> EXCUSED, PRESENT -> ABSENT) allowing multi-step, bidirectional undo operations that survive process restarts.
