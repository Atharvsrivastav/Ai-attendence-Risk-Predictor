import { NextRequest, NextResponse } from "next/server";
import { getSessionUser, isAuthorized } from "@/lib/auth/session";
import { AttendanceService } from "@/lib/services/attendanceService";
import { mergeSortWithMetrics, quickSortWithMetrics, Comparator } from "@/lib/dsa/algorithms/sorting";
import { linearSearch } from "@/lib/dsa/algorithms/searching";
import { StudentAttendanceProfile } from "@/lib/dsa/types";
import { prisma } from "@/lib/db/prisma";
import bcrypt from "bcryptjs";

export async function GET(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    const { searchParams } = new URL(req.url);
    const threshold = Number(searchParams.get("threshold")) || 75;
    const search = searchParams.get("search")?.trim().toLowerCase() || "";
    const sortBy = searchParams.get("sortBy") || "attendance"; // attendance, riskScore, absences, rollNumber
    const sortOrder = searchParams.get("sortOrder") === "desc" ? "desc" : "asc";
    const sortAlgorithm = searchParams.get("sortAlgorithm") || "mergeSort"; // mergeSort, quickSort
    const filterRisk = searchParams.get("filterRisk"); // LOW, MEDIUM, HIGH, CRITICAL
    const belowThresholdOnly = searchParams.get("belowThreshold") === "true";

    // If sessionUser is present and is a student, enforce student data isolation
    if (sessionUser && sessionUser.role === "STUDENT") {
      if (!sessionUser.studentId) {
        return NextResponse.json({ error: "Student record not linked" }, { status: 403 });
      }
      const myProfile = await AttendanceService.getStudentProfile(sessionUser.studentId, threshold);
      return NextResponse.json({
        students: myProfile ? [myProfile] : [],
        total: myProfile ? 1 : 0,
        dsaMetrics: null,
      });
    }

    // Faculty, Admin, or Demo/Preview Mode



    // Retrieve all student profiles via DSA layer (StudentHashMap + Algorithms)
    const { profiles, hashMap } = await AttendanceService.getAllStudentProfiles(threshold);

    let filtered = [...profiles];

    // DSA Linear Search demonstration for custom text queries
    let searchMetric = null;
    if (search) {
      const searchResult = linearSearch(
        filtered,
        (s) =>
          s.name.toLowerCase().includes(search) ||
          s.rollNumber.toLowerCase().includes(search) ||
          s.email.toLowerCase().includes(search),
        (s) => `${s.rollNumber}: ${s.name}`
      );
      searchMetric = {
        algorithm: searchResult.algorithm,
        timeTakenMicroseconds: searchResult.timeTakenMicroseconds,
        stepsCount: searchResult.stepsCount,
        timeComplexity: searchResult.timeComplexity,
      };

      // Also filter matching subset
      filtered = filtered.filter(
        (s) =>
          s.name.toLowerCase().includes(search) ||
          s.rollNumber.toLowerCase().includes(search) ||
          s.email.toLowerCase().includes(search)
      );
    }

    // Filter by Risk Category
    if (filterRisk) {
      filtered = filtered.filter((s) => s.riskCategory === filterRisk.toUpperCase());
    }

    // Filter below threshold
    if (belowThresholdOnly) {
      filtered = filtered.filter((s) => s.overallPercentage < threshold);
    }

    // DSA Sorting Demonstration (MergeSort vs QuickSort)
    let comparator: Comparator<StudentAttendanceProfile>;

    if (sortBy === "riskScore") {
      comparator = (a, b) =>
        sortOrder === "desc" ? b.riskScore - a.riskScore : a.riskScore - b.riskScore;
    } else if (sortBy === "absences") {
      comparator = (a, b) =>
        sortOrder === "desc"
          ? b.totalSessionsAbsent - a.totalSessionsAbsent
          : a.totalSessionsAbsent - b.totalSessionsAbsent;
    } else if (sortBy === "rollNumber") {
      comparator = (a, b) =>
        sortOrder === "desc"
          ? b.rollNumber.localeCompare(a.rollNumber)
          : a.rollNumber.localeCompare(b.rollNumber);
    } else {
      // Default: attendance percentage
      comparator = (a, b) =>
        sortOrder === "desc"
          ? b.overallPercentage - a.overallPercentage
          : a.overallPercentage - b.overallPercentage;
    }

    const sortStart = performance.now();
    let sortedProfiles: StudentAttendanceProfile[];
    let comparisonsCount = 0;
    let swapsCount = 0;

    if (sortAlgorithm === "quickSort") {
      const qs = quickSortWithMetrics(filtered, comparator);
      sortedProfiles = qs.sorted;
      comparisonsCount = qs.comparisons;
      swapsCount = qs.swaps;
    } else {
      // MergeSort
      const ms = mergeSortWithMetrics(filtered, comparator);
      sortedProfiles = ms.sorted;
      comparisonsCount = ms.comparisons;
    }
    const sortEnd = performance.now();

    const dsaMetrics = {
      sort: {
        algorithm: sortAlgorithm === "quickSort" ? "QuickSort" : "MergeSort",
        comparisons: comparisonsCount,
        swaps: swapsCount,
        timeTakenMicroseconds: Math.max(Math.round((sortEnd - sortStart) * 1000), 1),
        timeComplexity:
          sortAlgorithm === "quickSort" ? "O(n log n) avg, O(n²) worst" : "O(n log n) guaranteed",
        spaceComplexity: sortAlgorithm === "quickSort" ? "O(log n)" : "O(n)",
      },
      search: searchMetric,
      hashMapStats: hashMap.getDiagnostics(),
    };

    return NextResponse.json({
      students: sortedProfiles,
      total: sortedProfiles.length,
      threshold,
      dsaMetrics,
    });
  } catch (error: any) {
    console.error("GET /api/students error:", error);
    return NextResponse.json({ error: "Failed to fetch students" }, { status: 500 });
  }
}

export async function POST(req: NextRequest) {
  try {
    const sessionUser = getSessionUser(req);
    if (!sessionUser || !isAuthorized(sessionUser, ["ADMIN", "FACULTY"])) {
      return NextResponse.json({ error: "Forbidden" }, { status: 403 });
    }

    const body = await req.json();
    const { name, email, rollNumber, department = "Computer Science & Engineering", semester = 5 } = body;

    if (!name || !email || !rollNumber) {
      return NextResponse.json({ error: "Name, email, and roll number are required" }, { status: 400 });
    }

    const defaultPasswordHash = await bcrypt.hash("student123", 10);

    const newUser = await prisma.user.create({
      data: {
        name,
        email: email.toLowerCase().trim(),
        passwordHash: defaultPasswordHash,
        role: "STUDENT",
      },
    });

    const student = await prisma.student.create({
      data: {
        userId: newUser.id,
        rollNumber: rollNumber.trim().toUpperCase(),
        department,
        semester: Number(semester),
      },
    });

    // Auto-enroll in all available subjects
    const subjects = await prisma.subject.findMany({ select: { id: true } });
    for (const sub of subjects) {
      await prisma.enrollment.create({
        data: {
          studentId: student.id,
          subjectId: sub.id,
        },
      });
    }

    return NextResponse.json({ success: true, student, user: newUser }, { status: 201 });
  } catch (error: any) {
    console.error("POST /api/students error:", error);
    return NextResponse.json(
      { error: error.code === "P2002" ? "Roll number or email already exists" : "Failed to create student" },
      { status: 400 }
    );
  }
}
