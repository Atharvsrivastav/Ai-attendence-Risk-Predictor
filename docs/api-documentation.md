# REST API Documentation

The C++ backend server listens on `http://127.0.0.1:8080`. When accessed through the Next.js frontend proxy, all endpoints are accessible under `http://localhost:3000/api/*`.

---

## 1. Authentication Endpoints

### POST `/api/auth/login`
Authenticates a user and issues a signed JWT session token.

- **Request Body:**
```json
{
  "email": "admin@college.edu",
  "password": "admin123"
}
```

- **Response (200 OK):**
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": "cmwjgg5mf000010smjs8chdqo",
    "email": "admin@college.edu",
    "name": "Dr. K. R. Raman (Dean of Academics)",
    "role": "ADMIN",
    "studentId": null,
    "facultyId": null
  }
}
```

### POST `/api/auth/logout`
Clears the session cookie.

- **Response (200 OK):**
```json
{
  "success": true,
  "message": "Logged out successfully"
}
```

### GET `/api/auth/me`
Retrieves claims of the currently authenticated user from cookie or Authorization header.

- **Response (200 OK):**
```json
{
  "authenticated": true,
  "user": {
    "id": "cmwjgg5mf000010smjs8chdqo",
    "email": "admin@college.edu",
    "name": "Dr. K. R. Raman (Dean of Academics)",
    "role": "ADMIN"
  }
}
```

---

## 2. Analytics & Overview Endpoints

### GET `/api/analytics`
Returns aggregate statistics across all students, subjects, and attendance sessions.

- **Query Parameters:** `threshold` (default: 75.0)
- **Response (200 OK):**
```json
{
  "totalStudents": 24,
  "averageAttendance": 74.23,
  "belowThresholdCount": 12,
  "atRiskCount": 12,
  "threshold": 75.0,
  "riskDistribution": {
    "low": 12,
    "medium": 5,
    "high": 0,
    "critical": 7
  },
  "subjectBreakdown": [
    {
      "code": "CS301",
      "name": "Data Structures & Algorithms",
      "averageAttendance": 74.1,
      "totalSessions": 20
    }
  ],
  "sessionTimeline": [ ... ],
  "recentAlerts": [ ... ],
  "undoStackDepth": 0,
  "alertQueueSize": 0
}
```

---

## 3. Subjects & Students Endpoints

### GET `/api/subjects`
Returns list of all active subjects with faculty assignments and stats.

- **Response (200 OK):**
```json
{
  "subjects": [
    {
      "id": "cmwjgg5n3000310smfkgo7fch",
      "code": "CS301",
      "name": "Data Structures & Algorithms",
      "credits": 4,
      "facultyId": "cmwjgg5mw000110sm8396dmkn",
      "facultyName": "Prof. Rajesh Sharma",
      "totalSessions": 20,
      "enrolledStudents": 24,
      "averageAttendance": 74.1
    }
  ]
}
```

### GET `/api/students`
Returns student profiles with attendance metrics, sorting, and search capabilities.

- **Query Parameters:**
  - `subjectId` (optional): Filter by course
  - `risk` (optional): Filter by risk category (LOW, MEDIUM, HIGH, CRITICAL)
  - `sortBy` (optional): `name`, `rollNumber`, `attendance`
  - `sortOrder` (optional): `asc`, `desc`
  - `search` (optional): Name or roll number substring
- **Response (200 OK):**
```json
{
  "total": 24,
  "students": [
    {
      "id": "cmwjgg644006b10sm34uj6du3",
      "studentId": "cmwjgg644006b10sm34uj6du3",
      "name": "Aryan Sharma",
      "rollNumber": "2024CS001",
      "overallPercentage": 85.0,
      "riskCategory": "LOW",
      "riskScore": 0.12,
      "trend": "IMPROVING",
      "currentConsecutiveAbsences": 0,
      "classesNeededToRecover": 0,
      "classesCanBeMissed": 2
    }
  ]
}
```

### GET `/api/students/:id`
Retrieves comprehensive attendance profile for a specific student, including subject-wise breakdown and ML explanation.

- **Response (200 OK):**
```json
{
  "profile": {
    "studentId": "cmwjgg644006b10sm34uj6du3",
    "name": "Aryan Sharma",
    "rollNumber": "2024CS001",
    "overallPercentage": 85.0,
    "riskCategory": "LOW",
    "riskScore": 0.12,
    "contributingFactors": [
      "Consistent attendance above institutional threshold (85.0%)",
      "Zero consecutive absences detected"
    ],
    "recommendation": "Student attendance is safe. Continue present pace.",
    "subjectWise": {
      "CS301": {
        "percentage": 90.0,
        "presentCount": 18,
        "absentCount": 2,
        "consecutiveAbsences": 0
      }
    }
  }
}
```

---

## 4. Attendance Marking & Undo Endpoints

### POST `/api/attendance`
Marks session attendance for multiple students.

- **Request Body:**
```json
{
  "subjectId": "cmwjgg5n3000310smfkgo7fch",
  "records": [
    {
      "studentId": "cmwjgg644006b10sm34uj6du3",
      "status": "PRESENT",
      "remarks": "On time"
    }
  ]
}
```

- **Response (200 OK):**
```json
{
  "success": true,
  "sessionId": "ses_1700000000",
  "recordsProcessed": 1,
  "alertsTriggered": []
}
```

### GET `/api/attendance/undo`
Returns recent undo actions and active alert queue items.

- **Response (200 OK):**
```json
{
  "undoStackSize": 1,
  "alertQueueLength": 0,
  "recentUndoHistory": [ ... ],
  "recentAlerts": [ ... ]
}
```

### POST `/api/attendance/undo`
Reverts the most recent attendance marking action.

- **Response (200 OK):**
```json
{
  "success": true,
  "message": "Reverted attendance for Aryan Sharma in CS301 back to ABSENT",
  "action": {
    "recordId": "rec_ses_1700000000_cmwjgg644006b10sm34uj6du3",
    "revertedTo": "ABSENT"
  }
}
```

---

## 5. Machine Learning & Prediction Endpoints

### POST `/api/prediction`
Simulates student attendance and calculates projected risk level and recovery options.

- **Request Body:**
```json
{
  "studentId": "cmwjgg644006b10sm34uj6du3",
  "attendNext": 10,
  "missNext": 1,
  "threshold": 75.0
}
```

- **Response (200 OK):**
```json
{
  "success": true,
  "current": {
    "attendancePercentage": 85.0,
    "riskCategory": "LOW",
    "riskScore": 0.12
  },
  "simulation": {
    "additionalAttended": 10,
    "additionalMissed": 1,
    "projectedTotalSessions": 31,
    "projectedAttendedSessions": 27,
    "projectedPercentage": 87.1,
    "meetsThreshold": true,
    "deltaPercentage": 2.1
  },
  "projectedRisk": {
    "category": "LOW",
    "score": 0.10,
    "recommendation": "Maintaining attendance will preserve safe academic standing.",
    "contributingFactors": [
      "Projected attendance 87.1% well above required 75.0% threshold."
    ]
  }
}
```

---

## 6. DSA Laboratory Endpoints

### GET `/api/dsa/benchmark`
Runs live benchmarks comparing Merge Sort vs. Quick Sort and Linear Search vs. Binary Search on real dataset records.

- **Response (200 OK):**
```json
{
  "success": true,
  "datasetSize": 24,
  "sorting": {
    "mergeSort": {
      "comparisons": 78,
      "durationMicroseconds": 658,
      "timeComplexity": "O(n log n)"
    },
    "quickSort": {
      "comparisons": 54,
      "swaps": 32,
      "durationMicroseconds": 100,
      "timeComplexity": "O(n log n) avg, O(n^2) worst"
    }
  },
  "searching": {
    "linearSearch": {
      "found": true,
      "stepsCount": 12,
      "timeComplexity": "O(n)"
    },
    "binarySearch": {
      "found": true,
      "stepsCount": 4,
      "timeComplexity": "O(log n)"
    }
  }
}
```

### GET `/api/dsa/graph`
Returns adjacency list graph model and real-time BFS/DFS traversals.

- **Query Parameters:** `start` (node ID, e.g., `FACULTY_cmwjgg5mw000110sm8396dmkn`)
- **Response (200 OK):**
```json
{
  "success": true,
  "startNode": "FACULTY_cmwjgg5mw000110sm8396dmkn",
  "graph": {
    "nodes": [ ... ],
    "edges": [ ... ]
  },
  "traversals": {
    "bfs": {
      "order": [ ... ],
      "nodesVisited": 27,
      "levels": { ... }
    },
    "dfs": {
      "order": [ ... ],
      "nodesVisited": 27
    }
  }
}
```
