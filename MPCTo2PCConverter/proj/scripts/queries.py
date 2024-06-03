# generic insert statements
insert_to_model = "INSERT INTO model (model_id, model_name) VALUES (%s, %s)"
insert_to_material_type = "INSERT INTO material_type (material_type_id, material_type) VALUES (%s, %s)"
insert_to_types_per_model = "INSERT INTO types_per_model (types_per_model_id, model_id, material_type_id,\
                            num_materials) VALUES (%s, %s, %s, %s)"
insert_to_shares = "INSERT INTO share (share_id, share, mac_share) VALUES (%s, %s, %s)"
insert_to_types_per_batch = "INSERT INTO types_per_batch (types_per_batch_id, batch_id,\
                  material_type_id, start_row, end_row, order_in_batch) VALUES (%s, %s, %s, %s, %s, %s)"
insert_to_shares_mpclib = "INSERT INTO share (PLAYER, CHANNEL, SHARE, MAC_SHARE)\
      VALUES (%s, %s, %s, %s)"
# actor specific insert to batch
dealer_insert_batch = "INSERT INTO batch (batch_id, model_id, mac_key_share, shares_seed, mac_key_mask) \
                        VALUES (%s, %s, %s, %s, %s)"
super_dealer_insert_batch = "INSERT INTO batch (batch_id, model_id, used)\
                            VALUES (%s, %s, %s)"
# generic Select queries
select_from_model = "SELECT * FROM model WHERE model_id"
select_from_batch = "SELECT * FROM batch WHERE batch_id"
select_from_types_per_batch = "SELECT * FROM types_per_batch WHERE batch_id"
select_from_material_type = "SELECT * FROM material_type WHERE material_type_id"