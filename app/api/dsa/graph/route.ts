import { NextRequest, NextResponse } from "next/server";
import { prisma } from "@/lib/db/prisma";
import { AttendanceGraph } from "@/lib/dsa/data-structures/AttendanceGraph";

export const dynamic = "force-dynamic";

export async function GET(req: NextRequest) {
  try {
    const { searchParams } = new URL(req.url);
    const startNode = searchParams.get("start") || "fac_1";

    const graph = new AttendanceGraph();

    // 1. Fetch Faculty
    const faculties = await prisma.faculty.findMany({
      include: { user: true },
    });
    for (const f of faculties) {
      graph.addNode({
        id: `fac_${f.id}`,
        label: f.user.name,
        type: "FACULTY",
        metadata: { department: f.department, employeeId: f.employeeId },
      });
    }

    // 2. Fetch Subjects
    const subjects = await prisma.subject.findMany({
      include: { faculty: true },
    });
    for (const s of subjects) {
      const subNodeId = `sub_${s.id}`;
      graph.addNode({
        id: subNodeId,
        label: `${s.code}: ${s.name}`,
        type: "SUBJECT",
        metadata: { credits: s.credits, code: s.code },
      });

      // Edge: Faculty -> Subject
      graph.addEdge({
        source: `fac_${s.facultyId}`,
        target: subNodeId,
        relationship: "TEACHES",
      });
    }

    // 3. Fetch Students & Enrollments (take sample 12 students to keep graph cleanly viewable)
    const enrollments = await prisma.enrollment.findMany({
      take: 50,
      include: {
        student: { include: { user: true } },
        subject: true,
      },
    });

    for (const enr of enrollments) {
      const stuNodeId = `stu_${enr.studentId}`;
      graph.addNode({
        id: stuNodeId,
        label: `${enr.student.rollNumber}: ${enr.student.user.name}`,
        type: "STUDENT",
        metadata: { roll: enr.student.rollNumber },
      });

      // Edge: Subject -> Student
      graph.addEdge({
        source: `sub_${enr.subjectId}`,
        target: stuNodeId,
        relationship: "ENROLLED_IN",
      });
    }

    // Determine actual start node
    const firstFacultyNode = faculties[0] ? `fac_${faculties[0].id}` : "";
    const activeStartNode = graph.getNode(startNode) ? startNode : firstFacultyNode;

    // Run BFS and DFS
    const bfsResult = graph.bfs(activeStartNode);
    const dfsResult = graph.dfs(activeStartNode);

    const graphData = graph.getGraphData();

    return NextResponse.json({
      success: true,
      startNode: activeStartNode,
      graph: graphData,
      traversals: {
        bfs: {
          algorithm: "Breadth-First Search (BFS)",
          startNode: activeStartNode,
          traversalOrder: bfsResult.order,
          nodesVisited: bfsResult.nodesVisited,
          levels: bfsResult.levels,
          timeComplexity: "O(V + E)",
          spaceComplexity: "O(V)",
          dataStructureUsed: "Queue (FIFO)",
        },
        dfs: {
          algorithm: "Depth-First Search (DFS)",
          startNode: activeStartNode,
          traversalOrder: dfsResult.order,
          nodesVisited: dfsResult.nodesVisited,
          timeComplexity: "O(V + E)",
          spaceComplexity: "O(V)",
          dataStructureUsed: "Stack / Recursion (LIFO)",
        },
      },
    });
  } catch (error: any) {
    console.error("GET /api/dsa/graph error:", error);
    return NextResponse.json({ error: "Failed to generate graph data" }, { status: 500 });
  }
}
