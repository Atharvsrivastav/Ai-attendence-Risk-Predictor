export type AttendanceStatus = "PRESENT" | "ABSENT" | "LATE" | "EXCUSED";

export type RiskCategory = "LOW" | "MEDIUM" | "HIGH" | "CRITICAL";

export type TrendType = "IMPROVING" | "STABLE" | "DECLINING";

export interface SessionRecord {
  id: string;
  sessionId: string;
  sessionDate: Date | string;
  subjectId: string;
  subjectCode: string;
  subjectName: string;
  status: AttendanceStatus;
  topic?: string | null;
}

export interface SubjectAttendance {
  subjectId: string;
  subjectCode: string;
  subjectName: string;
  totalConducted: number;
  presentCount: number;
  absentCount: number;
  lateCount: number;
  excusedCount: number;
  percentage: number;
  isBelowThreshold: boolean;
  consecutiveAbsences: number;
  trend: TrendType;
  classesNeededToRecover: number;
  classesCanBeMissed: number;
}

export interface StudentAttendanceProfile {
  studentId: string;
  userId: string;
  name: string;
  email: string;
  rollNumber: string;
  department: string;
  semester: number;
  totalSessionsConducted: number;
  totalSessionsAttended: number;
  totalSessionsAbsent: number;
  overallPercentage: number;
  trend: TrendType;
  trendSlope: number;
  currentConsecutiveAbsences: number;
  maxConsecutiveAbsences: number;
  recentWindowPercentage: number; // percentage in last 5 sessions
  recentAbsenceCount: number;
  classesNeededToRecover: number;
  classesCanBeMissed: number;
  subjectWise: Record<string, SubjectAttendance>;
  riskCategory: RiskCategory;
  riskScore: number; // 0.0 to 1.0
  contributingFactors: string[];
  recommendation: string;
}

export interface DSAOperationMetric {
  operation: string;
  algorithmOrDS: string;
  timeTakenUs: number; // microseconds
  comparisons?: number;
  swapsOrSteps?: number;
  timeComplexity: string;
  spaceComplexity: string;
  details?: string;
}

export interface GraphNode {
  id: string;
  label: string;
  type: "FACULTY" | "SUBJECT" | "STUDENT";
  metadata?: Record<string, unknown>;
}

export interface GraphEdge {
  source: string;
  target: string;
  relationship: "TEACHES" | "ENROLLED_IN" | "ATTENDS";
}

export interface DecisionTreeNode {
  id: string;
  feature?: string;
  condition?: string;
  threshold?: number;
  operator?: "<=" | ">" | "==";
  left?: DecisionTreeNode;
  right?: DecisionTreeNode;
  riskCategory?: RiskCategory;
  riskScore?: number;
  explanation?: string;
}
