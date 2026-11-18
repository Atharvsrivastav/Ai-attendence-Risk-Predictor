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
        Statement(sqlite3_stmt* stmt) : stmt_(stmt) {}
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

        bool step() { return sqlite3_step(stmt_) == SQLITE_ROW; }

        int getInt(int col) const { return sqlite3_column_int(stmt_, col); }
        double getDouble(int col) const { return sqlite3_column_double(stmt_, col); }
        std::string getString(int col) const {
            const unsigned char* text = sqlite3_column_text(stmt_, col);
            return text ? reinterpret_cast<const char*>(text) : "";
        }
        bool isNull(int col) const { return sqlite3_column_type(stmt_, col) == SQLITE_NULL; }

    private:
        sqlite3_stmt* stmt_{nullptr};
    };

    std::unique_ptr<Statement> prepare(const std::string& sql);
    sqlite3* rawHandle() { return db_; }

    void beginTransaction();
    void commit();
    void rollback();

private:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* db_{nullptr};
    std::mutex mutex_;
};

} // namespace attendance::db
