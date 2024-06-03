from conf import num_dealers, actor, db_engine
from utils import get_connection

def create_dealers(db):
    if db == db_engine.mysql.value:
        db_connection, cursor = get_connection(actor.dealer.value, db_engine.mysql.value)

        for schema_index in range(num_dealers): 
            schema_name = f"dealer_{schema_index}"

            drop_schema_query = f"DROP SCHEMA IF EXISTS `{schema_name}`;"
            cursor.execute(drop_schema_query)

            create_schema_query = f"CREATE SCHEMA IF NOT EXISTS `{schema_name}`;"
            cursor.execute(create_schema_query)

            copy_table_queries = [
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`model` LIKE `dealer`.`model`;",
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`batch` LIKE `dealer`.`batch`;",
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`share` LIKE `dealer`.`share`;",
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`material_type` LIKE `dealer`.`material_type`;",
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`types_per_batch` LIKE `dealer`.`types_per_batch`;",
                f"CREATE TABLE IF NOT EXISTS `{schema_name}`.`types_per_model` LIKE `dealer`.`types_per_model`;"
            ]
            for query in copy_table_queries:
                cursor.execute(query)

        db_connection.commit()
        cursor.close()
        db_connection.close()

    elif db == db_engine.postgres.value:
        db_connection, cursor = get_connection(actor.dealer.value, db_engine.postgres.value)

        for schema_index in range(num_dealers): 
            schema_name = f"dealer_{schema_index}"

            drop_schema_query = f"DROP SCHEMA IF EXISTS {schema_name} CASCADE;"
            cursor.execute(drop_schema_query)

            create_schema_query = f"CREATE SCHEMA IF NOT EXISTS {schema_name};"
            cursor.execute(create_schema_query)

            copy_table_queries = [
                f"CREATE TABLE {schema_name}.model AS TABLE dealer.model WITH NO DATA;",
                f"CREATE TABLE {schema_name}.batch AS TABLE dealer.batch WITH NO DATA;",
                f"CREATE TABLE {schema_name}.share AS TABLE dealer.share WITH NO DATA;",
                f"CREATE TABLE {schema_name}.material_type AS TABLE dealer.material_type WITH NO DATA;",
                f"CREATE TABLE {schema_name}.types_per_batch AS TABLE dealer.types_per_batch WITH NO DATA;",
                f"CREATE TABLE {schema_name}.types_per_model AS TABLE dealer.types_per_model WITH NO DATA;"
            ]
            for query in copy_table_queries:
                cursor.execute(query)
        db_connection.commit()
        cursor.close()
        db_connection.close()
