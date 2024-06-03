import os
import psycopg2
import mysql.connector
from conf import db_host_mysql, db_host_postgres, db_user_mysql, db_user_postgres, db_database_postgres
from utils import run_sql

#drop db mysql
db_connection = mysql.connector.connect(
    host=db_host_mysql,
    user=db_user_mysql
)
cursor = db_connection.cursor()
path = os.path.join(os.getcwd(), "db_scripts/schema_files/delete_tables_mysql.sql")
run_sql(path, cursor, db_connection)

#drop db postgres
db_connection = psycopg2.connect(
    host=db_host_postgres,
    database=db_database_postgres,
    user=db_user_postgres
)
cursor = db_connection.cursor()
path = os.path.join(os.getcwd(), "db_scripts/schema_files/delete_tables_postgresql.sql")
run_sql(path, cursor, db_connection)