#include <catch2/catch.hpp>
#include <filesystem>
#include<yaml-cpp/yaml.h>

#include "../src/mysql.h"
#include "../src/config_loader.h"
#include <string>

using std::string;

const string currentDir = std::filesystem::current_path().parent_path().string();
const YAML::Node all_config = ConfigLoader::loadConfig(currentDir+"/config/config.yaml");
const YAML::Node config = ConfigLoader::getModelOwnerConfig(all_config, 0);
const string sql_host = config["mo_mysql_host"].as<string>();
const int sql_port = config["mo_mysql_port"].as<int>();
const string sql_user = config["mo_mysql_user"].as<string>();
const string sql_password = config["mo_mysql_password"].as<string>();

TEST_CASE("MySQL create db") {
  MySQL db(sql_host, sql_user, sql_password, "", sql_port);
  db.execute("DROP DATABASE IF EXISTS test_db");
  db.execute("CREATE DATABASE test_db");
  auto result = db.execute_query("SHOW DATABASES LIKE 'test_db'");
  REQUIRE(result->next());
}

TEST_CASE("MySQL create table") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  db.execute("DROP TABLE IF EXISTS table1");
  db.execute("CREATE TABLE table1 (id INTEGER NOT NULL AUTO_INCREMENT, "
             "name VARCHAR(255), PRIMARY KEY (id))");
  auto result = db.execute_query("SHOW TABLES LIKE 'table1'");
  REQUIRE(result->next());
}

TEST_CASE("MySQL insert many") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  db.insert_many("table1", {"name"}, {{"Abdel"}, {"Ajith"}, {"Aryan"}});
  auto result = db.get_rows("table1");
  size_t num_rows = db.rows_count(result);
  INFO("Number of rows: " << num_rows)
  REQUIRE(num_rows == 3);
}

TEST_CASE("MySQL insert one") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  db.insert_one("table1", {"name"}, {"Alice"});
  db.insert_one("table1", {"name"}, {"Bob"});
  db.insert_one("table1", {"name"}, {"Charlie"});
  auto result = db.get_rows("table1");
  size_t num_rows = db.rows_count(result);
  INFO("Number of rows: " << num_rows)
  REQUIRE(num_rows == 6);
}

TEST_CASE("MySQL update data") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  db.update("table1", "name", "Charlie", {"name"}, {"Eve"});
  auto result = db.get_last_row("table1");
  auto rows = db.parse_result(result, {"name"});
  INFO("returned row = " << rows[0][0])
  REQUIRE(rows[0][0] == "Eve");
}

TEST_CASE("MySQL read data") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  auto result = db.get_first_row("table1");
  auto rows = db.parse_result(result, {"name"});
  INFO("returned row = " << rows[0][0])
  REQUIRE(rows[0][0] == "Abdel"); // Because this is the first entry now
}

TEST_CASE("MySQL destroy db") {
  MySQL db(sql_host, sql_user, sql_password, "test_db", sql_port);
  db.execute("DROP DATABASE IF EXISTS test_db");
  auto result = db.execute_query("SHOW DATABASES LIKE 'test_db'");
  INFO("number of databases after deletion: " << result->rowsCount())
  REQUIRE(result->rowsCount() == 0);
}
