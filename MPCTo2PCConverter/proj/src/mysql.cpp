#include "mysql.h"
#include <cassert>

#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

MySQL::MySQL(const string &host, const string &user, const string &password,
             const string &database, int port) {
  driver = get_driver_instance();
  if (port != 3306) {
    connection = shared_ptr<sql::Connection>(
        driver->connect(host + ":" + std::to_string(port), user, password),
        [](sql::Connection *conn) { conn->close(); });
  } else {
    connection = shared_ptr<sql::Connection>(
        driver->connect(host, user, password),
        [](sql::Connection *conn) { conn->close(); });
  }
  if (!database.empty()) {
    connection->setSchema(database);
  }
}

void MySQL::execute(const string &cmd) const {
  auto statement = unique_ptr<sql::Statement>(connection->createStatement());
  statement->execute(cmd);
}

void MySQL::insert_one(const string &table, const vector<string> &columns,
                   const vector<string> &values) const {
  assert(columns.size() == values.size());
  string stmt = "INSERT INTO " + table + " (";
  for (auto &column : columns) {
    stmt += column + ",";
  }
  stmt.pop_back();
  stmt += ") VALUES (";
  for (auto &value : values) {
    stmt += "'" + value + "',";
  }
  stmt.pop_back();
  stmt += ")";
  execute(stmt);
}
void MySQL::insert_many(const string &table, const vector<string> &columns,
                   const vector<vector<string>> &allValues) const {
    assert(!allValues.empty() && columns.size() == allValues[0].size());
    string columnList;
    for (auto &column : columns) {
        columnList += column + ",";
    }
    columnList.pop_back();
    string valuesList;
    for (const auto &values : allValues) {
        string valueList;
        for (auto &value : values) {
            valueList += "'" + value + "',";
        }
        valueList.pop_back();
        valuesList += "(" + valueList + "),";
    }
    valuesList.pop_back();
    string stmt = "INSERT INTO " + table + " (" + columnList + ") VALUES " + valuesList;
    execute(stmt);
}

void MySQL::update(const string &table, const string &key, const string &value,
                   const vector<string> &columns,
                   const vector<string> &values) const {
  assert(columns.size() == values.size());
  string stmt = "UPDATE " + table + " SET ";
  for (int i = 0; i < columns.size(); i++) {
    stmt += columns[i] + " = '" + values[i] + "',";
  }
  stmt.pop_back();
  stmt += " WHERE " + key + " = '" + value + "'";
  execute(stmt);
}

void MySQL::select_db(const string &db) const { connection->setSchema(db); }

shared_ptr<sql::ResultSet> MySQL::execute_query(const string &query) const {
  auto statement = unique_ptr<sql::Statement>(connection->createStatement());
  auto result = shared_ptr<sql::ResultSet>(statement->executeQuery(query));
  return result;
}

shared_ptr<sql::ResultSet>
MySQL::get_rows(const string &table, int start_row, int end_row,
                const vector<string> &columns) const {
  string query = "SELECT";
  if (columns.empty()) {
    query += " *";
  } else {
    for (auto &column : columns) {
      query += " " + column + ",";
    }
    query.pop_back();
  }
  if (columns.empty())
    query += " FROM " + table + " ORDER BY id";
  else
    query += " FROM " + table + " ORDER BY "+table+"_id";
  if (start_row < 0) {
    query += " DESC ";
  }
  if (end_row != 0) {
    query += " LIMIT ";
    if (start_row < 0) {
      query += std::to_string(-1 - start_row) + "," +
               std::to_string(start_row - end_row);
    } else {
      query +=
          std::to_string(start_row-1) + "," + std::to_string(end_row - start_row);
    }
  }
  return execute_query(query);
}

shared_ptr<sql::ResultSet> MySQL::find_rows(const string &table,
                                            const string &column,
                                            const string &value) const {
  string query =
      "SELECT * FROM " + table + " WHERE " + column + " = '" + value + "'";
  return execute_query(query);
}

size_t MySQL::rows_count(shared_ptr<sql::ResultSet> result) const {
  return result->rowsCount();
}

int MySQL::count_rows(const std::string &table, const std::string &column,
                      const std::string &value) const {
  string query = "SELECT COUNT(*) FROM " + table;
  if (!column.empty()) {
    query += " WHERE " + column + " = '" + value + "'";
  }

  if (auto result = execute_query(query); result->next()) {
    return result->getInt(1);
  }
  return -1;
}

vector<vector<string>> MySQL::parse_result(shared_ptr<sql::ResultSet> result,
                                           const vector<string> &columns,
                                           int start_row, int end_row) const {
  vector<vector<string>> rows;
  int row = 0;
  for (; row < start_row; row++) {
    result->next();
  }
  for (; result->next() && (end_row == 0 || row < end_row); row++) {
    vector<string> row_data;
    for (const auto &column : columns) {
      row_data.push_back(result->getString(column));
    }
    rows.push_back(row_data);
  }
  return rows;
}