#include "postgresql.h"
#include <cassert>
#include <pqxx/pqxx>

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::unique_ptr;
using std::vector;

PostgreSQL::PostgreSQL(const string &host, const string &user, const string &password, const string &database, int port) {
    connection = make_shared<pqxx::connection>("host=" + host + " port=" + std::to_string(port) + " user=" + user + " password=" + password + " dbname=" + database);}

void PostgreSQL::execute(const string &schema, const string &cmd) const {
    pqxx::work txn(*connection);
    txn.exec("set search_path to \""+schema+"\"");
    txn.exec(cmd);
    txn.commit();
}

void PostgreSQL::insert_one(const string &schema, const string &table, const vector<string> &columns, const vector<string> &values) const {
    assert(columns.size() == values.size());
    string stmt = "INSERT INTO \""+schema+"\".\"" + table + "\" (";
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
    execute(schema, stmt);
}
void PostgreSQL::insert_many(const string &schema, const string &table, const vector<string> &columns, const vector<vector<string>> &allValues) const {
    assert(!allValues.empty() && columns.size() == allValues[0].size());
    string columnList;
    for (auto &column : columns) {
        columnList += "\"" + column + "\",";
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
    string stmt = "INSERT INTO \"" + schema + "\".\"" + table + "\" (" + columnList + ") VALUES " + valuesList;
    execute(schema, stmt);
}


void PostgreSQL::update(const string &schema, const string &table, const string &key, const string &value, const vector<string> &columns, const vector<string> &values) const {
    assert(columns.size() == values.size());
    string stmt = "UPDATE \""+schema+"\".\"" + table + "\" SET ";
    for (int i = 0; i < columns.size(); i++) {
        stmt += columns[i] + " = '" + values[i] + "',";
    }
    stmt.pop_back();
    stmt += " WHERE " + key + " = '" + value + "'";
    execute(schema, stmt);
}

shared_ptr<pqxx::result> PostgreSQL::execute_query(const string &schema, const string &query) const {
    pqxx::work txn(*connection);
    txn.exec("set search_path to \""+schema+"\"");
    auto result = make_shared<pqxx::result>(txn.exec(query));
    return result;
}

shared_ptr<pqxx::result> PostgreSQL::get_rows(const string &schema, const string &table, int start_row, int end_row, const vector<string> &columns) const {
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
        query += " FROM  \""+schema+"\".\"" + table + "\" ORDER BY id";
    else
        query += " FROM  \""+schema+"\".\"" + table + "\" ORDER BY "+table+"_id";
    
    if (start_row < 0) {
        query += " DESC ";
    }
    if (end_row != 0) {
        query += " LIMIT ";
        if (start_row < 0) {
            query += std::to_string(-start_row) + " OFFSET " + std::to_string(-start_row - end_row);
        } else {
            query += std::to_string(end_row - start_row) + " OFFSET " + std::to_string(start_row-1);
        }
    }
    return execute_query(schema, query);
}

shared_ptr<pqxx::result> PostgreSQL::find_rows(const string &schema, const string &table, const string &column, const string &value) const {
    string query = "SELECT * FROM \""+schema+"\".\"" + table + "\" WHERE " + column + " = '" + value + "'";
    return execute_query(schema, query);
}

size_t PostgreSQL::rows_count(shared_ptr<pqxx::result> result) const {
    return result->size();
}

int PostgreSQL::count_rows(const string &schema, const std::string &table, const std::string &column, const std::string &value) const {
    string query = "SELECT COUNT(*) FROM  \""+schema+"\".\"" + table+"\"";
    if (!column.empty()) {
        query += " WHERE " + column + " = '" + value + "'";
    }
    auto result = execute_query(schema, query);
    if (int count = 0; !(result->at(0).at(0).to(count))){
        return count;
    }
    return -1;
}

vector<vector<string>> PostgreSQL::parse_result(shared_ptr<pqxx::result> result, const vector<string> &columns, int start_row, int end_row) const {
    vector<vector<string>> rows;
    int row = 0;
    for (; row < start_row; row++) {
        result->at(row);
    }
    for (; row < result->size() && (end_row == 0 || row < end_row); row++) {
        vector<string> row_data;
        for (const auto &column : columns) {
            row_data.push_back(result->at(row).at(column).as<string>());
        }
        rows.push_back(row_data);
    }
    return rows;
}