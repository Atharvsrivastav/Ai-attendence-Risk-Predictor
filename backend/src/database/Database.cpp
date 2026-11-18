#include "database/Database.hpp"
#include <iostream>

namespace attendance::db {

Database& Database::instance() {
    static Database inst;
    return inst;
}

Database::~Database() {
    close();
}

bool Database::open(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database at " << dbPath << ": " 
                  << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    // Enable foreign keys and WAL mode for concurrent performance
    char* err = nullptr;
    sqlite3_exec(db_, "PRAGMA foreign_keys = ON; PRAGMA journal_mode = WAL;", nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite execute error: " << (errMsg ? errMsg : "unknown") << std::endl;
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::runMigration(const std::string& schemaSql) {
    return execute(schemaSql);
}

std::unique_ptr<Database::Statement> Database::prepare(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) return nullptr;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQLite prepare error: " << sqlite3_errmsg(db_) 
                  << " for SQL: " << sql << std::endl;
        return nullptr;
    }
    return std::make_unique<Statement>(stmt);
}

void Database::beginTransaction() {
    execute("BEGIN TRANSACTION;");
}

void Database::commit() {
    execute("COMMIT;");
}

void Database::rollback() {
    execute("ROLLBACK;");
}

} // namespace attendance::db
