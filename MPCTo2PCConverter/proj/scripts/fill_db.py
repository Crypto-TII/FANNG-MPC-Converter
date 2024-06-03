import time
from conf import run_sql, model, types_per_model, batch, batch_sd,\
dealer_ids, test_mode, db_engine, actor, get_material_size, taas
from queries import insert_to_model, insert_to_material_type, \
insert_to_types_per_model, insert_to_shares, dealer_insert_batch,\
super_dealer_insert_batch, insert_to_types_per_batch, select_from_model,\
select_from_material_type, select_from_batch, select_from_types_per_batch
from utils import get_connection, set_search_path
import sys
from create_dealer_dbs import create_dealers
from generate_materials import generate_dummy_materials

def create_super_dealer_db(database, sql_path, db=db_engine.postgres.value):
    """
    creates super_dealer db and fills the required tables
    """
    db_connection, cursor = get_connection(database, db)
    run_sql(sql_path, cursor, db_connection)
    set_search_path(cursor, database, db)
    for element in batch_sd:
        batch_id = element['batch_id']
        model_id = element['model_id']
        used = element['used']
        # Check if model exists in model table and insert if it doesn't exist
        # cursor.execute(f"SELECT * FROM model WHERE model_id = {model_id}")
        cursor.execute(f"{select_from_model} = {model_id}")
        model_result = cursor.fetchone()
        if model_result is None:
            cursor.execute(insert_to_model, (model_id, model[model_id]))
        cursor.execute(super_dealer_insert_batch, (batch_id, model_id, used))
        
    db_connection.commit()

def create_client_db(database, sql_path, db=db_engine.postgres.value):
    """
        Creates client's DB and fills the required tables 
    """
    db_connection, cursor = get_connection(database, db)
    run_sql(sql_path, cursor, db_connection)
    set_search_path(cursor, database, db)
    types_per_model_id = 1
    for element in batch:
        model_id = element['model_id']
        # Get materials info for the model_id given in batch.
        cursor.execute(f'{select_from_model} = {model_id}')
        model_result = cursor.fetchone()
        if model_result is None:
            cursor.execute(insert_to_model, (model_id, model[model_id]))
        all_materials = [item for item in types_per_model if item['model_id'] == model_id]
        for model_config in all_materials:
            material_type_id = model_config['material_type_id']
            material_type = model_config['material_type']
            num_materials = model_config['num_materials']
            # Insert material info in types_per_model.
            # Add material_type to material_type table
            cursor.execute(f"{select_from_material_type} = {material_type_id}")
            material_result = cursor.fetchone()
            if material_result is None:
                cursor.execute(insert_to_material_type,\
                                (material_type_id, material_type))
            if model_result is None:
                cursor.execute(insert_to_types_per_model,(types_per_model_id,\
                                                    model_id, material_type_id, num_materials))
                types_per_model_id += 1
        db_connection.commit()

def create_model_owner_db(database, sql_path, db=db_engine.postgres.value):
    """
        Creates MO's DB and fills the required tables!
    """
    db_connection, cursor = get_connection(database, db)
    run_sql(sql_path, cursor, db_connection)
    set_search_path(cursor, database, db)
    types_per_model_id = 1
    for element in batch:
        model_id = element['model_id']
        # Check if model exists in model table and insert if it doesn't exist
        cursor.execute(f"{select_from_model} = {model_id}")
        model_result = cursor.fetchone()
        if model_result is None:
            cursor.execute(insert_to_model, (model_id, model[model_id]))
        # Get materials info for the model_id given in batch.
        all_materials = [item for item in types_per_model if item['model_id'] == model_id]
        for model_config in all_materials:
            num_materials = model_config['num_materials']
            material_type_id = model_config['material_type_id']
            material_type = model_config['material_type']
            # Insert material info in types_per_model.
            # Add material type to material_type table
            cursor.execute(f"{select_from_material_type} = {material_type_id}")
            material_result = cursor.fetchone()
            if material_result is None:
                cursor.execute(insert_to_material_type,\
                                (material_type_id, material_type))
            if model_result is None:
                cursor.execute(insert_to_types_per_model,\
                               (types_per_model_id, model_id, material_type_id, num_materials))
                types_per_model_id += 1
        db_connection.commit()

def create_dealer_db(database, db=db_engine.postgres.value):
    """
        Creates Dealer's DB and fills the required tables!
    """
    dealer_id = database.split("_")[-1]
    db_connection, cursor = get_connection(actor.dealer.value, db, int(dealer_id))
    set_search_path(cursor, database, db)
    end_row = 1
    types_per_batch_id = 1
    types_per_model_id = 1
    for element in batch:
        model_id, batch_id, mac_key_share, shares_seed, mac_key_mask = (
            element['model_id'], element['batch_id'], element['mac_key_share'],
            element['shares_seed'], element['mac_key_mask']
        )
        # Check if model exists in model table and insert if it doesn't exist
        model_result = insert_model_if_not_exists(cursor, model_id)
        insert_batch_if_not_exists(cursor, batch_id, model_id, mac_key_share, shares_seed, mac_key_mask)

        # Get materials info for the model_id given in batch.
        all_materials = [item for item in types_per_model if item['model_id'] == model_id]
        order_in_batch = 1
        for model_config in all_materials:
            num_materials = model_config['num_materials']
            material_type_id = model_config['material_type_id']
            material_type = model_config['material_type']
            material_size = get_material_size(material_type, num_materials)
            # Insert material info in types_per_model.
            # Add material type to material_type table
            cursor.execute(f"{select_from_material_type} = {material_type_id}")
            material_result = cursor.fetchone()
            if material_result is None:
                cursor.execute(insert_to_material_type,\
                                (material_type_id, material_type))
            if model_result is None:
                cursor.execute(insert_to_types_per_model,\
                               (types_per_model_id, model_id, material_type_id, num_materials))
                types_per_model_id += 1
            # Insert data to types_per_batch table.
            data_to_insert = (types_per_batch_id, batch_id, material_type_id, end_row, \
                              end_row+material_size, order_in_batch)
            types_per_batch_id += 1
            cursor.execute(insert_to_types_per_batch, data_to_insert)
            order_in_batch += 1
            end_row += material_size
        
        # Fill shares table with dummy entries 
        fill_shares_table(cursor, batch_id, dealer_id)        
    db_connection.commit()

def fill_shares_table(cursor, batch_id, dealer_id):
    with open(f"../unittest/{test_mode}_test/dealer_{dealer_id}_shares.txt", 'r', encoding="utf-8") as f:
        lines = f.readlines()
        cursor.execute(f"{select_from_types_per_batch} = {batch_id}")
        result = cursor.fetchall()
        for element in result:
            start_row, end_row = element[3], element[4]
            for idx in range(start_row, end_row):
                share_id, share, mac_share = lines[idx-1].split()
                data_to_insert = (share_id, share, mac_share)
                cursor.execute(insert_to_shares, data_to_insert)

def insert_model_if_not_exists(cursor, model_id):
    cursor.execute(select_from_model+" = %s", (model_id,))
    model_result = cursor.fetchone()
    if model_result is None:
        cursor.execute(insert_to_model, (model_id, model[model_id]))
    return model_result

def insert_batch_if_not_exists(cursor, batch_id, model_id, mac_key_share, shares_seed, mac_key_mask):
    cursor.execute(f"{select_from_batch} = {batch_id}")
    result = cursor.fetchone()
    if result is None:
        cursor.execute(dealer_insert_batch, (batch_id, model_id, mac_key_share, shares_seed, mac_key_mask))

def preprocess_materials(db):
    create_dealers(db)
    print("Generating dummy data!")
    generate_dummy_materials()

def create_and_fill_db(db):
    start_time = time.time()
    if not taas:
        preprocess_materials(db)
        for id in dealer_ids:
            create_dealer_db(f'dealer_{id}', db)
    # Creation of all dbs in mysql
    db_str = None
    if db == db_engine.mysql.value:
        db_str = "mysql"
    elif db == db_engine.postgres.value:
        db_str = "postgresql"
    
    create_model_owner_db(actor.model_owner.value, f"../db_scripts/schema_files/model_owner_db_schema_{db_str}.sql", db)
    create_super_dealer_db(actor.super_dealer.value, f"../db_scripts/schema_files/super_dealer_db_schema_{db_str}.sql", db)
    create_client_db(actor.client.value, f"../db_scripts/schema_files/client_db_schema_{db_str}.sql", db)

    print(f"Created super_dealer, model_owner, client and dealer DB in {db_str} in {time.time()-start_time} seconds")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 scripts/fill_db.py <DB MODE>")
        sys.exit(1)

    db = str(sys.argv[1])
    create_and_fill_db(db)
