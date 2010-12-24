#ifndef OSMIUM_SQLITE_HPP
#define OSMIUM_SQLITE_HPP

#include <stdexcept>
#include <string>

#include <sqlite3.h>

/**
*
*  Namespace for Sqlite wrapper classes
*
*/
namespace Sqlite {

    /**
    *
    *  Exception returned by Sqlite wrapper classes when there are errors in the Sqlite3 lib
    *
    */
    class Exception : public std::runtime_error {
    public:
        Exception(const std::string &msg, const std::string &error) : std::runtime_error(msg + ": " + error + '\n') {
        }
    };

    class Statement;

    /**
    *
    *  Wrapper class for Sqlite database
    *
    */
    class Database {

    private:

        sqlite3 *db;

    public:

        Database(const char *filename) {
            if (sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0)) {
                sqlite3_close(db);
                throw Sqlite::Exception("Can't open database", errmsg());
            }
        }

        ~Database() {
            if (db) {
                sqlite3_close(db);
            }
        }

        const std::string &errmsg() const {
            static std::string error = std::string(sqlite3_errmsg(db));
            return error;
        }

        sqlite3 *get_sqlite3() {
            return db;
        }

        Statement *prepare(const char *sql);

        void begin_transaction();

        void commit();

        void close();

    }; // class Database

    /**
    *
    * Wrapper class for Sqlite prepared statement.
    *
    */
    class Statement {

    private:

        Database *db_;
        sqlite3_stmt *statement;

        int bindnum;

    public:

        Statement(Database *db, const char *sql) : db_(db), statement(0), bindnum(1) {
            sqlite3_prepare_v2(db->get_sqlite3(), sql, -1, &statement, 0);
            if (statement == 0) {
                throw Sqlite::Exception("Can't prepare statement", db_->errmsg());
            }
        }

        ~Statement() {
            sqlite3_finalize(statement);
        }

        Statement *bind_null() {
            if (SQLITE_OK != sqlite3_bind_null(statement, bindnum++)) {
                throw Sqlite::Exception("Can't bind null value", db_->errmsg());
            }
            return this;
        }

        Statement *bind_text(const char *value) {
            if (SQLITE_OK != sqlite3_bind_text(statement, bindnum++, value, -1, SQLITE_STATIC)) {
                throw Sqlite::Exception("Can't bind text value", db_->errmsg());
            }
            return this;
        }

        Statement *bind_int(int value) {
            if (SQLITE_OK != sqlite3_bind_int(statement, bindnum++, value)) {
                throw Sqlite::Exception("Can't bind int value", db_->errmsg());
            }
            return this;
        }

        Statement *bind_int64(int64_t value) {
            if (SQLITE_OK != sqlite3_bind_int64(statement, bindnum++, value)) {
                throw Sqlite::Exception("Can't bind int64 value", db_->errmsg());
            }
            return this;
        }

        Statement *bind_blob(const void *value, int length) {
            if (SQLITE_OK != sqlite3_bind_blob(statement, bindnum++, value, length, 0)) {
                throw Sqlite::Exception("Can't bind blob value", db_->errmsg());
            }
            return this;
        }

        void execute() {
            sqlite3_step(statement);
            if (SQLITE_OK != sqlite3_reset(statement)) {
                throw Sqlite::Exception("Can't execute statement", db_->errmsg());
            }
            bindnum = 1;
        }

    }; // class Statement

    inline Statement *Database::prepare(const char *sql) {
        return new Statement(this, sql);
    }

    inline void Database::begin_transaction() {
        prepare("BEGIN TRANSACTION;")->execute();
    }

    inline void Database::commit() {
        prepare("COMMIT;")->execute();
    }

    inline void Database::close() {
        if (db) {
            sqlite3_close(db);
            db = 0;
        }
    }

} // namespace Sqlite

#endif // OSMIUM_SQLITE_HPP
