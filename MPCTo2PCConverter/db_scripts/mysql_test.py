import time
from conf import db_engine, db_host_mysql, db_user_mysql
from utils import measure_time, create_db, insert_shares
import mysql.connector

db_connection = mysql.connector.connect(
    host=db_host_mysql,
    user=db_user_mysql
)
cursor = db_connection.cursor()

# Creation time for dealer's DB in mysql

print("################### Creating dealer's Database ###################")
print(f'MySQL: Time taken to create dealer DB:\
      {measure_time(create_db, db_connection, cursor, "dealer_db_schema_mysql.sql", "dealer", db=db_engine.mysql.value)} seconds')
print(f'MySQL: Time taken to insert dealer shares:\
       {measure_time(insert_shares, db_connection, cursor, "dealer", db=db_engine.mysql.value)} seconds')
start_time = time.time()
cursor.execute("SELECT * FROM dealer.share")
rows = cursor.fetchall()
print(f"MySQL: Time taken to retrieve dealer's shares: {time.time() - start_time} seconds")

# Creation time for model_owner's DB in mysql

print("################### Creating model_owner's Database ###################")
print(f'MySQL: Time taken to create model_owner DB:\
      {measure_time(create_db, db_connection, cursor, "model_owner_db_schema_mysql.sql", "model_owner", db=db_engine.mysql.value)} seconds')
print(f'MySQL: Time taken to insert model_owner shares:\
       {measure_time(insert_shares, db_connection, cursor, "model_owner", db=db_engine.mysql.value)} seconds')
start_time = time.time()
cursor.execute("SELECT * FROM `model_owner`.share")
rows = cursor.fetchall()
print(f"MySQL: Time taken to retrieve model_owner's shares: {time.time() - start_time} seconds")

# Creation time for client's DB in mysql

print("################### Creating client's Database ###################")
print(f'MySQL: Time taken to create client DB:\
      {measure_time(create_db, db_connection, cursor, "client_db_schema_mysql.sql", "client", db=db_engine.mysql.value)} seconds')
print(f'MySQL: Time taken to insert client shares:\
       {measure_time(insert_shares, db_connection, cursor, "client", db=db_engine.mysql.value)} seconds')
start_time = time.time()
cursor.execute("SELECT * FROM `client`.share")
rows = cursor.fetchall()
print(f"MySQL: Time taken to retrieve client's shares: {time.time() - start_time} seconds")


cursor.close()
db_connection.close()
