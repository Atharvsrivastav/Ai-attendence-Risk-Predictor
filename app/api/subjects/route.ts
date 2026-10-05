import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { prisma } from "@/lib/db/prisma";

export async function GET(req: NextRequest) {
  try {
    // Public/Preview accessible

    const subjects = await prisma.subject.findMany({
      include: {
        faculty: {
          include: {
            user: {
              select: { name: true, email: true },
            },
          },
        },
        _count: {
          select: {
            enrollments: true,
            sessions: true,
          },
        },
      },
      orderBy: { code: "asc" },
    });

    const formatted = subjects.map((s) => ({
      id: s.id,
      code: s.code,
      name: s.name,
      credits: s.credits,
      facultyId: s.facultyId,
      facultyName: s.faculty.user.name,
      facultyEmail: s.faculty.user.email,
      totalSessions: s._count.sessions,
      enrolledCount: s._count.enrollments,
    }));

    return NextResponse.json({ subjects: formatted });
  } catch (error: any) {
    console.error("GET /api/subjects error:", error);
    return NextResponse.json({ error: "Failed to fetch subjects" }, { status: 500 });
  }
}

export async function POST(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (!sessionUser || !isAuthorized(sessionUser, ["ADMIN", "FACULTY"])) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const body = await req.json();
    const { code, name, credits = 3, facultyId } = body;

    if (!code || !name) {
      return NextResponse.json({ error: "Code and name are required" }, { status: 400 });
    }

    const targetFacultyId = facultyId || sessionUser.facultyId;
    if (!targetFacultyId) {
      return NextResponse.json({ error: "Faculty ID is required" }, { status: 400 });
    }

    const subject = await prisma.subject.create({
      data: {
        code: code.trim().toUpperCase(),
        name: name.trim(),
        credits: Number(credits),
        facultyId: targetFacultyId,
      },
    });

    return NextResponse.json({ success: true, subject }, { status: 201 });
  } catch (error: any) {
    console.error("POST /api/subjects error:", error);
    return NextResponse.json(
      { error: error.code === "P2002" ? "Subject code already exists" : "Failed to create subject" },
      { status: 400 }
    );
  }
}
