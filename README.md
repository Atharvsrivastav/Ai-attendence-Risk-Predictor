# 🎓 AI Attendance Risk Prediction System
### A Production-Quality, DSA-Focused College Mini Project

[![TypeScript](https://img.shields.io/badge/TypeScript-5.6-blue.svg)](https://www.typescriptlang.org/)
[![Next.js](https://img.shields.io/badge/Next.js-14.2-black.svg)](https://nextjs.org/)
[![Prisma ORM](https://img.shields.io/badge/Prisma-5.21-teal.svg)](https://www.prisma.io/)
[![Tailwind CSS](https://img.shields.io/badge/TailwindCSS-3.4-38bdf8.svg)](https://tailwindcss.com/)
[![DSA Core](https://img.shields.io/badge/DSA-Array%20|%20HashMap%20|%20Queue%20|%20Stack%20|%20Tree%20|%20Graph-indigo.svg)](#7-data-structures-used)
[![ML Engine](https://img.shields.io/badge/ML-Decision%20Tree%20|%20Logistic%20Regression-emerald.svg)](#11-aiml-methodology)

---

## 1. Project Title
**AI Attendance Risk Prediction System** (Academic Attendance Defaulter & Risk Forecasting Engine).

---

## 2. Problem Statement
In higher education institutions, the mandatory attendance threshold (typically **75%**) is a prerequisite for examination eligibility. However, traditional college enterprise systems behave as passive, retrospective CRUD applications:
1. They notify students of attendance shortage only at the end of the term, when it is mathematically impossible to recover.
2. They treat attendance as an isolated scalar percentage without analyzing trajectory (whether attendance is declining or improving).
3. They fail to detect early behavioral danger signs, such as consecutive absence streaks or sudden drops in recent lecture windows.
4. They do not demonstrate meaningful computer science fundamentals or Data Structures & Algorithms (DSA).

---

## 3. Objectives
- **Early Defaulter Detection:** Identify at-risk students weeks before examinations using continuous predictive monitoring.
- **Explicit DSA Utilization:** Solve real engineering bottlenecks using 7 core Data Structures (`Array`, `HashMap`, `Set`, `Queue`, `Stack`, `Tree`, `Graph`) and foundational algorithms (`MergeSort`, `QuickSort`, `Binary Search`, `Linear Search`, `BFS`, `DFS`).
- **Explainable Machine Learning:** Blend a supervised Hierarchical Decision Tree and Logistic Regression with feature attribution (odds ratios) so predictions answer *"Why is this student at risk?"*.
- **Recovery Optimization:** Provide an exact algebraic recovery calculator and What-If simulator computing the exact number of consecutive sessions needed to restore eligibility.
- **Strict Role Isolation:** Provide specialized dashboards for Faculty, Students, and Administrators while preventing students from inspecting peers' attendance records.

---

## 4. Proposed Solution
An intelligent, reactive attendance platform built with Next.js, TypeScript, Prisma ORM, and Tailwind CSS. The system processes chronological attendance events through an in-memory DSA layer, feeds engineered statistical features into an ML classifier, and renders interactive, viva-ready dashboards with live algorithm benchmarking and telemetry.

---

## 5. System Features

### Faculty Portal
- **High-level Class Intelligence:** Real-time KPI cards for Total Students, Class Average %, Below 75% Count, High Risk & Critical Risk counts.
- **Interactive Visualizations:** Chronological attendance progression line chart, risk distribution donut chart, and subject-wise comparison bars via Recharts.
- **Configurable Threshold:** Dynamic slider (default 75%) that re-evaluates all recovery calculations and risk classifications in real time.
- **DSA Sorting Benchmarking:** Sort students by Attendance %, Risk Score, Total Absences, or Roll Number using manually implemented **MergeSort** vs **QuickSort**, displaying execution time in microseconds (µs) and comparison counts.
- **Linear & Binary Search:** Search by name or roll number with instant step tracking.
- **Mark Session Attendance:** Quick multi-student roll call modal that enqueues events to `AttendanceQueue` (FIFO) and saves state to `AttendanceStack` (LIFO).
- **Undo Last Action (LIFO):** Instant rollback button that pops the most recent attendance edit from the stack and reverts the database record.
- **CSV Export:** Download full attendance audit reports with a single click.

### Student Portal
- **Personalized Attendance Gauge:** High-contrast overall attendance display and threshold clearance status.
- **Subject-Wise Breakdown:** Progress bars, session tallies, and individual subject trends.
- **Explainable AI (XAI) Panel:** Transparent explanation listing contributing risk factors (e.g., *"Attendance is 64% (11% deficit)", "4 consecutive absences detected"*).
- **Interactive Recovery Simulator:** Slider allowing students to project their future attendance % and risk tier if they attend the next $X$ consecutive lectures.
- **Missable Class Buffer:** Shows how many classes a student can safely miss without dropping below 75%.

### Admin Console
- **Institutional Health Index:** Total students, global class average, active defaulters, and queue processing statistics.
- **Student Enrollment:** Instant student onboarding with automatic curriculum subject enrollments.
- **Faculty & Course Roster:** Overview of all departmental faculty and course allocations.

### Dedicated DSA Analysis & Viva Center (`/dsa-lab`)
- **Data Structure Visualizers:** Interactive inspectors for HashMap buckets/collisions, Queue FIFO stream, Stack LIFO undo operations, Set intersection of absentees, Decision Tree branches, and Graph BFS/DFS traversal runners.
- **Live Algorithm Benchmarks:** Side-by-side comparison of MergeSort, QuickSort, and TimSort.
- **Viva Voce Cheatsheet:** Detailed answers to top external examiner questions.

---

## 6. System Architecture

```
                       [Attendance Database (PostgreSQL / SQLite)]
                                          │
                                          ▼
                             [Prisma ORM Client Layer]
                                          │
                                          ▼
                           [DSA In-Memory Processing Layer]
     ┌───────────────┬──────────────┬──────────────┬──────────────┬──────────────┐
     │               │              │              │              │              │
[StudentHashMap] [AttendanceQueue] [AttendanceStack] [AttendanceSet] [AttendanceGraph]
  (O(1) Lookup)    (FIFO Alert)   (LIFO Undo)   (Unique Sets)  (BFS / DFS)
     │               │              │              │              │              │
     └───────────────┴──────────────┼──────────────┴──────────────┴──────────────┘
                                    │
                                    ▼
                         [Core DSA Algorithms]
         - MergeSort & QuickSort (Sorting by % and Risk Score)
         - Linear Search & Binary Search (Student Search)
         - Ordinary Least Squares (OLS) Linear Regression (Trend)
         - Sliding Window Tail Slice (Recent 5 classes)
         - Closed-form Recovery Equations
                                    │
                                    ▼
                       [Feature Engineering Vector]
         [overall %, recent %, consecutive absences, trend slope, threshold gap]
                                    │
                                    ▼
                        [ML Risk Prediction Ensemble]
             ┌──────────────────────────────────────────────┐
             │ 1. Hierarchical Decision Tree (Structural)    │
             │ 2. Logistic Regression (Probabilistic Log-odds)│
             └──────────────────────────────────────────────┘
                                    │
                                    ▼
               [Explainable AI Output & Recommendation Engine]
                                    │
                                    ▼
               [Interactive Dashboards & Viva Visualizers]
```

---

## 7. Data Structures Used

| Data Structure | Implementation File | Engineering Purpose | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Array / List** | [`lib/dsa/algorithms/recentWindowAnalysis.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/recentWindowAnalysis.ts) | Stores chronological session statuses; sliding window over recent $k=5$ lectures. | Index: $O(1)$<br>Window: $O(k)$ | $O(n)$ |
| **StudentHashMap** | [`lib/dsa/data-structures/StudentHashMap.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/StudentHashMap.ts) | Fast lookup by studentId and rollNumber without $O(n)$ database scans. Separate chaining with djb2 hash. | Avg: $O(1)$<br>Worst: $O(n)$ | $O(n)$ |
| **AttendanceQueue** | [`lib/dsa/data-structures/AttendanceQueue.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/AttendanceQueue.ts) | Linked FIFO queue for processing sequential attendance events and risk alert dispatches. | Enqueue: $O(1)$<br>Dequeue: $O(1)$ | $O(n)$ |
| **AttendanceStack** | [`lib/dsa/data-structures/AttendanceStack.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/AttendanceStack.ts) | LIFO undo stack saving state change history. Enables instant roll-call correction rollback. | Push: $O(1)$<br>Pop: $O(1)$ | $O(k)$ |
| **AttendanceSet** | [`lib/dsa/data-structures/AttendanceSet.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/AttendanceSet.ts) | Enforces uniqueness of IDs/dates and performs Set Intersection to find cross-subject defaulters. | Add/Has: $O(1)$<br>Intersection: $O(\min(A,B))$ | $O(n)$ |
| **RiskDecisionTree** | [`lib/dsa/data-structures/RiskDecisionTree.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/RiskDecisionTree.ts) | Rule-based hierarchical tree evaluating threshold margins and streaks for explainable AI. | Traversal: $O(h) \approx O(1)$ | $O(\text{nodes})$ |
| **AttendanceGraph** | [`lib/dsa/data-structures/AttendanceGraph.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/data-structures/AttendanceGraph.ts) | Adjacency list modeling Faculty $\to$ Subject $\to$ Student network with BFS and DFS traversals. | BFS: $O(V + E)$<br>DFS: $O(V + E)$ | $O(V + E)$ |

---

## 8. Algorithms Used

### 1. MergeSort (Divide-and-Conquer)
- **File:** [`lib/dsa/algorithms/sorting.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/sorting.ts)
- **Role:** Sorts students by Attendance %, Risk Score, or Total Absences.
- **Guarantee:** Strict $O(n \log n)$ comparisons with stable ordering.

### 2. QuickSort (Lomuto Partitioning)
- **File:** [`lib/dsa/algorithms/sorting.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/sorting.ts)
- **Role:** In-place partitioning sort; compared against MergeSort in the live telemetry dashboard.

### 3. Binary Search
- **File:** [`lib/dsa/algorithms/searching.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/searching.ts)
- **Role:** $O(\log n)$ search on sorted roll numbers; records step-by-step trace showing interval halving.

### 4. Linear Search
- **File:** [`lib/dsa/algorithms/searching.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/searching.ts)
- **Role:** $O(n)$ search across unsorted student names or emails.

### 5. Ordinary Least Squares (OLS) Linear Regression Trend
- **File:** [`lib/dsa/algorithms/trendAnalysis.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/trendAnalysis.ts)
- Computes mathematical slope $m$ across binary session attendance indicators ($1 = \text{Present}, 0 = \text{Absent}$):
  $$m = \frac{n \sum (xy) - \sum x \sum y}{n \sum x^2 - (\sum x)^2}$$
  - $m > +0.05 \implies \text{IMPROVING}$
  - $-0.05 \le m \le +0.05 \implies \text{STABLE}$
  - $m < -0.05 \implies \text{DECLINING}$

### 6. Consecutive Absence Detection
- **File:** [`lib/dsa/algorithms/consecutiveAbsences.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/consecutiveAbsences.ts)
- Performs a backward scan from the latest session to detect active absence streaks and a forward pass for historical max streak.

### 7. Recovery & What-If Simulator Equations
- **File:** [`lib/dsa/algorithms/recoveryCalculator.ts`](file:///c:/Users/Atharv/Desktop/Attendance%20Risk%20Prediction/lib/dsa/algorithms/recoveryCalculator.ts)
- **Classes Needed for 75%:**
  $$X = \left\lceil \max\left(0, \frac{R \cdot T - P}{1 - R}\right) \right\rceil = \lceil 3T - 4P \rceil \quad (\text{for } R = 0.75)$$
- **Missable Class Buffer:**
  $$M = \left\lfloor \max\left(0, \frac{P - R \cdot T}{R}\right) \right\rfloor = \left\lfloor \frac{4P - 3T}{3} \right\rfloor \quad (\text{for } R = 0.75)$$

---

## 9. Time & Space Complexity Summary

| Component | Operation | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| `StudentHashMap` | Lookup / Insert | $O(1)$ avg, $O(n)$ worst | $O(n)$ |
| `AttendanceQueue` | Enqueue / Dequeue | $O(1)$ | $O(n)$ |
| `AttendanceStack` | Push / Pop | $O(1)$ | $O(k)$ |
| `AttendanceSet` | Intersection | $O(\min(\|A\|, \|B\|))$ | $O(\|A\| + \|B\|)$ |
| `RiskDecisionTree`| Evaluation | $O(h) \approx O(1)$ | $O(\text{nodes})$ |
| `AttendanceGraph` | BFS / DFS | $O(V + E)$ | $O(V)$ |
| `MergeSort` | Sorting | $O(n \log n)$ guaranteed | $O(n)$ |
| `QuickSort` | Sorting | $O(n \log n)$ avg, $O(n^2)$ worst | $O(\log n)$ |
| `Binary Search` | Search | $O(\log n)$ | $O(1)$ |
| `Linear Search` | Search | $O(n)$ | $O(1)$ |

---

## 10. AI/ML Methodology

### Ensemble Architecture
The system employs an **Ensemble Model** combining:
1. **Hierarchical Decision Tree:** White-box rule branching providing human-readable logical reasoning paths.
2. **Logistic Regression Classifier:** Calibrated probabilistic model with sigmoid activation:
   $$P(\text{Risk}) = \sigma(z) = \frac{1}{1 + e^{-z}}$$
   $$\text{where } z = \beta_0 + \beta_1 (\text{Gap}) + \beta_2 (\text{Streak}) + \beta_3 (\text{Slope}) + \beta_4 (\text{RecentAbsenceFreq})$$

### Feature Attribution & Explainability
Every prediction generates an itemized contribution list detailing the exact log-odds contribution of each feature, ensuring predictions are transparent and defensible.

---

## 11. Database Schema

Built with **Prisma ORM** supporting **PostgreSQL** and **SQLite**:
- `User`: Base credentials, role (`ADMIN`, `FACULTY`, `STUDENT`), and profile info.
- `Student`: Linked to User; rollNumber, department, semester.
- `Faculty`: Linked to User; employeeId, department.
- `Subject`: Course code, course name, credits, assigned facultyId.
- `Enrollment`: Many-to-Many join table linking Student and Subject.
- `AttendanceSession`: Lecture date, time, topic, subjectId, facultyId.
- `AttendanceRecord`: Atomic attendance entry (`PRESENT`, `ABSENT`, `LATE`, `EXCUSED`), remarks, sessionId, studentId.
- `RiskPrediction`: Cached ML prediction, riskLevel, riskScore, contributingFactors, recommendation.
- `RiskHistory`: Historical snapshots for longitudinal risk trend auditing.

---

## 12. API Documentation

| Method | Endpoint | Description | Role Required |
| :--- | :--- | :--- | :--- |
| `POST` | `/api/auth/login` | Authenticate user and issue JWT | Public |
| `POST` | `/api/auth/logout` | Invalidate cookie and clear session | Public |
| `GET` | `/api/auth/me` | Fetch currently logged-in user profile | Authenticated |
| `GET` | `/api/students` | Get students with sorting, searching, and DSA telemetry | Faculty / Admin |
| `POST` | `/api/students` | Enroll a new student | Faculty / Admin |
| `GET` | `/api/students/:id` | Get single student attendance profile | Owner / Faculty / Admin |
| `GET` | `/api/subjects` | List academic subjects and faculty | Authenticated |
| `POST` | `/api/attendance` | Record session attendance (pushes to Queue & Stack) | Faculty / Admin |
| `POST` | `/api/attendance/undo` | Undo most recent roll-call action (LIFO Stack) | Faculty / Admin |
| `GET` | `/api/attendance/:id` | Chronological session attendance history | Owner / Faculty / Admin |
| `GET` | `/api/attendance/:id/summary` | Calculated percentage, streak, and recovery stats | Owner / Faculty / Admin |
| `GET` | `/api/risk/:id` | Detailed ML prediction, tree path, and XAI breakdown | Owner / Faculty / Admin |
| `GET` | `/api/analytics` | High-level class analytics and defaulter metrics | Faculty / Admin |
| `POST` | `/api/prediction` | Interactive What-If attendance recovery simulator | Authenticated |
| `GET` | `/api/dsa/benchmark` | Live benchmarks of MergeSort, QuickSort, and Searches | Public / Viva |
| `GET` | `/api/dsa/graph` | Graph node/edge data and live BFS/DFS traversals | Public / Viva |

---

## 13. Installation & Setup

### Prerequisites
- Node.js v18.0.0 or higher
- npm v9.0.0 or higher

### Steps
```bash
# 1. Clone or navigate to the project directory
cd "Attendance Risk Prediction"

# 2. Install dependencies
npm install

# 3. Initialize Prisma ORM database
npx prisma generate
npx prisma db push

# 4. Seed the database with 24 realistic students, 5 subjects, and 2,400 attendance records
node scripts/seed.mjs
```

---

## 14. Environment Variables

Create or inspect `.env` in the root directory:
```env
# Database (SQLite default for zero-config local run):
DATABASE_URL="file:./dev.db"

# For PostgreSQL deployment (Optional):
# DATABASE_URL="postgresql://postgres:password@localhost:5432/attendance_risk_db?schema=public"

JWT_SECRET="attendance-risk-prediction-jwt-secret-key-987654321"
DEFAULT_ATTENDANCE_THRESHOLD="75"
PORT=3000
```

---

## 15. Running the Project

```bash
# Run Development Server
npm run dev

# Run Production Build
npm run build
npm start
```
Access the application at: `http://localhost:3000`

### Pre-Configured Test Credentials

| Role | Name | Email | Password | Key Characteristics |
| :--- | :--- | :--- | :--- | :--- |
| **Faculty** | Prof. Rajesh Sharma | `prof.sharma@college.edu` | `faculty123` | Teaches CS301 & CS304. Access to class table, sorting, undo stack. |
| **Student** | Rahul Verma | `rahul.verma@college.edu` | `student123` | **64% Attendance (High Risk)**, 4 consecutive absences, declining trend. |
| **Student** | Aryan Sharma | `aryan.sharma@college.edu` | `student123` | **94% Attendance (Low Risk)**, exemplary record, missable buffer = 5. |
| **Admin** | Dr. K. R. Raman (Dean) | `admin@college.edu` | `admin123` | Full administrative control, student enrollment, course roster. |

*Note: You can also use the **Role Switcher** in the top navigation bar to switch personas with one click.*

---

## 16. Verification & Automated Tests

Run the complete DSA & ML automated test suite:
```bash
npm run test:dsa
```
**Test Coverage:**
- `StudentHashMap`: Insert, lookup, collisions, dynamic resizing, load factor.
- `AttendanceQueue`: FIFO order, enqueue, dequeue, peek.
- `AttendanceStack`: LIFO order, push, pop, undo rollback.
- `AttendanceSet`: Set union and intersection for chronic absentees.
- `AttendanceGraph`: Node creation, BFS level-by-level traversal, DFS pathfinding.
- `MergeSort & QuickSort`: Correct ordering, comparison and swap tracking.
- `Linear & Binary Search`: Target match, step counts, complexity verification.
- `Attendance Calculator & Recovery`: Formulas, ceil/floor boundaries, what-if simulator.
- `ML Risk Engine`: Feature extraction, Decision Tree pathing, Logistic Regression probabilities.

---

## 17. Limitations
- Current dataset uses simulated academic semester timelines (though strictly derived from real records).
- Biometric hardware integration (RFID/Fingerprint scanners) is mocked via session batch endpoints.

---

## 18. Future Scope
- Integration with university SMS / WhatsApp APIs for automated parent alerts.
- RFID / Face Recognition automated attendance ingest via IoT gateways.
- Advanced longitudinal LSTM recurrent models for multi-semester attendance forecasting.
