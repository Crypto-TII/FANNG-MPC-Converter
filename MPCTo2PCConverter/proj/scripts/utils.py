import mysql.connector
import psycopg2
import os
import yaml
from conf import model_owner_db, super_dealer_db, client_db, \
    dealer_db, mo_id, sd_id, client_id, dealer_ids,\
    db_engine, actor

def set_search_path(cursor, database, db):
    if db == db_engine.postgres.value:
        cursor.execute(f'SET search_path = "{database}"')
    elif db == db_engine.mysql.value:
        cursor.execute(f"USE `{database}`")

def get_connection_mysql(connection: str, dealer_id: int = 0):
    db_obj = None
    if connection == actor.model_owner.value:
        db_obj = mysql.connector.connect(
            host=model_owner_db[db_engine.mysql.value]["host"],
            user=model_owner_db[db_engine.mysql.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.client.value:
        db_obj = mysql.connector.connect(
            host=client_db[db_engine.mysql.value]["host"],
            user=client_db[db_engine.mysql.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.super_dealer.value:
        db_obj = mysql.connector.connect(
            host=super_dealer_db[db_engine.mysql.value]["host"],
            user=super_dealer_db[db_engine.mysql.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.dealer.value:
        db_obj = mysql.connector.connect(
            host=dealer_db[db_engine.mysql.value][dealer_id]["host"],
            user=dealer_db[db_engine.mysql.value][dealer_id]["user"],
        )
        cursor = db_obj.cursor()
    return db_obj, cursor

def get_connection_postgres(connection: str, dealer_id: int = 0):
    db_obj = None
    if connection == actor.model_owner.value:
        db_obj = psycopg2.connect(
            host=model_owner_db[db_engine.postgres.value]["host"],
            database=model_owner_db[db_engine.postgres.value]["user"],
            user=model_owner_db[db_engine.postgres.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.client.value:
        db_obj = psycopg2.connect(
            host=client_db[db_engine.postgres.value]["host"],
            database = client_db[db_engine.postgres.value]["user"],
            user=client_db[db_engine.postgres.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.super_dealer.value:
        db_obj = psycopg2.connect(
            host=super_dealer_db[db_engine.postgres.value]["host"],
            database=super_dealer_db[db_engine.postgres.value]["user"],
            user=super_dealer_db[db_engine.postgres.value]["user"],
        )
        cursor = db_obj.cursor()
    elif connection == actor.dealer.value:
        db_obj = psycopg2.connect(
            host=dealer_db[db_engine.postgres.value][dealer_id]["host"],
            database=dealer_db[db_engine.postgres.value][dealer_id]["user"],
            user=dealer_db[db_engine.postgres.value][dealer_id]["user"],
        )
        cursor = db_obj.cursor()
    return db_obj, cursor

def get_connection(connection: str, db: str, dealer_id: int = 0):
    if db == db_engine.mysql.value:
        return get_connection_mysql(connection, dealer_id)
    elif db == db_engine.postgres.value:
        return get_connection_postgres(connection, dealer_id)

def modify_config_per_test(https: str, db: str):
    with open(os.path.join(os.getcwd(), 'config/config.yaml'), 'r') as yaml_file:
        config = yaml.safe_load(yaml_file)
    for item in ["https", "db_engine"]:
        val = https if item == "https" else db
        val = bool(int(val)) if item == "https" else int(val)
        config["super_dealers"][sd_id][item] = val
        config["model_owners"][mo_id][item] = val
        config["clients"][client_id][item] = val
        for id in dealer_ids:
            config["dealers"][id][item] = val
    with open(os.path.join(os.getcwd(), 'config/config.yaml'), 'w') as yaml_file:
        yaml.dump(config, yaml_file)
    

