import time
from conf import db_database_postgres, db_host_postgres, db_user_postgres
from utils import measure_time, create_db, insert_shares
import psycopg2

db_connection = psycopg2.connect(
    host=db_host_postgres,
    database=db_database_postgres,
    user=db_user_postgres
)
cursor = db_connection.cursor()

# Creation time for dealer's DB in postgres

print("################### Creating dealer's Database ###################")
print(f'PostgreSQL: Time taken to create dealer DB:\
      {measure_time(create_db, db_connection, cursor, "dealer_db_schema_postgresql.sql", "dealer")} seconds')
print(f'PostgreSQL: Time taken to insert dealer shares:\
       {measure_time(insert_shares, db_connection, cursor, "dealer")} seconds')
start_time = time.time()
cursor.execute('SELECT * FROM "dealer"."share"')
rows = cursor.fetchall()
print(f"PostgreSQL: Time taken to retrieve dealer's shares: {time.time() - start_time} seconds")

# Creation time for model_owner's DB in postgres

print("################### Creating model_owner's Database ###################")
print(f'PostgreSQL: Time taken to create model_owner DB:\
      {measure_time(create_db, db_connection, cursor, "model_owner_db_schema_postgresql.sql", "model_owner")} seconds')
print(f'PostgreSQL: Time taken to insert model_owner shares:\
       {measure_time(insert_shares, db_connection, cursor, "model_owner")} seconds')
start_time = time.time()
cursor.execute('SELECT * FROM "model_owner".share')
rows = cursor.fetchall()
print(f"PostgreSQL: Time taken to retrieve model_owner's shares: {time.time() - start_time} seconds")

# Creation time for client's DB in postgres

print("################### Creating client's Database ###################")
print(f'PostgreSQL: Time taken to create client DB:\
      {measure_time(create_db, db_connection, cursor, "client_db_schema_postgresql.sql", "client")} seconds')
print(f'PostgreSQL: Time taken to insert client shares:\
       {measure_time(insert_shares, db_connection, cursor, "client")} seconds')
start_time = time.time()
cursor.execute('SELECT * FROM "client".share')
rows = cursor.fetchall()
print(f"PostgreSQL: Time taken to retrieve client's shares: {time.time() - start_time} seconds")


cursor.close()
db_connection.close()
