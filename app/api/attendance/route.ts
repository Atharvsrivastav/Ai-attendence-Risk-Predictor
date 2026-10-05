import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { prisma } from "@/lib/db/prisma";
import { AttendanceStatus } from "@/lib/dsa/types";
import { globalAlertQueue } from "@/lib/dsa/data-structures/AttendanceQueue";
import { globalAttendanceUndoStack } from "@/lib/dsa/data-structures/AttendanceStack";

export async function POST(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (sessionUser && sessionUser.role === "STUDENT") {
      return NextResponse.json({ error: "Forbidden: Faculty or Admin role required" }, { status: 403 });
    }

    const body = await req.json();
    const {
      subjectId,
      sessionDate = new Date(),
      topic = "Class Session",
      records, // Array of { studentId: string, status: AttendanceStatus, remarks?: string }
    } = body;

    if (!subjectId || !records || !Array.isArray(records)) {
      return NextResponse.json({ error: "Subject ID and records array are required" }, { status: 400 });
    }

    const facultyId = sessionUser?.facultyId || (await prisma.faculty.findFirst())?.id;
    if (!facultyId) {
      return NextResponse.json({ error: "Faculty reference missing" }, { status: 400 });
    }

    // Create the session
    const session = await prisma.attendanceSession.create({
      data: {
        subjectId,
        facultyId,
        sessionDate: new Date(sessionDate),
        topic,
      },
      include: { subject: true },
    });

    const createdRecords = [];
    for (const r of records) {
      const student = await prisma.student.findUnique({
        where: { id: r.studentId },
        include: { user: true },
      });

      const rec = await prisma.attendanceRecord.create({
        data: {
          sessionId: session.id,
          studentId: r.studentId,
          status: r.status as AttendanceStatus,
          remarks: r.remarks,
        },
      });

      // Push to AttendanceQueue (FIFO Event Processing)
      globalAlertQueue.enqueue({
        id: `evt-${Date.now()}-${Math.random().toString(36).substring(2, 7)}`,
        type: "RECORD_CREATED",
        studentId: r.studentId,
        studentName: student?.user.name,
        subjectCode: session.subject.code,
        message: `Attendance marked ${r.status} for ${student?.user.name} in ${session.subject.code}`,
        timestamp: new Date(),
        severity: r.status === "ABSENT" ? "WARNING" : "INFO",
      });

      // Push to AttendanceStack for Undo capability
      globalAttendanceUndoStack.push({
        id: `undo-${rec.id}`,
        recordId: rec.id,
        sessionId: session.id,
        studentId: r.studentId,
        studentName: student?.user.name || "Student",
        subjectCode: session.subject.code,
        previousStatus: "ABSENT", // default opposite or initial state
        newStatus: r.status as AttendanceStatus,
        timestamp: new Date(),
        facultyId: sessionUser?.userId || "demo-faculty-id",
        reason: "Initial roll call submission",
      });

      createdRecords.push(rec);
    }

    return NextResponse.json({
      success: true,
      sessionId: session.id,
      recordsCount: createdRecords.length,
      queueSize: globalAlertQueue.size(),
      undoStackDepth: globalAttendanceUndoStack.size(),
    });
  } catch (error: any) {
    console.error("POST /api/attendance error:", error);
    return NextResponse.json({ error: "Failed to mark attendance" }, { status: 500 });
  }
}

/**
 * PATCH /api/attendance
 * Allows updating a single attendance record, pushing state change to the Undo Stack.
 */
export async function PATCH(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (!sessionUser || !isAuthorized(sessionUser, ["ADMIN", "FACULTY"])) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const body = await req.json();
    const { recordId, newStatus, reason } = body;

    if (!recordId || !newStatus) {
      return NextResponse.json({ error: "Record ID and newStatus are required" }, { status: 400 });
    }

    const existing = await prisma.attendanceRecord.findUnique({
      where: { id: recordId },
      include: {
        student: { include: { user: true } },
        session: { include: { subject: true } },
      },
    });

    if (!existing) {
      return NextResponse.json({ error: "Attendance record not found" }, { status: 404 });
    }

    const previousStatus = existing.status as AttendanceStatus;

    // Update in database
    const updated = await prisma.attendanceRecord.update({
      where: { id: recordId },
      data: { status: newStatus },
    });

    // Push previous state onto LIFO AttendanceStack
    globalAttendanceUndoStack.push({
      id: `undo-patch-${Date.now()}`,
      recordId: existing.id,
      sessionId: existing.sessionId,
      studentId: existing.studentId,
      studentName: existing.student.user.name,
      subjectCode: existing.session.subject.code,
      previousStatus: previousStatus,
      newStatus: newStatus as AttendanceStatus,
      timestamp: new Date(),
      facultyId: sessionUser.userId,
      reason: reason || "Manual correction by faculty",
    });

    // Enqueue event notification
    globalAlertQueue.enqueue({
      id: `evt-correct-${Date.now()}`,
      type: "ATTENDANCE_CORRECTED",
      studentId: existing.studentId,
      studentName: existing.student.user.name,
      subjectCode: existing.session.subject.code,
      message: `Status corrected from ${previousStatus} to ${newStatus} for ${existing.student.user.name}`,
      timestamp: new Date(),
      severity: "INFO",
    });

    return NextResponse.json({
      success: true,
      updatedRecord: updated,
      undoStackDepth: globalAttendanceUndoStack.size(),
    });
  } catch (error: any) {
    console.error("PATCH /api/attendance error:", error);
    return NextResponse.json({ error: "Failed to update attendance" }, { status: 500 });
  }
}
