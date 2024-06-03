import sys
import mysql
from conf import actor, db_engine, run_sql, mpclib_share_schema_path
from utils import get_connection, set_search_path
from queries import select_from_types_per_batch, insert_to_shares_mpclib

def fill_share_mpclib(batch_id, player, db):
    conn, cursor = get_connection(player, db)
    if player == actor.client.value:
        run_sql(mpclib_share_schema_path, cursor, conn)
    set_search_path(cursor, player, db)
    player_int = "0" if player == actor.client.value else "1"
    cursor.execute(f'{select_from_types_per_batch} = {batch_id}')
    types_per_batch_rows = cursor.fetchall()
    for row in types_per_batch_rows:
        set_search_path(cursor, player, db)
        material_type_id = row[2]
        start_row = row[3]
        end_row = row[4]
        limit = end_row - start_row
        select_query_start_to_end = f"SELECT * FROM share LIMIT \
            {limit} OFFSET {start_row-1};"
        cursor.execute(select_query_start_to_end)
        shares = cursor.fetchall()
        set_search_path(cursor, "scale_mamba", db)
        for share in shares:
            cursor.execute(insert_to_shares_mpclib, \
                                (player_int, material_type_id, share[1], share[2]))
    conn.commit()
    conn.close()

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("Usage: python share_converter_mpclib.py <BATCH ID> <DB MODE>")
        sys.exit(1)
    
    batch_id = str(sys.argv[1])
    db = str(sys.argv[2])

    fill_share_mpclib(batch_id, actor.client.value, db)
    fill_share_mpclib(batch_id, actor.model_owner.value, db)
    


