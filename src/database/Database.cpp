#include "database/Database.hpp"
#include <iostream>
#include <sstream>

namespace attendance::db {

Database& Database::instance() {
    static Database s_instance;
    return s_instance;
}

Database::~Database() {
    close();
}

bool Database::open(const std::string& dbPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    close();

    int rc = sqlite3_open_v2(
        dbPath.c_str(),
        &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );

    if (rc != SQLITE_OK) {
        std::cerr << "SQLite open error: " << (db_ ? sqlite3_errmsg(db_) : "Failed to allocate") << std::endl;
        close();
        return false;
    }

    // Set busy timeout to 5 seconds to gracefully handle concurrent access
    sqlite3_busy_timeout(db_, 5000);

    // Enforce foreign key constraints
    execute("PRAGMA foreign_keys = ON;");

    // Enable WAL mode if not an in-memory database
    if (dbPath != ":memory:") {
        execute("PRAGMA journal_mode = WAL;");
        execute("PRAGMA synchronous = NORMAL;");
    }

    return true;
}

void Database::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (db_) {
        sqlite3_close_v2(db_);
        db_ = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!db_) return false;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            std::cerr << "SQLite exec error: " << errMsg << " on SQL: " << sql << std::endl;
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}

bool Database::runMigration(const std::string& schemaSql) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!db_) return false;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, schemaSql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            std::cerr << "Migration error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}

std::unique_ptr<Database::Statement> Database::prepare(const std::string& sql) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!db_) return nullptr;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite prepare error: " << sqlite3_errmsg(db_) << " on SQL: " << sql << std::endl;
        return nullptr;
    }
    return std::make_unique<Statement>(db_, stmt);
}

bool Database::beginTransaction() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return execute("BEGIN IMMEDIATE TRANSACTION;");
}

bool Database::commit() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return execute("COMMIT;");
}

bool Database::rollback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return execute("ROLLBACK;");
}

} // namespace attendance::db
