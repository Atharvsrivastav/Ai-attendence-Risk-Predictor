import { NextRequest, NextResponse } from "next/server";
import { getSessionUser } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";
import { predictAttendanceRisk } from "@/lib/ml/riskPredictor";
import { prisma } from "@/lib/db/prisma";
import { AttendanceStatus } from "@/lib/dsa/types";

export async function GET(
  req: NextRequest,
  { params }: { params: { studentId: string } }
) {
  try {
    const sessionUser = getSessionUser(req);
    const { studentId } = params;
    const { searchParams } = new URL(req.url);
    const threshold = Number(searchParams.get("threshold")) || 75;

    // Student privacy check
    if (sessionUser && sessionUser.role === "STUDENT" && sessionUser.studentId !== studentId) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const records = await prisma.attendanceRecord.findMany({
      where: { studentId },
      include: { session: true },
      orderBy: { session: { sessionDate: "asc" } },
    });

    const statuses = records.map((r) => r.status as AttendanceStatus);
    const prediction = predictAttendanceRisk(statuses, threshold);
    const profile = await AttendanceService.getStudentProfile(studentId, threshold);

    return NextResponse.json({
      studentId,
      studentName: profile?.name,
      rollNumber: profile?.rollNumber,
      attendancePercentage: profile?.overallPercentage,
      threshold,
      prediction,
    });
  } catch (error: any) {
    console.error("GET /api/risk/[studentId] error:", error);
    return NextResponse.json({ error: "Failed to evaluate risk prediction" }, { status: 500 });
  }
}
