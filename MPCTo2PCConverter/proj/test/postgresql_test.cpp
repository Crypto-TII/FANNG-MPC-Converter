#include <catch2/catch.hpp>
#include<yaml-cpp/yaml.h>

#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "../src/postgresql.h"  // Include your PostgreSQL header file
#include "../src/config_loader.h"
using std::string;

const string currentDir = std::filesystem::current_path().parent_path().string();
const YAML::Node all_config = ConfigLoader::loadConfig(currentDir+"/config/config.yaml");
const YAML::Node config = ConfigLoader::getModelOwnerConfig(all_config, 0);
const string psql_host = config["mo_postgres_host"].as<string>();
const int psql_port = config["mo_postgres_port"].as<int>();
const string psql_user = config["mo_postgres_user"].as<string>();
const string psql_password = config["mo_postgres_password"].as<string>();
const string psql_database = config["mo_postgres_database"].as<string>();
const string psql_schema = "test_schema";

TEST_CASE("PostgreSQL connection establishment") {
    PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);
    REQUIRE(db.connection->is_open());
}

TEST_CASE("PostgreSQL create schema") {
    PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);
    db.execute(psql_database, "BEGIN");
    db.execute(psql_database, R"(DROP SCHEMA IF EXISTS ")" + psql_schema + R"(" CASCADE)");
    db.execute(psql_database, R"(CREATE SCHEMA ")" + psql_schema + R"(")");
    db.execute(psql_database, "COMMIT");
    auto result = db.execute_query(psql_database, "SELECT schema_name FROM information_schema.schemata WHERE schema_name = '" + psql_schema + "'");
    REQUIRE(result->size() == 1);
}

TEST_CASE("PostgreSQL create table") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);
  db.execute(psql_schema, R"(DROP TABLE IF EXISTS ")" + psql_schema + R"(".table1)");
  db.execute(psql_schema, R"(CREATE TABLE IF NOT EXISTS ")" + psql_schema + R"(".table1 ( "id" SERIAL PRIMARY KEY, "name" VARCHAR(255)))");
  auto result = db.execute_query(psql_schema, "SELECT table_name FROM information_schema.tables WHERE table_schema = '"+psql_schema+"' AND table_name = 'table1'");
  REQUIRE(result->size() == 1);
}

TEST_CASE("PostgreSQL insert") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);

  // Insert data into the table
  db.insert_one(psql_schema, "table1", {"name"}, {"Alice"});
  db.insert_one(psql_schema, "table1", {"name"}, {"Bob"});
  db.insert_one(psql_schema, "table1", {"name"}, {"Charlie"});

  // Get the number of rows
  auto result = db.get_rows(psql_schema, "table1");
  size_t num_rows = db.rows_count(result);
  INFO("Number of rows: " << num_rows)
  REQUIRE(num_rows == 3);
}
TEST_CASE("PostgreSQL insert many") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);

  // Insert data into the table
  db.insert_many(psql_schema, "table1", {"name"}, {{"Abdel"}, {"Ajith"}, {"Aryan"}});

  // Get the number of rows
  auto result = db.get_rows(psql_schema, "table1");
  size_t num_rows = db.rows_count(result);
  INFO("Number of rows: " << num_rows)
  REQUIRE(num_rows == 6);
}

TEST_CASE("PostgreSQL update data") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);

  // Update data in the table
  db.update(psql_schema, "table1", "name", "Charlie", {"name"}, {"Eve"});

  // Get the updated data and check
  auto result = db.find_rows(psql_schema, "table1", "name", "Eve");
  REQUIRE(result->size() == 1);
}

TEST_CASE("PostgreSQL read data") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);

  // Read data from the table
  auto result = db.get_rows(psql_schema, "table1");
  REQUIRE(result->size() == 6);
}

TEST_CASE("PostgreSQL destroy schema") {
  PostgreSQL db(psql_host, psql_user, psql_password, psql_database, psql_port);

  // Drop the test schema
  db.execute(psql_database, R"(DROP SCHEMA IF EXISTS ")" + psql_schema + R"(" CASCADE)");

  // Check if the schema was deleted
  auto result = db.execute_query(psql_database, "SELECT schema_name FROM information_schema.schemata WHERE schema_name = '" + psql_schema + "'");
  REQUIRE(result->size() == 0);
}
