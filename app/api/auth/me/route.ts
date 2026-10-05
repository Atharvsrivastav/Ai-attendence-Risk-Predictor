import { NextRequest, NextResponse } from "next/server";
import { getSessionUser } from "@/lib/auth/session";
import { prisma } from "@/lib/db/prisma";

export const dynamic = "force-dynamic";

export async function GET(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (!sessionUser) {
      return NextResponse.json({ authenticated: false, user: null }, { status: 401 });
    }

    const user = await prisma.user.findUnique({
      where: { id: sessionUser.userId },
      select: {
        id: true,
        email: true,
        name: true,
        role: true,
        student: {
          select: {
            id: true,
            rollNumber: true,
            department: true,
            semester: true,
          },
        },
        faculty: {
          select: {
            id: true,
            employeeId: true,
            department: true,
          },
        },
      },
    });

    if (!user) {
      return NextResponse.json({ authenticated: false, user: null }, { status: 401 });
    }

    return NextResponse.json({
      authenticated: true,
      user: {
        id: user.id,
        email: user.email,
        name: user.name,
        role: user.role,
        studentId: user.student?.id,
        facultyId: user.faculty?.id,
        rollNumber: user.student?.rollNumber,
        department: user.student?.department || user.faculty?.department,
        semester: user.student?.semester,
      },
    });
  } catch (error: any) {
    console.error("Auth me error:", error);
    return NextResponse.json({ error: "Failed to verify session" }, { status: 500 });
  }
}
