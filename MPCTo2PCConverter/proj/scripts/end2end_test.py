import subprocess
from conf import MAX_RETRIES, build_dir, mo_exe, model_id, dealer_exe, \
    dealer_ids, super_dealer_exe, client_exe, P, types_per_model, taas,\
    mo_id, sd_id, client_id, actor, get_material_size
from utils import get_connection, set_search_path, modify_config_per_test
from fill_db import create_and_fill_db
from time import sleep
from create_keys import create_all_keys, delete_all_keys
import sys

def launch_model_owner():
    print("launching model owner to make batch requests to Super Dealer")
    args = [mo_exe,
            '--id', str(mo_id),
            ]
    return subprocess.Popen(args, cwd=build_dir)

def launch_dealer(id: int):
    print(f'launching dealer_{id}')
    return subprocess.Popen([dealer_exe,
                             '--id', str(id),
                            ], cwd=build_dir)


def launch_super_dealer():
    print("launching super dealer")
    return subprocess.Popen([super_dealer_exe,
                             '--id', str(sd_id),
                            ], cwd=build_dir)

def launch_client():
    print(f'launching client for model id = {model_id}')
    args = [client_exe,
            '--id', str(client_id),
            ]
    return subprocess.Popen(args, cwd=build_dir)

client_process = None
super_dealer_process = None
model_owner_process = None
dealer_process = []

def handle_exit():
    global client_process, model_owner_process, super_dealer_process, dealer_process
    print("terminating all process")
    if client_process:
        subprocess.Popen.kill(client_process)
        client_process = None
    if model_owner_process:
        subprocess.Popen.kill(model_owner_process)
        model_owner_process = None
    
    if super_dealer_process:
        subprocess.Popen.kill(super_dealer_process)
        super_dealer_process = None
    
    for i in range(len(dealer_process)):
        if dealer_process[i]:
            subprocess.Popen.kill(dealer_process[i])
            dealer_process[i] = None

    # delete certificate and key files
    delete_all_keys(build_dir)

def get_shares(actor, db, dealer_id = None):
    conn, cursor = get_connection(actor, db)
    if dealer_id is not None:
        set_search_path(cursor, actor+f"_{dealer_id}", db)
    else:
        set_search_path(cursor, actor, db)
    cursor.execute('select * from share order by share_id')
    shares = cursor.fetchall()
    conn.close()
    return shares

def test_2pc_shares(db):
    mo_shares = get_shares(actor.model_owner.value, db)
    c_shares = get_shares(actor.client.value, db)
    d_shares = []
    for dealer_id in dealer_ids:
        d_shares.append(get_shares(actor.dealer.value, db, dealer_id))
    all_materials = [item for item in types_per_model if item['model_id'] == model_id]
    total_shares = 0
    dealer_secret = 0
    for element in all_materials:
        num_shares = get_material_size(element["material_type"], element["num_materials"])
        for _ in range(num_shares):
            mo_share = int(mo_shares[total_shares][1])
            client_share = int(c_shares[total_shares][1])
            dealer_secret = 0
            for d_share in d_shares:
                dealer_secret += int(d_share[total_shares][1])
            assert (client_share + mo_share) % P == dealer_secret%P, f'2pc share test failed for share {total_shares}'
            total_shares += 1
    print('2pc share test passed')

def test_2pc_mac_key(db):
    mo_conn, mo_cursor = get_connection(actor.model_owner.value, db)
    set_search_path(mo_cursor, actor.model_owner.value, db)
    mo_cursor.execute('select * from batch order by batch_id')
    mo_batch = int(mo_cursor.fetchone()[2])
    mo_conn.close()

    c_conn, c_cursor = get_connection(actor.client.value, db)
    set_search_path(c_cursor, actor.client.value, db)
    c_cursor.execute('select * from batch order by batch_id')
    c_batch = int(c_cursor.fetchone()[2])
    c_conn.close()
    assert mo_batch + c_batch == len(dealer_ids), "Mac Key Reconstruction failed!"

    print('2pc mac key test passed')

def dealer_shares_transfer_test(db: str):
     # Check if dealers sent correct amount of shares!
    conn, cursor = get_connection(actor.model_owner.value, db)
    set_search_path(cursor, actor.model_owner.value, db)
    cursor.execute("SELECT COUNT(*) FROM share")
    total_shares_mo = cursor.fetchone()[0]
    total_materials_count = 0
    types_per_model_rows = [row for row in types_per_model if row["model_id"] == model_id]
    for row in types_per_model_rows:
        total_materials_count += get_material_size(row['material_type'], row['num_materials']) 
    conn.close()
    assert total_materials_count == total_shares_mo, "The total_materials required is not equal to total shares received!"
    print("Received correct amount of shares from Dealers!")

def wait_for_mo(db):
    shares_not_processed = True
    while(shares_not_processed):
        conn, cursor = get_connection(actor.model_owner.value, db)
        set_search_path(cursor, actor.model_owner.value, db)
        cursor.execute('select * from batch')
        shares_processed_status = int(cursor.fetchone()[3])
        if shares_processed_status == 1:
            print("Shares processing done by Model Owner!")
            shares_not_processed = False
            conn.close()
            handle_exit()  
            
def main(https: str, db: str):
    print("creating https server certificates")
    create_all_keys(build_dir)
    create_and_fill_db(db)
    modify_config_per_test(https, db)
    global model_owner_process, dealer_process, super_dealer_process
    try:
        print("Starting Super Dealer!")
        super_dealer_process = launch_super_dealer()
        sleep(2)
        for i in range(len(dealer_ids)):
            dealer_process.append(launch_dealer(dealer_ids[i]))
            sleep(1)
        model_owner_process = launch_model_owner()
        sleep(2)
        # Attempt making requests for a maximum of 5 tries
        for _ in range(MAX_RETRIES):
            try:
                client_process = launch_client()
                client_process.wait()
                wait_for_mo(db)
                if client_process.returncode == 0:
                    handle_exit()  # Terminate processes if client gets the desired response
                    break
            except Exception as e:
                print(e)
                sleep(0.5)
    finally:
        print("Starting tests!")
        dealer_shares_transfer_test(db)
        test_2pc_shares(db)
        test_2pc_mac_key(db)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python end2end.py <HTTP MODE> <DB MODE>")
        sys.exit(1)

    https = str(sys.argv[1])
    db = str(sys.argv[2])

    print(f"Received parameters: https: {https}, db: {db}")
    main(https, db)
