import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";

export async function GET(
  req: NextRequest,
  { params }: { params: { id: string } }
) {
  try {
    const sessionUser = getSessionUser(req);
    const { searchParams } = new URL(req.url);
    const threshold = Number(searchParams.get("threshold")) || 75;
    const studentId = params.id;

    // Student privacy: only faculty/admin, preview mode, or the student themselves
    if (sessionUser && sessionUser.role === "STUDENT" && sessionUser.studentId !== studentId) {
      return NextResponse.json(
        { error: "Forbidden: Students may not inspect another student's attendance records" },
        { status: 403 }
      );
    }

    const profile = await AttendanceService.getStudentProfile(studentId, threshold);
    if (!profile) {
      return NextResponse.json({ error: "Student not found" }, { status: 404 });
    }

    return NextResponse.json({ profile });
  } catch (error: any) {
    console.error("GET /api/students/[id] error:", error);
    return NextResponse.json({ error: "Failed to fetch student profile" }, { status: 500 });
  }
}
