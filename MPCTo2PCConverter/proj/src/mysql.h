#ifndef __MYSQL_H__
#define __MYSQL_H__

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <memory>
#include <string>
#include <vector>

class MySQL {
private:
  sql::Driver *driver;
  std::shared_ptr<sql::Connection> connection;

public:
  MySQL(const std::string& host, const std::string& user, 
        const std::string& password = "",
        const std::string& database = "", int port = 3306);
  /// @brief execute SQL command
  /// @param cmd
  void execute(const std::string& cmd) const;
  /// @brief execute SQL query
  /// @param query
  std::shared_ptr<sql::ResultSet> execute_query(const std::string& query) const;

  /// @brief parse result set to vector
  /// @param result - sql result set
  /// @param columns - vector of column names
  /// @param start_row - optional start row to filter set
  /// @param end_row - optional end row to filter set
  /// @return vector of vector of string (rows of columns)
  std::vector<std::vector<std::string>>
  parse_result(std::shared_ptr<sql::ResultSet> result,
               const std::vector<std::string> &columns, int start_row = 0,
               int end_row = 0) const;

  // convenience functions for common commands:
  /// @brief insert row to table
  /// @param table - table name
  /// @param columns - vector of column names
  /// @param values - vector of values
  void insert_one(const std::string& table, 
              const std::vector<std::string>& columns,
              const std::vector<std::string>& values) const;

  /// @brief update row in table
  /// @param table - table name
  /// @param key - column name of key
  /// @param value - value of key
  /// @param columns - columns to update
  /// @param values - new values
  void insert_many(const std::string& table, 
              const std::vector<std::string>& columns,
              const std::vector<std::vector<std::string>>& values) const;

  /// @brief update row in table
  /// @param table - table name
  /// @param key - column name of key
  /// @param value - value of key
  /// @param columns - columns to update
  /// @param values - new values
  void update(const std::string& table, const std::string& key, 
              const std::string& value, 
              const std::vector<std::string>& columns, 
              const std::vector<std::string>& values) const;

  /// @brief select database
  /// @param database
  void select_db(const std::string& db) const;

  /// @brief get rows from table in the range [start_row, end_row) optionally
  /// filter columns. If start_row is negative, the rows are returned in
  /// descending order (start_row=-1 ==> last row, -2 ==> next to last).
  /// @param table
  /// @param start_row inclusive starting from 0
  /// @param end_row exculsive (if both are 0, return all rows)
  /// @param columns
  std::shared_ptr<sql::ResultSet>
  get_rows(const std::string& table, int start_row=0, int end_row=0,
           const std::vector<std::string> &columns = {}) const;

  /// @brief get first row from table optionally filter columns
  /// @param table
  /// @param columns
  /// @return sql::ResultSet of first row
  std::shared_ptr<sql::ResultSet>
  get_first_row(const std::string& table,
                const std::vector<std::string> &columns = {}) const{
    auto result = get_rows(table, 1, 2, columns);
    return result;
  }

  /// @brief get last row from table optionally filter columns
  /// @param table
  /// @param columns
  /// @return sql::ResultSet of last row
  std::shared_ptr<sql::ResultSet>
  get_last_row(const std::string& table,
               const std::vector<std::string> &columns = {}) const{
    auto result = get_rows(table, -1, -2, columns);
    return result;
  }

  /// @brief return rows with matching value in column
  /// @param table table name
  /// @param column name of column to match
  /// @param value value to match
  /// @return sql::ResultSet of matching rows
  std::shared_ptr<sql::ResultSet>
  find_rows(const std::string& table, 
            const std::string& column, const std::string& value) const;

  /// @brief return number of rows in query result
  /// @param result - sql result set
  /// @return number of rows
  size_t rows_count(std::shared_ptr<sql::ResultSet> result) const;

  /// @brief return number of rows with matching value in column
  /// @param table table name
  /// @param column name of column to match (optional)
  /// @param value value to match (optional, unliess column is not empty)
  /// @return number of matching rows or -1 if none exist
  int
  count_rows(const std::string& table, const std::string& column="", 
             const std::string& value="") const;

};

#endif //__MYSQL_H__