#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>

class PostgreSQL {
public:
    PostgreSQL(const std::string &host, const std::string &user, const std::string &password, const std::string &database, int port = 5432);
    void execute(const std::string &schema, const std::string &cmd) const;
    void insert_one(const std::string &schema, const std::string &table, const std::vector<std::string> &columns, const std::vector<std::string> &values) const;
    void insert_many(const std::string &schema, const std::string &table, const std::vector<std::string> &columns, const std::vector<std::vector<std::string>> &values) const;
    void update(const std::string &schema, const std::string &table, const std::string &key, const std::string &value, const std::vector<std::string> &columns, const std::vector<std::string> &values) const;
    std::shared_ptr<pqxx::result> execute_query(const std::string &schema, const std::string &query) const;
    std::shared_ptr<pqxx::result> get_rows(const std::string &schema, const std::string &table, int start_row = 0, int end_row = 0, const std::vector<std::string> &columns = {}) const;
    std::shared_ptr<pqxx::result> find_rows(const std::string &schema, const std::string &table, const std::string &column, const std::string &value) const;
    size_t rows_count(std::shared_ptr<pqxx::result> result) const;
    int count_rows(const std::string &schema, const std::string &table, const std::string &column = "", const std::string &value = "") const;
    std::vector<std::vector<std::string>> parse_result(std::shared_ptr<pqxx::result> result, const std::vector<std::string> &columns, int start_row = 0, int end_row = 0) const;
    std::shared_ptr<pqxx::connection> connection;
};

#endif // POSTGRESQL_H