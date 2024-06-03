import os
import yaml
import subprocess
from enum import Enum
# change test mode here for 0p, 3p or 5p testing
test_mode = "3p"
taas = False

if taas == True and test_mode != "3p":
    raise ValueError("Taas only supported with test_mode = 3p")

if taas:
    subprocess.run(["cp", "../unittest/3p_test/config_taas.yaml", "./config/config.yaml"])
else:
    subprocess.run(["cp", f"../unittest/{test_mode}_test/config.yaml", "./config/"])

with open(os.path.join(os.getcwd(), 'config/config.yaml'), 'r') as yaml_file:
    config = yaml.safe_load(yaml_file)

seed=42
#actor ids
mo_id = 0
sd_id = 0
client_id = 0
# secret that the client wants to share
secret = 1
# Model Id being used in functional testing
model_id = config['clients'][0]['model_id']
# Modulo
P =  170141183460469231731687303715887185921

materials_lower_bound = -100
materials_upper_bound = 100
# Max number of retries while requesting a server
MAX_RETRIES = 5

num_dealers = config["clients"][0]["num_dealers"]

# relative path to directory containing executable, ssl keys and certificates
build_dir = os.path.join(os.getcwd(), "build")

# mpclib shares schema mysql path
mpclib_share_schema_path = os.path.abspath(os.path.join(os.getcwd(), os.pardir))+\
"/db_scripts/schema_files/mpclib-share-schema.sql"

# executable filenames
super_dealer_exe = os.path.join(build_dir, "super_dealer")
mo_exe = os.path.join(build_dir, "model_owner")
client_exe = os.path.join(build_dir, "client")
dealer_exe = os.path.join(build_dir, "dealer")
# database parameters
super_dealer_db = {'0':{'host': config["super_dealers"][0]["sd_mysql_host"], 'port': config["super_dealers"][0]["sd_mysql_port"], 
                        'user': config["super_dealers"][0]["sd_mysql_user"], 'password': config["super_dealers"][0]["sd_mysql_password"]},
                '1':{'host': config["super_dealers"][0]["sd_postgres_host"], 'port': config["super_dealers"][0]["sd_postgres_port"], 
                     'user': config["super_dealers"][0]["sd_postgres_user"], 'password': config["super_dealers"][0]["sd_postgres_password"]}}
model_owner_db = {'0':{'host': config["model_owners"][0]["mo_mysql_host"], 'port': config["model_owners"][0]["mo_mysql_port"], 
                       'user': config["model_owners"][0]["mo_mysql_user"], 'password': config["model_owners"][0]["mo_mysql_password"]},
                '1':{'host': config["model_owners"][0]["mo_postgres_host"], 'port': config["model_owners"][0]["mo_postgres_port"], 
                     'user': config["model_owners"][0]["mo_postgres_user"], 'password': config["model_owners"][0]["mo_postgres_password"]}}
client_db = {'0':{'host': config["clients"][0]["client_mysql_host"], 'port': config["clients"][0]["client_mysql_port"], 
                  'user': config["clients"][0]["client_mysql_user"], 'password': config["clients"][0]["client_mysql_password"]},
                '1':{'host': config["clients"][0]["client_postgres_host"], 'port': config["clients"][0]["client_postgres_port"], 
                     'user': config["clients"][0]["client_postgres_user"], 'password': config["clients"][0]["client_postgres_password"]}}

dealer_db = {'0': [{'host': config["dealers"][i]['mysql_host'],  'port': config["dealers"][i]['mysql_port'], 
                    'user': config["dealers"][i]['mysql_user'], 'password': config["dealers"][i]['mysql_pwd']} 
                    for i in range(num_dealers)],
             '1': [{'host': config["dealers"][i]['postgres_host'], 'port': config["dealers"][i]['postgres_port'], 
                    'user': config["dealers"][i]['postgres_user'], 'password': config["dealers"][i]['postgres_pwd']} 
                    for i in range(num_dealers)]}
if taas:
    dealer_db = {'0': [{'container_name': config["taas_dealers"][i]["container_name"],'host': config["taas_dealers"][i]['mysql_host'], 
                            'port': config["taas_dealers"][i]['mysql_port'], 'user': config["taas_dealers"][i]['mysql_user'], 
                            'password': config["taas_dealers"][i]['mysql_pwd']} for i in range(3)]}

dealer_ids_str = config["clients"][0]["dealer_ids_str"]
dealer_ids = list(map(int, dealer_ids_str.split()))
grpc_hosts = [config["dealers"][i]['host'] for i in range(num_dealers)]
grpc_ports = [config["dealers"][i]['port'] for i in range(num_dealers)]
grpc_certs = [config["dealers"][i]['grpc_cert'] for i in range(num_dealers)]
grpc_privkey = [config["dealers"][i]['grpc_privkey'] for i in range(num_dealers)]

dealer_rest_hosts = [config["dealers"][i]['rest_host'] for i in range(num_dealers)]
dealer_rest_ports = [config["dealers"][i]['rest_port'] for i in range(num_dealers)]
dealer_rest_certs = [config["dealers"][i]['rest_cert'] for i in range(num_dealers)]
dealer_rest_privkey = [config["dealers"][i]['rest_privkey'] for i in range(num_dealers)]

model_owner_host = config["model_owners"][0]["mo_host"]
model_owner_port = config["model_owners"][0]["mo_port"]
model_owner_cert = config["model_owners"][0]["mo_cert"]
model_owner_privkey = config["model_owners"][0]["mo_privkey"]

super_dealer_host = config["super_dealers"][0]["sd_host"]
super_dealer_port = config["super_dealers"][0]["sd_port"]
super_dealer_cert = config["super_dealers"][0]["sd_cert"]
super_dealer_privkey = config["super_dealers"][0]["sd_privkey"]

batch_sd = [
    {'batch_id': '1', 'model_id': 1, 'used': '1'},
    {'batch_id': '2', 'model_id': 2, 'used': '0'},
    {'batch_id': '3', 'model_id': 1, 'used': '0'},
    {'batch_id': '4', 'model_id': 2, 'used': '1'},
]
batch = [
    {'batch_id': '1', 'model_id': 1, 'mac_key_share': '1', 'shares_seed': 'null', 'mac_key_mask': 'null', 'shares_delivered': '0'},
    {'batch_id': '2', 'model_id': 2, 'mac_key_share': '1', 'shares_seed': 'null', 'mac_key_mask': 'null', 'shares_delivered': '0'},
    {'batch_id': '3', 'model_id': 1, 'mac_key_share': '1', 'shares_seed': 'null', 'mac_key_mask': 'null', 'shares_delivered': '0'},
    {'batch_id': '4', 'model_id': 2, 'mac_key_share': '1', 'shares_seed': 'null', 'mac_key_mask': 'null', 'shares_delivered': '0'},
]
model = { 1: 'model1',
         2: 'model2'}

types_per_model = [
    {'model_id': 1, 'num_materials': 1, 'material_type_id': 1, 'material_type': 'TRIPLE_BEAVER'},
    {'model_id': 1, 'num_materials': 1, 'material_type_id': 2, 'material_type': 'TRIPLE_MATRIX_64_64_64'},
    {'model_id': 1, 'num_materials': 1, 'material_type_id': 3, 'material_type': 'TRIPLE_MATRIX_128_64_512'},
    {'model_id': 1, 'num_materials': 1, 'material_type_id': 4, 'material_type': 'TRIPLE_MATRIX_128_128_128'},
    {'model_id': 1, 'num_materials': 10, 'material_type_id': 70000, 'material_type': 'BEAVER'},
    {'model_id': 1, 'num_materials': 2, 'material_type_id': 777777, 'material_type': 'LENET11_25_2_9_4_9_2'},
    {'model_id': 1, 'num_materials': 2, 'material_type_id': 777778, 'material_type': 'PRUNEDRESNET_5_5_2_3_3_2_1_0'},
    {'model_id': 2, 'num_materials': 5, 'material_type_id': 1, 'material_type': 'TRIPLE_BEAVER'},
    {'model_id': 2, 'num_materials': 2, 'material_type_id': 2, 'material_type': 'TRIPLE_MATRIX_64_64_64'},
    {'model_id': 2, 'num_materials': 1, 'material_type_id': 3, 'material_type': 'TRIPLE_MATRIX_128_64_512'},
    {'model_id': 2, 'num_materials': 1, 'material_type_id': 4, 'material_type': 'TRIPLE_MATRIX_128_128_128'},
]

def get_nn_triple_dimensions(w, h, s, kh, kw, s_, stride, padding):
    w_in = w + (padding * 2)
    h_in = h + (padding * 2)
    w_out = (w_in - kw + 1)
    h_out = (h_in - kh + 1)
    if stride == 2:
        w_out_p = (w_out // stride)
        h_out_p = (h_out // stride)
        if (w_in % stride) > 0:
            w_out_p += 1
        if (h_in % stride) > 0:
            h_out_p += 1
        w_out = w_out_p
        h_out = h_out_p
    A = [w_in * h_in, s]
    B = [kh * kw, s * s_]
    C = [w_out * h_out, s_]
    return A, B, C

def get_material_size(material_name, material_count):
    name = material_name.split("_")
    if name[0] == "BEAVER" or name[1] == "BEAVER":
        return 3*material_count
    elif name[1] == "MATRIX":
        a, b, c = int(name[-3]), int(name[-2]), int(name[-1])  
        return (a*b + b*c + a*c)*material_count
    elif name[0] == "LENET11":
        a, b, c, d, e, f = int(name[-6]), int(name[-5]), int(name[-4]), int(name[-3]), int(name[-2]), int(name[-1])
        return (a*b + c*d + e*f)*material_count
    elif name[0] == "PRUNEDRESNET":
        w, h, s, kh, kw = int(name[-8]), int(name[-7]), int(name[-6]), int(name[-5]), int(name[-4])
        s_, stride, padding = int(name[-3]), int(name[-2]), int(name[-1]),
        a, b, c = get_nn_triple_dimensions(w, h, s, kh, kw, s_, stride, padding)
        return (a[0]*a[1] + b[0]*b[1] + c[0]*c[1])*material_count
    else:
        return 1*material_count

# number of materials generated for dynamic dealers
num_rows = sum(get_material_size(entry['material_type'], entry['num_materials']) for entry in types_per_model)*2

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
    # Split queries using semicolon as delimiter
    query_list = queries.split(';')
    for query in query_list:
        query = query.strip()
        if query:
            cursor.execute(query)
    connection.commit()

class db_engine(Enum):
    mysql = "0"
    postgres = "1"

class actor(Enum):
    model_owner = "model_owner"
    client = "client"
    super_dealer = "super_dealer"
    dealer = "dealer"

class protocol(Enum):
    http = "0"
    https = "1"