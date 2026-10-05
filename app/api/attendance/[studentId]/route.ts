import { NextRequest, NextResponse } from "next/server";
import { getSessionUser } from "@/lib/auth/session";
import { prisma } from "@/lib/db/prisma";

export async function GET(
  req: NextRequest,
  { params }: { params: { studentId: string } }
) {
  try {
    const sessionUser = getSessionUser(req);
    const { studentId } = params;

    // Student privacy: students cannot view other students' records
    if (sessionUser && sessionUser.role === "STUDENT" && sessionUser.studentId !== studentId) {
      return NextResponse.json(
        { error: "Forbidden: You are only permitted to view your own attendance records" },
        { status: 403 }
      );
    }

    const records = await prisma.attendanceRecord.findMany({
      where: { studentId },
      include: {
        session: {
          include: {
            subject: true,
            faculty: {
              include: { user: { select: { name: true } } },
            },
          },
        },
      },
      orderBy: {
        session: {
          sessionDate: "asc",
        },
      },
    });

    const formatted = records.map((r) => ({
      id: r.id,
      sessionId: r.sessionId,
      sessionDate: r.session.sessionDate,
      subjectId: r.session.subjectId,
      subjectCode: r.session.subject.code,
      subjectName: r.session.subject.name,
      facultyName: r.session.faculty.user.name,
      topic: r.session.topic,
      status: r.status,
      remarks: r.remarks,
    }));

    return NextResponse.json({ records: formatted, count: formatted.length });
  } catch (error: any) {
    console.error("GET /api/attendance/[studentId] error:", error);
    return NextResponse.json({ error: "Failed to fetch attendance history" }, { status: 500 });
  }
}
