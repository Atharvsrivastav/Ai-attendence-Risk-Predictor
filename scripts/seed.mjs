import { PrismaClient } from "@prisma/client";
import bcrypt from "bcryptjs";

const prisma = new PrismaClient();

async function main() {
  console.log("🌱 Starting academic attendance database seeding...");

  // Clean existing tables in reverse dependency order
  await prisma.riskHistory.deleteMany();
  await prisma.riskPrediction.deleteMany();
  await prisma.attendanceRecord.deleteMany();
  await prisma.attendanceSession.deleteMany();
  await prisma.enrollment.deleteMany();
  await prisma.subject.deleteMany();
  await prisma.student.deleteMany();
  await prisma.faculty.deleteMany();
  await prisma.user.deleteMany();

  const commonFacultyPasswordHash = await bcrypt.hash("faculty123", 10);
  const commonStudentPasswordHash = await bcrypt.hash("student123", 10);
  const adminPasswordHash = await bcrypt.hash("admin123", 10);

  // 1. Create Admin
  const adminUser = await prisma.user.create({
    data: {
      email: "admin@college.edu",
      name: "Dr. K. R. Raman (Dean of Academics)",
      passwordHash: adminPasswordHash,
      role: "ADMIN",
    },
  });
  console.log("✓ Created Admin:", adminUser.email);

  // 2. Create Faculty
  const faculty1User = await prisma.user.create({
    data: {
      email: "prof.sharma@college.edu",
      name: "Prof. Rajesh Sharma",
      passwordHash: commonFacultyPasswordHash,
      role: "FACULTY",
    },
  });

  const faculty1 = await prisma.faculty.create({
    data: {
      userId: faculty1User.id,
      employeeId: "FAC-CS-01",
      department: "Computer Science & Engineering",
    },
  });

  const faculty2User = await prisma.user.create({
    data: {
      email: "dr.patel@college.edu",
      name: "Dr. Sunita Patel",
      passwordHash: commonFacultyPasswordHash,
      role: "FACULTY",
    },
  });

  const faculty2 = await prisma.faculty.create({
    data: {
      userId: faculty2User.id,
      employeeId: "FAC-CS-02",
      department: "Computer Science & Engineering",
    },
  });
  console.log("✓ Created Faculty: Prof. Rajesh Sharma & Dr. Sunita Patel");

  // 3. Create Core Subjects
  const subjectsData = [
    { code: "CS301", name: "Data Structures & Algorithms", facultyId: faculty1.id, credits: 4 },
    { code: "CS302", name: "Database Management Systems", facultyId: faculty2.id, credits: 4 },
    { code: "CS303", name: "Machine Learning & AI", facultyId: faculty2.id, credits: 3 },
    { code: "CS304", name: "Computer Networks", facultyId: faculty1.id, credits: 3 },
    { code: "CS305", name: "Software Engineering", facultyId: faculty1.id, credits: 3 },
  ];

  const subjects = [];
  for (const s of subjectsData) {
    const created = await prisma.subject.create({ data: s });
    subjects.push(created);
  }
  console.log(`✓ Created ${subjects.length} Subjects.`);

  // 4. Create 24 Students with diverse, realistic attendance patterns
  const studentProfiles = [
    { name: "Aryan Sharma", email: "aryan.sharma@college.edu", roll: "2024CS001", pattern: "EXEMPLARY" },
    { name: "Sneha Rao", email: "sneha.rao@college.edu", roll: "2024CS002", pattern: "EXEMPLARY" },
    { name: "Rahul Verma", email: "rahul.verma@college.edu", roll: "2024CS003", pattern: "HIGH_RISK_DECLINING" }, // Below 75%, 4 streak
    { name: "Priya Nair", email: "priya.nair@college.edu", roll: "2024CS004", pattern: "BORDERLINE" }, // ~75%
    { name: "Rohan Gupta", email: "rohan.gupta@college.edu", roll: "2024CS005", pattern: "SAFE" },
    { name: "Ananya Desai", email: "ananya.desai@college.edu", roll: "2024CS006", pattern: "CRITICAL" }, // < 50%
    { name: "Vikram Singh", email: "vikram.singh@college.edu", roll: "2024CS007", pattern: "SAFE" },
    { name: "Tanvi Joshi", email: "tanvi.joshi@college.edu", roll: "2024CS008", pattern: "BORDERLINE" },
    { name: "Aditya Kulkarni", email: "aditya.kulkarni@college.edu", roll: "2024CS009", pattern: "HIGH_RISK_RECENT_DROP" },
    { name: "Meera Iyer", email: "meera.iyer@college.edu", roll: "2024CS010", pattern: "EXEMPLARY" },
    { name: "Karan Mehta", email: "karan.mehta@college.edu", roll: "2024CS011", pattern: "CRITICAL" },
    { name: "Ishita Roy", email: "ishita.roy@college.edu", roll: "2024CS012", pattern: "SAFE" },
    { name: "Devansh Jain", email: "devansh.jain@college.edu", roll: "2024CS013", pattern: "BORDERLINE" },
    { name: "Riya Kapoor", email: "riya.kapoor@college.edu", roll: "2024CS014", pattern: "EXEMPLARY" },
    { name: "Arjun Reddy", email: "arjun.reddy@college.edu", roll: "2024CS015", pattern: "HIGH_RISK_DECLINING" },
    { name: "Kavya Pillai", email: "kavya.pillai@college.edu", roll: "2024CS016", pattern: "SAFE" },
    { name: "Manish Agarwal", email: "manish.agarwal@college.edu", roll: "2024CS017", pattern: "CRITICAL" },
    { name: "Pooja Hegde", email: "pooja.hegde@college.edu", roll: "2024CS018", pattern: "SAFE" },
    { name: "Harsh Vardhan", email: "harsh.vardhan@college.edu", roll: "2024CS019", pattern: "BORDERLINE" },
    { name: "Simran Kaur", email: "simran.kaur@college.edu", roll: "2024CS020", pattern: "HIGH_RISK_RECENT_DROP" },
    { name: "Gaurav Sen", email: "gaurav.sen@college.edu", roll: "2024CS021", pattern: "EXEMPLARY" },
    { name: "Bhavna Mishra", email: "bhavna.mishra@college.edu", roll: "2024CS022", pattern: "SAFE" },
    { name: "Nikhil Chawla", email: "nikhil.chawla@college.edu", roll: "2024CS023", pattern: "SAFE" },
    { name: "Divya Nambiar", email: "divya.nambiar@college.edu", roll: "2024CS024", pattern: "BORDERLINE" },
  ];

  const students = [];
  for (const s of studentProfiles) {
    const user = await prisma.user.create({
      data: {
        email: s.email,
        name: s.name,
        passwordHash: commonStudentPasswordHash,
        role: "STUDENT",
      },
    });

    const student = await prisma.student.create({
      data: {
        userId: user.id,
        rollNumber: s.roll,
        department: "Computer Science & Engineering",
        semester: 5,
      },
    });

    // Enroll in all 5 subjects
    for (const sub of subjects) {
      await prisma.enrollment.create({
        data: {
          studentId: student.id,
          subjectId: sub.id,
        },
      });
    }

    students.push({ student, pattern: s.pattern, name: s.name, roll: s.roll });
  }
  console.log(`✓ Created ${students.length} Students with Enrollments.`);

  // 5. Create Attendance Sessions across the semester (20 sessions per subject)
  // Generating dates spaced 2 days apart over past 6 weeks
  const baseDate = new Date();
  baseDate.setDate(baseDate.getDate() - 40);

  const sessions = [];
  const sessionTopics = [
    "Course Introduction & Overview",
    "Time & Space Complexity Analysis",
    "Arrays and Dynamic Memory",
    "Singly and Doubly Linked Lists",
    "Stacks: Applications and Implementation",
    "Queues and Priority Queues",
    "Recursion and Backtracking Principles",
    "Binary Trees and Traversal Orders",
    "Binary Search Trees (BST) & AVL",
    "Hash Tables and Collision Strategies",
    "Heaps and Heap Sort",
    "Graph Representation: Adjacency Matrix & List",
    "Graph Traversal: BFS Algorithms",
    "Graph Traversal: DFS Algorithms",
    "Shortest Path: Dijkstra Algorithm",
    "Minimum Spanning Trees: Prim and Kruskal",
    "Dynamic Programming: Memoization & Tabulation",
    "Greedy Algorithms and Interval Scheduling",
    "Divide and Conquer Master Theorem",
    "Comprehensive Review & Problem Solving",
  ];

  for (const sub of subjects) {
    for (let sessionIdx = 0; sessionIdx < sessionTopics.length; sessionIdx++) {
      const sDate = new Date(baseDate.getTime() + sessionIdx * 2 * 24 * 60 * 60 * 1000);
      sDate.setHours(9 + (sessionIdx % 5), 0, 0, 0);

      const session = await prisma.attendanceSession.create({
        data: {
          subjectId: sub.id,
          facultyId: sub.facultyId,
          sessionDate: sDate,
          topic: `${sub.code} - ${sessionTopics[sessionIdx]}`,
        },
      });
      sessions.push({ session, subjectId: sub.id, sessionIdx });
    }
  }
  console.log(`✓ Created ${sessions.length} Attendance Sessions across all subjects.`);

  // 6. Generate Realistic Attendance Records for each student
  // Each student pattern determines deterministic presence/absence sequence
  console.log("📝 Populating individual student attendance records...");

  let totalRecords = 0;

  for (const item of students) {
    const { student, pattern } = item;

    for (const sess of sessions) {
      const idx = sess.sessionIdx;
      const totalSessions = sessionTopics.length;
      let status = "PRESENT";

      if (pattern === "EXEMPLARY") {
        // Attends ~95%, occasional late or 1 absent
        if (idx === 7) status = "ABSENT";
        else if (idx === 14) status = "LATE";
        else status = "PRESENT";
      } else if (pattern === "SAFE") {
        // Attends ~85%
        if (idx === 3 || idx === 11 || idx === 17) status = "ABSENT";
        else if (idx === 8) status = "LATE";
        else status = "PRESENT";
      } else if (pattern === "BORDERLINE") {
        // Attends ~74-76% (Near 75% threshold)
        if (idx === 2 || idx === 6 || idx === 10 || idx === 15 || idx === 18) {
          status = "ABSENT";
        } else if (idx === 9) {
          status = "LATE";
        } else {
          status = "PRESENT";
        }
      } else if (pattern === "HIGH_RISK_DECLINING") {
        // Rahul Verma: started well, then missed 4 consecutive recent sessions!
        // Sessions 0-14: attended most (missed 2)
        // Sessions 16, 17, 18, 19: ABSENT consecutively!
        if (idx === 4 || idx === 9 || idx >= 16) {
          status = "ABSENT";
        } else if (idx === 12) {
          status = "LATE";
        } else {
          status = "PRESENT";
        }
      } else if (pattern === "HIGH_RISK_RECENT_DROP") {
        // Missed 3 of last 4 sessions
        if (idx === 3 || idx === 7 || idx === 12 || idx === 17 || idx === 18 || idx === 19) {
          status = "ABSENT";
        } else {
          status = "PRESENT";
        }
      } else if (pattern === "CRITICAL") {
        // Ananya Desai: Chronic absenteeism, misses > 50%
        if (idx % 2 === 0 || idx >= 14) {
          status = "ABSENT";
        } else {
          status = "PRESENT";
        }
      }

      await prisma.attendanceRecord.create({
        data: {
          sessionId: sess.session.id,
          studentId: student.id,
          status,
          remarks: status === "ABSENT" ? "Unexcused Absence" : undefined,
        },
      });
      totalRecords++;
    }
  }

  console.log(`✓ Inserted ${totalRecords} Attendance Records with zero fake numbers.`);
  console.log("\n========================================================");
  console.log("🎉 Academic Database Seeding Completed Successfully!");
  console.log("========================================================");
  console.log("Credentials:");
  console.log("  Admin:   admin@college.edu / admin123");
  console.log("  Faculty: prof.sharma@college.edu / faculty123");
  console.log("  Faculty: dr.patel@college.edu / faculty123");
  console.log("  Student: rahul.verma@college.edu / student123 (High Risk)");
  console.log("  Student: aryan.sharma@college.edu / student123 (Low Risk)");
  console.log("  Student: ananya.desai@college.edu / student123 (Critical Risk)");
  console.log("========================================================\n");
}

main()
  .catch((e) => {
    console.error(e);
    process.exit(1);
  })
  .finally(async () => {
    await prisma.$disconnect();
  });
