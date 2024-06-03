import psycopg2
from utils import run_sql
from os import getcwd
from conf import db_host_postgres, db_user_postgres, db_database_postgres

db_connection = psycopg2.connect(
    host=db_host_postgres,
    database=db_database_postgres,
    user=db_user_postgres
)
cursor = db_connection.cursor()

sql_path = getcwd()+ "/db_scripts/schema_files/"
# Create Dealer DB PostgreSQL
run_sql(sql_path+"dealer_db_schema_postgresql.sql", cursor, db_connection)

# Create Model OWner DB PostgreSQL
run_sql(sql_path+"model_owner_db_schema_postgresql.sql", cursor, db_connection)

# Create Client DB PostgreSQL
run_sql(sql_path+"client_db_schema_postgresql.sql", cursor, db_connection)

# Create Super Dealer DB PostgreSQL
run_sql(sql_path+"super_dealer_db_schema_postgresql.sql", cursor, db_connection)

print("Created All DBs in PostgreSQL")

