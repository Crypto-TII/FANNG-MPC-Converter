from os import getcwd
from utils import run_sql, measure_time
import mysql.connector
import psycopg2
from conf import batch, model, db_host_postgres, db_database_postgres, db_user_postgres,\
    db_user_mysql, db_host_mysql 

# MySQL connection
mysql_connection = mysql.connector.connect(
    host=db_host_mysql,
    user=db_user_mysql
)
mysql_cursor = mysql_connection.cursor()

def create_super_dealer_db_mysql():
    """
        Creates Super Dealer's DB and fills the required tables 
        except for the shares tables
    """
    sql_path = getcwd()+\
        "/db_scripts/schema_files/super_dealer_db_schema_mysql.sql"
    run_sql(sql_path, mysql_cursor, mysql_connection)
    for element in batch:
        batch_id = element['batch_id']
        model_id = element['model_id']
        # Check if model exists in Super Dealer's model table and insert if it doesn't exist
        mysql_cursor.execute(f"SELECT * FROM `super_dealer`.model WHERE model_id = {model_id}")
        model_result = mysql_cursor.fetchone()
        if model_result is None:
            mysql_cursor.execute("INSERT INTO `super_dealer`.model (model_id, model_name) \
                                 VALUES (%s, %s)", (model_id, model[model_id]))
        # Add data to Super Dealer's batch table if it doesn't exist.
        mysql_cursor.execute(f"SELECT * FROM `super_dealer`.batch WHERE batch_id = '{batch_id}'")
        result = mysql_cursor.fetchone()
        mysql_connection.commit()
        if result is None:
            mysql_cursor.execute("INSERT INTO `super_dealer`.batch (model_id, batch_id, used)\
                                 VALUES (%s, %s, %s)", (model_id, batch_id, 0))
    mysql_connection.commit()

# Creation time for Super Dealer's DB in mysql
print(f"MySQL: Time taken to create Dealer's DB:\
       {measure_time(create_super_dealer_db_mysql)} seconds")

mysql_cursor.close()
mysql_connection.close()

# PostgreSQL connection
postgres_connection = psycopg2.connect(
    host=db_host_postgres,
    database=db_user_postgres,
    user=db_database_postgres
)
postgres_cursor = postgres_connection.cursor()

def create_super_dealer_db_postgres():
    """
        Creates Super Dealer's DB and fills the required tables 
        except for the shares tables
    """
    sql_path = getcwd()+\
        "/db_scripts/schema_files/super_dealer_db_schema_postgresql.sql"
    run_sql(sql_path, postgres_cursor, postgres_connection)
    for element in batch:
        batch_id = element['batch_id']
        model_id = element['model_id']
        # Check if model exists in Super Dealer's model table and insert if it doesn't exist
        postgres_cursor.execute(f'SELECT * FROM "super_dealer"."model" \
                                WHERE model_id = {model_id}')
        model_result = postgres_cursor.fetchone()
        if model_result is None:
            postgres_cursor.execute('INSERT INTO "super_dealer"."model" (model_id, model_name)\
                                     VALUES (%s, %s)', (model_id, model[model_id]))
        # Add data to Super Dealer's batch table if it doesn't exist.
        postgres_cursor.execute('SELECT * FROM "super_dealer"."batch" WHERE\
                                 "batch_id" = %s', (batch_id,))
        result = postgres_cursor.fetchone()
        if result is None:
            postgres_cursor.execute('INSERT INTO "super_dealer"."batch" (model_id, batch_id, used)\
                            VALUES (%s, %s, %s)', (model_id, batch_id, 0))
    postgres_connection.commit()

# Insertion time for Super Dealer Shares in mysql
print(f"PostgreSQL: Time taken to create Dealer's DB:\
       {measure_time(create_super_dealer_db_postgres)} seconds")

postgres_cursor.close()
postgres_connection.close()
