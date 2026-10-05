import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { prisma } from "@/lib/db/prisma";
import { globalAttendanceUndoStack } from "@/lib/dsa/data-structures/AttendanceStack";

export async function POST(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (sessionUser && sessionUser.role === "STUDENT") {
      return NextResponse.json({ error: "Forbidden: Faculty or Admin role required" }, { status: 403 });
    }

    if (globalAttendanceUndoStack.isEmpty()) {
      return NextResponse.json({
        success: false,
        message: "No operations in the AttendanceStack to undo.",
        stackSize: 0,
      });
    }

    // Pop the most recent action (LIFO)
    const lastAction = globalAttendanceUndoStack.pop();
    if (!lastAction) {
      return NextResponse.json({ success: false, message: "Stack was empty" });
    }

    // Revert the status in the database to previousStatus
    const revertedRecord = await prisma.attendanceRecord.update({
      where: { id: lastAction.recordId },
      data: { status: lastAction.previousStatus },
    });

    return NextResponse.json({
      success: true,
      message: `Reverted attendance for ${lastAction.studentName} in ${lastAction.subjectCode} back to ${lastAction.previousStatus} (was ${lastAction.newStatus})`,
      undoneAction: lastAction,
      revertedRecord,
      remainingStackDepth: globalAttendanceUndoStack.size(),
    });
  } catch (error: any) {
    console.error("POST /api/attendance/undo error:", error);
    return NextResponse.json({ error: "Failed to undo attendance action" }, { status: 500 });
  }
}

export async function GET(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (!sessionUser || !isAuthorized(sessionUser, ["ADMIN", "FACULTY"])) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    // Inspect stack contents without popping (for viva stack visualizer!)
    const history = globalAttendanceUndoStack.toArray();
    return NextResponse.json({
      stackSize: globalAttendanceUndoStack.size(),
      history,
    });
  } catch (error: any) {
    return NextResponse.json({ error: "Failed to read stack" }, { status: 500 });
  }
}
