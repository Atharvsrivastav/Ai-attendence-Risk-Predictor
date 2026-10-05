import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";
import { globalAlertQueue } from "@/lib/dsa/data-structures/AttendanceQueue";
import { globalAttendanceUndoStack } from "@/lib/dsa/data-structures/AttendanceStack";

export const dynamic = "force-dynamic";

export async function GET(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    // In student session mode, restrict access
    if (sessionUser && sessionUser.role === "STUDENT") {
      return NextResponse.json({ error: "Forbidden: Analytics requires Faculty or Admin access" }, { status: 403 });
    }

    const { searchParams } = new URL(req.url);
    const threshold = Number(searchParams.get("threshold")) || 75;

    const analytics = await AttendanceService.getClassAnalytics(threshold);

    // Also include live Queue and Stack telemetry
    const recentAlerts = globalAlertQueue.toArray().slice(-8);

    return NextResponse.json({
      ...analytics,
      recentAlerts,
      alertQueueSize: globalAlertQueue.size(),
      undoStackDepth: globalAttendanceUndoStack.size(),
    });
  } catch (error: any) {
    console.error("GET /api/analytics error:", error);
    return NextResponse.json({ error: "Failed to generate analytics" }, { status: 500 });
  }
}
