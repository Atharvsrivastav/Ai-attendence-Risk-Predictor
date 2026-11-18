#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>
#include "sqlite3.h"
#include "json.hpp"

namespace attendance::db {

class Database {
public:
    static Database& instance();

    bool open(const std::string& dbPath);
    void close();

    bool execute(const std::string& sql);
    bool runMigration(const std::string& schemaSql);

    // RAII statement wrapper
    class Statement {
    public:
        Statement(sqlite3* db, sqlite3_stmt* stmt) : db_(db), stmt_(stmt) {}
        ~Statement() {
            if (stmt_) {
                sqlite3_finalize(stmt_);
            }
        }
        sqlite3_stmt* get() const { return stmt_; }

        void bind(int index, int val) { sqlite3_bind_int(stmt_, index, val); }
        void bind(int index, double val) { sqlite3_bind_double(stmt_, index, val); }
        void bind(int index, const std::string& val) { 
            sqlite3_bind_text(stmt_, index, val.c_str(), -1, SQLITE_TRANSIENT); 
        }
        void bindNull(int index) { sqlite3_bind_null(stmt_, index); }

        // Returns true if a row is ready to read (SQLITE_ROW)
        bool step() {
            int rc = sqlite3_step(stmt_);
            lastRc_ = rc;
            return rc == SQLITE_ROW;
        }

        // Returns true if command finished successfully (SQLITE_DONE or SQLITE_ROW)
        bool execute() {
            int rc = sqlite3_step(stmt_);
            lastRc_ = rc;
            return rc == SQLITE_DONE || rc == SQLITE_ROW;
        }

        int stepRaw() {
            int rc = sqlite3_step(stmt_);
            lastRc_ = rc;
            return rc;
        }

        int lastReturnCode() const { return lastRc_; }
        std::string lastErrorMessage() const {
            return db_ ? sqlite3_errmsg(db_) : "";
        }

        int getInt(int col) const { return sqlite3_column_int(stmt_, col); }
        double getDouble(int col) const { return sqlite3_column_double(stmt_, col); }
        std::string getString(int col) const {
            const unsigned char* text = sqlite3_column_text(stmt_, col);
            return text ? reinterpret_cast<const char*>(text) : "";
        }
        bool isNull(int col) const { return sqlite3_column_type(stmt_, col) == SQLITE_NULL; }

    private:
        sqlite3* db_{nullptr};
        sqlite3_stmt* stmt_{nullptr};
        int lastRc_{SQLITE_OK};
    };

    std::unique_ptr<Statement> prepare(const std::string& sql);
    sqlite3* rawHandle() { return db_; }

    void beginTransaction();
    void commit();
    void rollback();

    int lastErrorCode() const { return db_ ? sqlite3_errcode(db_) : 0; }
    std::string lastErrorMessage() const { return db_ ? sqlite3_errmsg(db_) : "No database connection"; }
    long long lastInsertRowId() const { return db_ ? sqlite3_last_insert_rowid(db_) : 0; }

    // RAII transaction guard
    class ScopedTransaction {
    public:
        explicit ScopedTransaction(Database& db) : db_(db), active_(true) {
            db_.beginTransaction();
        }
        ~ScopedTransaction() {
            if (active_) {
                db_.rollback();
            }
        }
        void commit() {
            if (active_) {
                db_.commit();
                active_ = false;
            }
        }
    private:
        Database& db_;
        bool active_;
    };

private:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* db_{nullptr};
    std::recursive_mutex mutex_;
};

} // namespace attendance::db
