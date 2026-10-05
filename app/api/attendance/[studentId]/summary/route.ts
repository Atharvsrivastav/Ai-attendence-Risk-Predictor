import { NextRequest, NextResponse } from "next/server";
import { getSessionUser } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";

export async function GET(
  req: NextRequest,
  { params }: { params: { studentId: string } }
) {
  try {
    const sessionUser = getSessionUser(req);
    const { studentId } = params;
    const { searchParams } = new URL(req.url);
    const threshold = Number(searchParams.get("threshold")) || 75;

    // Student privacy
    if (sessionUser && sessionUser.role === "STUDENT" && sessionUser.studentId !== studentId) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const profile = await AttendanceService.getStudentProfile(studentId, threshold);
    if (!profile) {
      return NextResponse.json({ error: "Student not found" }, { status: 404 });
    }

    const summary = {
      studentId: profile.studentId,
      name: profile.name,
      rollNumber: profile.rollNumber,
      overallPercentage: profile.overallPercentage,
      threshold,
      isBelowThreshold: profile.overallPercentage < threshold,
      totalSessionsConducted: profile.totalSessionsConducted,
      totalSessionsAttended: profile.totalSessionsAttended,
      totalSessionsAbsent: profile.totalSessionsAbsent,
      trend: profile.trend,
      trendSlope: profile.trendSlope,
      currentConsecutiveAbsences: profile.currentConsecutiveAbsences,
      recentWindowPercentage: profile.recentWindowPercentage,
      riskCategory: profile.riskCategory,
      riskScore: profile.riskScore,
      classesNeededToRecover: profile.classesNeededToRecover,
      classesCanBeMissed: profile.classesCanBeMissed,
      recommendation: profile.recommendation,
      subjectWise: profile.subjectWise,
    };

    return NextResponse.json({ summary });
  } catch (error: any) {
    console.error("GET /api/attendance/[studentId]/summary error:", error);
    return NextResponse.json({ error: "Failed to generate attendance summary" }, { status: 500 });
  }
}
