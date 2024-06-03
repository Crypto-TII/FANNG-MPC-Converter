import subprocess
from conf import MAX_RETRIES, model_owner_host, model_owner_port, build_dir, \
    mo_exe, mo_id, model_id, dealer_exe, dealer_ids, types_per_model,\
    get_material_size, db_engine, actor, protocol, taas
from utils import get_connection, set_search_path, modify_config_per_test
from time import sleep
from create_keys import delete_all_keys
import requests
from fill_db import create_and_fill_db

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

model_owner_process = None
super_dealer_process = None
dealer_process = []

def handle_exit():
    global model_owner_process, super_dealer_process, dealer_process
    print("terminating all process")
    if model_owner_process:
        subprocess.Popen.kill(model_owner_process)
        model_owner_process = None
    
    if super_dealer_process:
        subprocess.Popen.kill(super_dealer_process)
        super_dealer_process = None
    
    for i in range(len(dealer_process)):
        if dealer_process[i]:
            subprocess.Popen.kill(dealer_process[i])
            super_dealer_process = None

    # delete certificate and key files
    delete_all_keys(build_dir)

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
    global model_owner_process, dealer_process, super_dealer_process
    create_and_fill_db(db)
    modify_config_per_test(https, db)
    try:
        print("Starting Mock Super Dealer!")
        super_dealer_process = subprocess.Popen(["uvicorn", "server:super_dealer", "--port", "9998", "--reload",], cwd="scripts/", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for i in range(len(dealer_ids)):
            dealer_process.append(launch_dealer(dealer_ids[i]))
            sleep(1)  
        model_owner_process = launch_model_owner()
        sleep(2)
        # Attempt making requests for a maximum of 5 tries
        for _ in range(MAX_RETRIES):
            try:
                url_model_owner = f"http://{model_owner_host}:{model_owner_port}/model_inference?model_id={model_id}"
                response = requests.get(url_model_owner)
                sleep(4) # Give time for dealer thread to prepare shares!
                if response.status_code == 200:
                    print("Response by Model Owner: ", response.json())
                    wait_for_mo(db)
                    break  # If successful, exit the retry loop
            except requests.RequestException as req_exc:
                print(f"failed: {req_exc}")
            except Exception as e:
                print(f"failed with an unexpected error: {e}")
            sleep(0.5)
        else:
            print("Max retries reached, could not complete the request.") 

        # Check if dealers sent correct amount of shares!
        total_shares_mo = 0
        cursor = None
        conn, cursor = get_connection(actor.model_owner.value, db)
        set_search_path(cursor, actor.model_owner.value, db)
        cursor.execute("SELECT COUNT(*) FROM share")
        total_shares_mo = cursor.fetchone()[0]
        total_materials = 0
        types_per_model_rows = [row for row in types_per_model if row["model_id"] == model_id]
        for row in types_per_model_rows:
            total_materials += get_material_size(row['material_type'], row['num_materials']) 
        conn.close()
        assert total_materials == total_shares_mo, "The total_materials required is not equal to total shares received!"
        print("Received correct amount of shares from Dealers!")
    finally:
        handle_exit()

if __name__ == '__main__':
    for https in protocol.__members__.values():
        for db in db_engine.__members__.values():
            if https == protocol.https: # No mocking of server side for https 
                continue
            if taas == True and db == db_engine.postgres:
                continue
            print(f"Running Test with HTTPS: {https}, DB: {db}")
            main(https.value, db.value)