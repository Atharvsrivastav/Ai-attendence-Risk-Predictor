import sqlite3
import os

def migrate():
    os.makedirs('database', exist_ok=True)
    os.makedirs('database/seeds', exist_ok=True)
    
    src_db = 'prisma/dev.db'
    dst_db = 'database/attendance.db'
    
    if os.path.exists(dst_db):
        os.remove(dst_db)
        
    src = sqlite3.connect(src_db)
    dst = sqlite3.connect(dst_db)
    
    # Read initial schema
    with open('database/migrations/001_initial_schema.sql', 'r', encoding='utf-8') as f:
        schema_sql = f.read()
    dst.executescript(schema_sql)
    
    # Copy Users
    users = src.execute('SELECT id, email, passwordHash, name, role, createdAt, updatedAt FROM User;').fetchall()
    dst.executemany('INSERT INTO users (id, email, password_hash, name, role, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', users)
    
    # Copy Faculty
    faculty = src.execute('SELECT id, userId, employeeId, department, createdAt, updatedAt FROM Faculty;').fetchall()
    dst.executemany('INSERT INTO faculty (id, user_id, employee_id, department, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?);', faculty)
    
    # Copy Students
    students = src.execute('SELECT id, userId, rollNumber, department, semester, createdAt, updatedAt FROM Student;').fetchall()
    dst.executemany('INSERT INTO students (id, user_id, roll_number, department, semester, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', students)
    
    # Copy Subjects
    subjects = src.execute('SELECT id, code, name, facultyId, credits, createdAt, updatedAt FROM Subject;').fetchall()
    dst.executemany('INSERT INTO subjects (id, code, name, faculty_id, credits, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', subjects)
    
    # Copy Enrollments
    enrollments = src.execute('SELECT id, studentId, subjectId, createdAt FROM Enrollment;').fetchall()
    dst.executemany('INSERT INTO enrollments (id, student_id, subject_id, created_at) VALUES (?, ?, ?, ?);', enrollments)
    
    # Copy AttendanceSessions
    sessions = src.execute('SELECT id, subjectId, facultyId, sessionDate, topic, createdAt, updatedAt FROM AttendanceSession;').fetchall()
    dst.executemany('INSERT INTO attendance_sessions (id, subject_id, faculty_id, session_date, topic, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', sessions)
    
    # Copy AttendanceRecords
    records = src.execute('SELECT id, sessionId, studentId, status, remarks, createdAt, updatedAt FROM AttendanceRecord;').fetchall()
    dst.executemany('INSERT INTO attendance_records (id, session_id, student_id, status, remarks, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', records)
    
    dst.commit()
    print(f"Successfully migrated {len(users)} users, {len(students)} students, {len(subjects)} subjects, and {len(records)} records into {dst_db}!")
    
    # Dump to seed_data.sql as well
    with open('database/seeds/seed_data.sql', 'w', encoding='utf-8') as f:
        for line in dst.iterdump():
            f.write(f"{line}\n")
    print("Exported database/seeds/seed_data.sql")
    
    src.close()
    dst.close()

if __name__ == '__main__':
    migrate()
