from enum import Enum

types_per_model = [
    {'model_id': 1, 'num_materials': 1000, 'material_type_id': 1, 'material_type': 'triple1'},
    {'model_id': 1, 'num_materials': 1000, 'material_type_id': 2, 'material_type': 'triple2'},
    {'model_id': 1, 'num_materials': 1000, 'material_type_id': 3, 'material_type': 'MATRIX_TRIPLE_64_64_64'},
    {'model_id': 1, 'num_materials': 1000, 'material_type_id': 4, 'material_type': 'MATRIX_TRIPLE_128_64_512'},
    {'model_id': 1, 'num_materials': 1000, 'material_type_id': 5, 'material_type': 'MATRIX_TRIPLE_128_128_128'},
    {'model_id': 2, 'num_materials': 10000, 'material_type_id': 1, 'material_type': 'triple1'},
    {'model_id': 2, 'num_materials': 10000, 'material_type_id': 2, 'material_type': 'triple2'},
    {'model_id': 2, 'num_materials': 10000, 'material_type_id': 3, 'material_type': 'MATRIX_TRIPLE_64_64_64'},
    {'model_id': 2, 'num_materials': 10000, 'material_type_id': 4, 'material_type': 'MATRIX_TRIPLE_128_64_512'},
    {'model_id': 2, 'num_materials': 10000, 'material_type_id': 5, 'material_type': 'MATRIX_TRIPLE_128_128_128'}
]
batch = [
    {'batch_id': '1', 'model_id': 1, 'mac_key_share': '1', 'shares_seed': "null", 'mac_key_mask': "null", 'shares_delivered': "0"},
    {'batch_id': '2', 'model_id': 2, 'mac_key_share': '1', 'shares_seed': "null", 'mac_key_mask': "null", 'shares_delivered': "0"},
    {'batch_id': '3', 'model_id': 1, 'mac_key_share': '1', 'shares_seed': "null", 'mac_key_mask': "null", 'shares_delivered': "0"},
    {'batch_id': '4', 'model_id': 2, 'mac_key_share': '1', 'shares_seed': "null", 'mac_key_mask': "null", 'shares_delivered': "0"},
]
model = { 1: 'model1',
         2: 'model2'}

taas = False
db_host_mysql = "localhost" if taas is False else "database-taas-converter"
db_user_mysql = "root"

db_host_postgres = "localhost"
db_user_postgres = "postgres"
db_database_postgres = "postgres"

class db_engine(Enum):
    mysql = 0
    postgres = 1

class actor(Enum):
    model_owner = "model_owner"
    client = "client"
    super_dealer = "super_dealer"
    dealer = "dealer"
