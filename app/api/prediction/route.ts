import { NextRequest, NextResponse } from "next/server";
import { getSessionUser } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";
import { simulateAttendance } from "@/lib/dsa/algorithms/recoveryCalculator";
import { predictAttendanceRisk } from "@/lib/ml/riskPredictor";
import { AttendanceStatus } from "@/lib/dsa/types";

export async function POST(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    const body = await req.json();
    const {
      studentId,
      attendNext = 0,
      missNext = 0,
      threshold = 75,
    } = body;

    const targetStudentId = studentId || sessionUser?.studentId;
    if (!targetStudentId) {
      return NextResponse.json({ error: "Student ID required" }, { status: 400 });
    }

    // Privacy
    if (sessionUser && sessionUser.role === "STUDENT" && sessionUser.studentId !== targetStudentId) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const profile = await AttendanceService.getStudentProfile(targetStudentId, threshold);
    if (!profile) {
      return NextResponse.json({ error: "Student not found" }, { status: 404 });
    }

    // Run simulation
    const simulation = simulateAttendance(
      profile.totalSessionsAttended,
      profile.totalSessionsConducted,
      Number(attendNext),
      Number(missNext),
      threshold
    );

    // Build simulated status array
    const simulatedStatuses: AttendanceStatus[] = [];
    for (let i = 0; i < profile.totalSessionsAttended; i++) simulatedStatuses.push("PRESENT");
    for (let i = 0; i < profile.totalSessionsAbsent; i++) simulatedStatuses.push("ABSENT");
    for (let i = 0; i < Number(attendNext); i++) simulatedStatuses.push("PRESENT");
    for (let i = 0; i < Number(missNext); i++) simulatedStatuses.push("ABSENT");

    const projectedPrediction = predictAttendanceRisk(simulatedStatuses, threshold);

    return NextResponse.json({
      success: true,
      current: {
        attendancePercentage: profile.overallPercentage,
        riskCategory: profile.riskCategory,
        riskScore: profile.riskScore,
      },
      simulation,
      projectedRisk: {
        category: projectedPrediction.riskCategory,
        score: projectedPrediction.riskScore,
        recommendation: projectedPrediction.recommendation,
        contributingFactors: projectedPrediction.contributingFactors,
      },
    });
  } catch (error: any) {
    console.error("POST /api/prediction error:", error);
    return NextResponse.json({ error: "Failed to simulate prediction" }, { status: 500 });
  }
}
