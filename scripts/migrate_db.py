import sqlite3
import os
import sys
import shutil
import datetime
import argparse

def create_backup(db_path):
    if not os.path.exists(db_path):
        return None
    backup_dir = os.path.join(os.path.dirname(db_path), 'backups')
    os.makedirs(backup_dir, exist_ok=True)
    timestamp = datetime.datetime.utcnow().strftime('%Y%m%d_%H%M%S')
    backup_path = os.path.join(backup_dir, f"attendance_backup_{timestamp}.db")
    shutil.copy2(db_path, backup_path)
    print(f"📦 Created timestamped database backup at: {backup_path}")
    return backup_path

def migrate(force=False):
    src_db = 'prisma/dev.db'
    dst_db = 'database/attendance.db'
    
    if not os.path.exists(src_db):
        print(f"❌ Error: Source database {src_db} does not exist.")
        sys.exit(1)
        
    os.makedirs('database', exist_ok=True)
    os.makedirs('database/seeds', exist_ok=True)
    
    if os.path.exists(dst_db):
        if not force:
            print(f"⚠️ Destination database {dst_db} already exists.")
            print("To safely migrate and overwrite, provide the --force or --overwrite flag.")
            sys.exit(1)
        backup_path = create_backup(dst_db)
    else:
        backup_path = None

    src = sqlite3.connect(src_db)
    dst = sqlite3.connect(dst_db)
    dst.execute("PRAGMA foreign_keys = ON;")

    try:
        # Atomic migration within a transaction
        with dst:
            # Read schema
            schema_file = 'database/migrations/001_initial_schema.sql'
            if os.path.exists(schema_file):
                with open(schema_file, 'r', encoding='utf-8') as f:
                    dst.executescript(f.read())
            
            # 1. Users
            users = src.execute('SELECT id, email, passwordHash, name, role, createdAt, updatedAt FROM User;').fetchall()
            for u in users:
                pw = u[2]
                if not (pw.startswith('$2a$') or pw.startswith('$2b$')):
                    print(f"⚠️ Notice: User {u[1]} has non-standard password hash. Password setup required.")
            dst.executemany('INSERT OR REPLACE INTO users (id, email, password_hash, name, role, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', users)

            # 2. Faculty
            faculty = src.execute('SELECT id, userId, employeeId, department, createdAt, updatedAt FROM Faculty;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO faculty (id, user_id, employee_id, department, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?);', faculty)

            # 3. Students
            students = src.execute('SELECT id, userId, rollNumber, department, semester, createdAt, updatedAt FROM Student;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO students (id, user_id, roll_number, department, semester, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', students)

            # 4. Subjects
            subjects = src.execute('SELECT id, code, name, facultyId, credits, createdAt, updatedAt FROM Subject;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO subjects (id, code, name, faculty_id, credits, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', subjects)

            # 5. Enrollments
            enrollments = src.execute('SELECT id, studentId, subjectId, createdAt FROM Enrollment;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO enrollments (id, student_id, subject_id, created_at) VALUES (?, ?, ?, ?);', enrollments)

            # 6. Sessions
            sessions = src.execute('SELECT id, subjectId, facultyId, sessionDate, topic, createdAt, updatedAt FROM AttendanceSession;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO attendance_sessions (id, subject_id, faculty_id, session_date, topic, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', sessions)

            # 7. Records
            records = src.execute('SELECT id, sessionId, studentId, status, remarks, createdAt, updatedAt FROM AttendanceRecord;').fetchall()
            dst.executemany('INSERT OR REPLACE INTO attendance_records (id, session_id, student_id, status, remarks, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?);', records)

        # Integrity Validation
        fk_errors = dst.execute("PRAGMA foreign_key_check;").fetchall()
        if fk_errors:
            raise RuntimeError(f"Foreign key violations detected during migration: {fk_errors}")

        print("==================================================================")
        print("✅ DATABASE MIGRATION COMPLETED SUCCESSFULLY")
        print("==================================================================")
        print(f"Users migrated:       {len(users)}")
        print(f"Faculty migrated:     {len(faculty)}")
        print(f"Students migrated:    {len(students)}")
        print(f"Subjects migrated:    {len(subjects)}")
        print(f"Enrollments migrated: {len(enrollments)}")
        print(f"Sessions migrated:    {len(sessions)}")
        print(f"Records migrated:     {len(records)}")
        print("Foreign key integrity: 100% Validated (0 errors)")

        # Export deterministic seed script
        with open('database/seeds/seed_data.sql', 'w', encoding='utf-8') as f:
            for line in dst.iterdump():
                f.write(f"{line}\n")
        print("Exported: database/seeds/seed_data.sql")
        print("==================================================================")

    except Exception as e:
        print(f"❌ Migration failed: {e}")
        if backup_path and os.path.exists(backup_path):
            print(f"Rolling back destination database from backup: {backup_path}")
            dst.close()
            shutil.copy2(backup_path, dst_db)
            print("Rollback complete.")
        sys.exit(1)
    finally:
        src.close()
        dst.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Transactional Database Migration with Backup Safety")
    parser.add_argument('--force', '--overwrite', dest='force', action='store_true', help="Confirm overwrite of existing database")
    args = parser.parse_args()
    migrate(force=args.force)
