#ifndef OSMIUM_UTILS_SQLITE_HPP
#define OSMIUM_UTILS_SQLITE_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <stdexcept>
#include <string>
#include <iostream>

#include <sqlite3.h>

namespace Osmium {

    /**
    *  @brief The %Sqlite classes wrap the %Sqlite C library.
    */
    namespace Sqlite {

        /**
        *  Exception returned by Sqlite wrapper classes when there are errors in the Sqlite3 lib
        */
        class Exception : public std::runtime_error {

        public:

            Exception(const std::string &msg, const std::string &error) : std::runtime_error(msg + ": " + error + '\n') {
            }

        };

        class Statement;

        /**
        *  Wrapper class for Sqlite database
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
                sqlite3_close(db);
            }

            const std::string &errmsg() const {
                static std::string error = std::string(sqlite3_errmsg(db));
                return error;
            }

            sqlite3 *get_sqlite3() {
                return db;
            }

            void begin_transaction() {
                if (SQLITE_OK != sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0)) {
                    std::cerr << "Database error: " << sqlite3_errmsg(db) << "\n";
                    sqlite3_close(db);
                    exit(1);
                }
            }

            void commit() {
                if (SQLITE_OK != sqlite3_exec(db, "COMMIT;", 0, 0, 0)) {
                    std::cerr << "Database error: " << sqlite3_errmsg(db) << "\n";
                    sqlite3_close(db);
                    exit(1);
                }
            }

            Statement *prepare(const char *sql);

        }; // class Database

        /**
        * Wrapper class for Sqlite prepared statement.
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

    } // namespace Sqlite

} // namespace Osmium

#endif // OSMIUM_UTILS_SQLITE_HPP
