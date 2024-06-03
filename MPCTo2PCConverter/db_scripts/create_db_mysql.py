import mysql.connector
from utils import run_sql
from os import getcwd
from conf import db_host_mysql, db_user_mysql

db_connection = mysql.connector.connect(
    host=db_host_mysql,
    user=db_user_mysql
)
cursor = db_connection.cursor()

sql_path = getcwd()+ "/db_scripts/schema_files/"
# Create Dealer DB MySQL
run_sql(sql_path+"dealer_db_schema_mysql.sql", cursor, db_connection)

# Create Model OWner DB MySQL
run_sql(sql_path+"model_owner_db_schema_mysql.sql", cursor, db_connection)

# Create Client DB MySQL
run_sql(sql_path+"client_db_schema_mysql.sql", cursor, db_connection)

# Create Super Dealer DB MySQL
run_sql(sql_path+"super_dealer_db_schema_mysql.sql", cursor, db_connection)

print("Created All DBs in MySQL")
