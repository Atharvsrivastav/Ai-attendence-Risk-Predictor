/**
 * ATTENDANCE SERVICE
 * 
 * Orchestrates:
 * Database (Prisma)
 *    ↓
 * Data Structures (StudentHashMap, AttendanceQueue, AttendanceStack, AttendanceSet)
 *    ↓
 * Algorithms (Percentage, Consecutive Absences, Trend OLS, Window, Recovery)
 *    ↓
 * Feature Engineering
 *    ↓
 * ML Risk Prediction
 *    ↓
 * Profile Assembly & Cached Lookups
 */

import { prisma } from "../db/prisma";
import {
  AttendanceStatus,
  StudentAttendanceProfile,
  SubjectAttendance,
} from "../dsa/types";
import { StudentHashMap } from "../dsa/data-structures/StudentHashMap";
import { AttendanceSet } from "../dsa/data-structures/AttendanceSet";
import { calculateAttendancePercentage } from "../dsa/algorithms/attendanceCalculator";
import { detectConsecutiveAbsences } from "../dsa/algorithms/consecutiveAbsences";
import { analyzeAttendanceTrend } from "../dsa/algorithms/trendAnalysis";
import { analyzeRecentAttendanceWindow } from "../dsa/algorithms/recentWindowAnalysis";
import { calculateRecovery } from "../dsa/algorithms/recoveryCalculator";
import { predictAttendanceRisk } from "../ml/riskPredictor";

export class AttendanceService {
  /**
   * Builds a full StudentAttendanceProfile for a given student.
   * All stats are dynamically calculated from stored chronological attendance records.
   */
  public static async getStudentProfile(
    studentId: string,
    threshold = 75
  ): Promise<StudentAttendanceProfile | null> {
    const student = await prisma.student.findUnique({
      where: { id: studentId },
      include: {
        user: true,
        enrollments: {
          include: {
            subject: true,
          },
        },
        attendanceRecords: {
          include: {
            session: {
              include: {
                subject: true,
              },
            },
          },
          orderBy: {
            session: {
              sessionDate: "asc",
            },
          },
        },
      },
    });

    if (!student) return null;

    // Collect all chronological status codes across all subjects
    const allStatuses: AttendanceStatus[] = [];
    // Subject wise grouping using a Subject HashMap
    const subjectMap = new StudentHashMap<string, AttendanceStatus[]>();
    const subjectInfoMap = new StudentHashMap<string, { code: string; name: string }>();

    for (const enr of student.enrollments) {
      subjectMap.set(enr.subjectId, []);
      subjectInfoMap.set(enr.subjectId, {
        code: enr.subject.code,
        name: enr.subject.name,
      });
    }

    for (const record of student.attendanceRecords) {
      const status = record.status as AttendanceStatus;
      allStatuses.push(status);

      const subjList = subjectMap.get(record.session.subjectId);
      if (subjList) {
        subjList.push(status);
      } else {
        subjectMap.set(record.session.subjectId, [status]);
        subjectInfoMap.set(record.session.subjectId, {
          code: record.session.subject.code,
          name: record.session.subject.name,
        });
      }
    }

    // Run DSA Algorithms on overall sequence
    const overallCalc = calculateAttendancePercentage(allStatuses, threshold);
    const consecutive = detectConsecutiveAbsences(allStatuses);
    const trend = analyzeAttendanceTrend(allStatuses);
    const recentWindow = analyzeRecentAttendanceWindow(
      allStatuses,
      5,
      overallCalc.percentage
    );
    const recovery = calculateRecovery(
      overallCalc.effectivePresentCount,
      overallCalc.totalConducted,
      threshold
    );

    // Run ML Risk Prediction Engine
    const prediction = predictAttendanceRisk(allStatuses, threshold);

    // Calculate subject-wise metrics
    const subjectWise: Record<string, SubjectAttendance> = {};
    for (const entry of subjectMap.entries()) {
      const subjId = entry.key;
      const statuses = entry.value;
      const info = subjectInfoMap.get(subjId) || { code: "UNK", name: "Unknown" };

      const subCalc = calculateAttendancePercentage(statuses, threshold);
      const subConsecutive = detectConsecutiveAbsences(statuses);
      const subTrend = analyzeAttendanceTrend(statuses);
      const subRecovery = calculateRecovery(
        subCalc.effectivePresentCount,
        subCalc.totalConducted,
        threshold
      );

      subjectWise[subjId] = {
        subjectId: subjId,
        subjectCode: info.code,
        subjectName: info.name,
        totalConducted: subCalc.totalConducted,
        presentCount: subCalc.presentCount,
        absentCount: subCalc.absentCount,
        lateCount: subCalc.lateCount,
        excusedCount: subCalc.excusedCount,
        percentage: subCalc.percentage,
        isBelowThreshold: subCalc.percentage < threshold,
        consecutiveAbsences: subConsecutive.currentStreak,
        trend: subTrend.trend,
        classesNeededToRecover: subRecovery.classesNeededToRecover,
        classesCanBeMissed: subRecovery.classesCanBeMissed,
      };
    }

    return {
      studentId: student.id,
      userId: student.userId,
      name: student.user.name,
      email: student.user.email,
      rollNumber: student.rollNumber,
      department: student.department,
      semester: student.semester,
      totalSessionsConducted: overallCalc.totalConducted,
      totalSessionsAttended: overallCalc.presentCount,
      totalSessionsAbsent: overallCalc.absentCount,
      overallPercentage: overallCalc.percentage,
      trend: trend.trend,
      trendSlope: trend.slope,
      currentConsecutiveAbsences: consecutive.currentStreak,
      maxConsecutiveAbsences: consecutive.maxStreak,
      recentWindowPercentage: recentWindow.recentPercentage,
      recentAbsenceCount: recentWindow.absentCount,
      classesNeededToRecover: recovery.classesNeededToRecover,
      classesCanBeMissed: recovery.classesCanBeMissed,
      subjectWise,
      riskCategory: prediction.riskCategory,
      riskScore: prediction.riskScore,
      contributingFactors: prediction.contributingFactors,
      recommendation: prediction.recommendation,
    };
  }

  /**
   * Retrieves all student profiles, organized in an in-memory StudentHashMap for O(1) lookups.
   */
  public static async getAllStudentProfiles(
    threshold = 75
  ): Promise<{ profiles: StudentAttendanceProfile[]; hashMap: StudentHashMap<string, StudentAttendanceProfile> }> {
    const students = await prisma.student.findMany({
      select: { id: true },
      orderBy: { rollNumber: "asc" },
    });

    const profiles: StudentAttendanceProfile[] = [];
    const hashMap = new StudentHashMap<string, StudentAttendanceProfile>(64);

    for (const s of students) {
      const p = await this.getStudentProfile(s.id, threshold);
      if (p) {
        profiles.push(p);
        hashMap.set(p.studentId, p);
        hashMap.set(p.rollNumber, p); // Index by roll number as well
      }
    }

    return { profiles, hashMap };
  }

  /**
   * Computes high-level analytics for Faculty / Admin dashboard.
   */
  public static async getClassAnalytics(threshold = 75) {
    const { profiles } = await this.getAllStudentProfiles(threshold);

    const totalStudents = profiles.length;
    let sumPercentage = 0;
    let belowThresholdCount = 0;
    let lowRiskCount = 0;
    let mediumRiskCount = 0;
    let highRiskCount = 0;
    let criticalRiskCount = 0;

    const subjects = await prisma.subject.findMany({
      include: { faculty: { include: { user: true } } },
    });

    // Subject aggregate accumulator
    const subjectAggregates: Record<
      string,
      { code: string; name: string; faculty: string; totalPercentage: number; count: number }
    > = {};

    for (const sub of subjects) {
      subjectAggregates[sub.id] = {
        code: sub.code,
        name: sub.name,
        faculty: sub.faculty.user.name,
        totalPercentage: 0,
        count: 0,
      };
    }

    for (const p of profiles) {
      sumPercentage += p.overallPercentage;
      if (p.overallPercentage < threshold) belowThresholdCount++;

      switch (p.riskCategory) {
        case "LOW":
          lowRiskCount++;
          break;
        case "MEDIUM":
          mediumRiskCount++;
          break;
        case "HIGH":
          highRiskCount++;
          break;
        case "CRITICAL":
          criticalRiskCount++;
          break;
      }

      for (const [subId, subAttendance] of Object.entries(p.subjectWise)) {
        if (subjectAggregates[subId]) {
          subjectAggregates[subId].totalPercentage += subAttendance.percentage;
          subjectAggregates[subId].count++;
        }
      }
    }

    const averageAttendance =
      totalStudents === 0 ? 0 : Number((sumPercentage / totalStudents).toFixed(2));

    const subjectBreakdown = Object.entries(subjectAggregates).map(([id, data]) => ({
      subjectId: id,
      subjectCode: data.code,
      subjectName: data.name,
      faculty: data.faculty,
      averageAttendance:
        data.count === 0 ? 0 : Number((data.totalPercentage / data.count).toFixed(2)),
      enrolledStudents: data.count,
    }));

    // Real session timeline progression computed from database records
    const allSessions = await prisma.attendanceSession.findMany({
      orderBy: { sessionDate: "asc" },
      include: { records: true },
    });

    const sessionTimeline: { session: string; avg: number; atRisk: number }[] = [];
    if (allSessions.length > 0) {
      const numBuckets = Math.min(5, allSessions.length);
      const bucketSize = Math.max(1, Math.ceil(allSessions.length / numBuckets));

      for (let i = 0; i < allSessions.length; i += bucketSize) {
        const chunk = allSessions.slice(i, i + bucketSize);
        let present = 0;
        let total = 0;
        const studentPresences: Record<string, { present: number; total: number }> = {};

        for (const s of chunk) {
          for (const r of s.records) {
            total++;
            if (r.status === "PRESENT") present++;
            if (!studentPresences[r.studentId]) {
              studentPresences[r.studentId] = { present: 0, total: 0 };
            }
            studentPresences[r.studentId].total++;
            if (r.status === "PRESENT") {
              studentPresences[r.studentId].present++;
            }
          }
        }

        const avg = total > 0 ? Number(((present / total) * 100).toFixed(1)) : 0;
        let atRisk = 0;
        for (const st of Object.values(studentPresences)) {
          if (st.total > 0 && (st.present / st.total) * 100 < threshold) {
            atRisk++;
          }
        }

        const startIdx = i + 1;
        const endIdx = Math.min(i + chunk.length, allSessions.length);
        sessionTimeline.push({
          session: `Sess ${startIdx}-${endIdx}`,
          avg,
          atRisk,
        });
      }
    }

    return {
      totalStudents,
      averageAttendance,
      threshold,
      belowThresholdCount,
      riskDistribution: {
        low: lowRiskCount,
        medium: mediumRiskCount,
        high: highRiskCount,
        critical: criticalRiskCount,
      },
      subjectBreakdown,
      sessionTimeline,
      atRiskCount: highRiskCount + criticalRiskCount,
    };
  }
}
