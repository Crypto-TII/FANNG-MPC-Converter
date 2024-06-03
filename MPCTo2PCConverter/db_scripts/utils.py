import time
from os import getcwd
from conf import batch, model, types_per_model, db_engine, actor
from queries import insert_to_model, insert_to_material_type, \
insert_to_types_per_model, insert_to_shares, dealer_insert_batch,\
client_insert_batch, insert_to_types_per_batch, select_from_model,\
select_from_material_type, select_from_batch, mo_insert_batch, \
select_from_types_per_batch

def set_search_path(cursor, database, db):
    if db == db_engine.postgres.value:
        cursor.execute(f'SET search_path = "{database}"')
    elif db == db_engine.mysql.value:
        cursor.execute(f"USE `{database}`")

def measure_time(func, *args, **kwargs) -> float:
    """
    Measure the execution time of a given function.
    Args:
        func (callable): The function to be executed and measured.
        *args: Variable arguments to be passed to the function.
        **kwargs: Keyword arguments to be passed to the function.
    Returns:
        float: The time taken to run the given function in seconds.
    """
    time_start = time.time()
    func(*args, **kwargs)  
    return time.time() - time_start

def run_sql(path, cursor, connection) -> None:
    """
    Runs sql script and creates schema
    Args:
        path : path for the sql file to be run.
        cursor: mysql.connector cursor object
        connection: mysql.connector connection object
    """
    with open(path, 'r', encoding="utf-8") as file:
        queries = file.read()
    query_list = queries.split(';')
    for query in query_list:
        query = query.strip()
        if query:
            cursor.execute(query)
    connection.commit()

def insert_to_batch(database, cursor, batch_id, model_id, mac_key_share,
                    shares_seed, mac_key_mask, shares_delivered):
    if database == actor.dealer.value:
        cursor.execute(dealer_insert_batch, \
                        (batch_id, model_id, mac_key_share, shares_seed, mac_key_mask))
    elif database == actor.model_owner.value:
        cursor.execute(mo_insert_batch, (batch_id, model_id, mac_key_share, shares_delivered))
    elif database == actor.client.value:
        cursor.execute(client_insert_batch, (batch_id, model_id, mac_key_share))

def create_db(db_connection, cursor, schema_path, database, db = db_engine.postgres.value):
    """
        Creates Dealer, Model Owner and Client's DB and fills the required tables 
        except for the shares tables
        Args:
            db_connection: The database connection object (PostgreSQL or MySQL).
            cursor: The database cursor object (PostgreSQL or MySQL).
            schema_path: The path to the schema SQL file (PostgreSQL or MySQL).
            database: The name of the database to be created.
            client: Indicates whether the client flag is set (default is False).
            postgres: Indicates whether PostgreSQL is used (default is True).
    """
    sql_path = getcwd()+ "/db_scripts/schema_files/"+schema_path
    run_sql(sql_path, cursor, db_connection)
    set_search_path(cursor, database, db)
    end_row = 0
    types_per_batch_id = 1
    types_per_model_id = 1
    for element in batch:
        model_id, batch_id, mac_key_share, shares_seed, mac_key_mask, shares_delivered = (
            element['model_id'], element['batch_id'], element['mac_key_share'],
            element['shares_seed'], element['mac_key_mask'], element['shares_delivered']
        )
        # Check if model exists in model table and insert if it doesn't exist
        cursor.execute(f"{select_from_model} = {model_id}")
        model_result = cursor.fetchone()
        if model_result is None:
            cursor.execute(insert_to_model, (model_id, model[model_id]))
        # Add data to batch table if it doesn't exist.
        cursor.execute(f"{select_from_batch} = {batch_id}")
        result = cursor.fetchone()
        if result is None:
            insert_to_batch(database, cursor, batch_id, model_id, mac_key_share, shares_seed, mac_key_mask, shares_delivered)
        # Get material info for the model_id given in batch.
        all_materials = [item for item in types_per_model if item['model_id'] == model_id]
        order_in_batch = 1
        for model_config in all_materials:
            num_materials = model_config['num_materials']
            material_type_id = model_config['material_type_id']
            material_type = model_config['material_type']
            # Add material type to material table
            cursor.execute(select_from_material_type+' = (%s)',\
                            (material_type_id,))
            material_result = cursor.fetchone()
            if material_result is None:
                # Insert material info in types_per_model.
                cursor.execute(insert_to_material_type,\
                                (material_type_id, material_type))
            if model_result is None:
                cursor.execute(insert_to_types_per_model,\
                               (types_per_model_id, model_id, material_type_id, num_materials))
                types_per_model_id += 1
            # Insert data to types_per_batch table.
            data_to_insert = (types_per_batch_id, batch_id, material_type_id, end_row, \
                              end_row+num_materials, order_in_batch)
            cursor.execute(insert_to_types_per_batch, data_to_insert)
            types_per_batch_id += 1
            order_in_batch += 1
            end_row += num_materials+1
            db_connection.commit()


def insert_shares(db_connection, cursor, database, db=db_engine.postgres.value):
    """
        Fills {database}'s shares table 
        Args:
            db_connection: database connection object
            cursor: The database cursor object.
            database: The name of the database where shares will be inserted.
            postgres: Indicates whether PostgreSQL is used (default is True).
    """
    set_search_path(cursor, database, db)
    share_id=1
    for batch_info in batch:
        batch_id = batch_info["batch_id"]
        with open(f"{getcwd()}/unittest/0p_test/P0-sample_matrix_triple.txt", 'r', encoding="utf-8") as f:
            lines = f.readlines()
            cursor.execute(f"{select_from_types_per_batch} = {batch_id}")
            result = cursor.fetchall()
            for element in result:
                start_row = element[3]
                end_row = element[4]
                for idx in range(start_row, end_row):
                    (_, share, mac_share) = lines[idx].split()
                    data_to_insert = (share_id, share, mac_share)
                    cursor.execute(insert_to_shares, data_to_insert)
                    share_id += 1
    db_connection.commit()
