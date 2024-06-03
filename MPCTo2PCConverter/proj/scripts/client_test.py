import subprocess
import sys
from time import sleep
from create_keys import create_all_keys, delete_all_keys
from conf import MAX_RETRIES, client_exe, build_dir,model_owner_port, \
    client_id, model_id, dealer_ids,dealer_rest_ports, protocol, db_engine, taas
from fill_db import create_and_fill_db
from utils import modify_config_per_test

def launch_client():
    print(f'launching client for model id = {model_id}')
    args = [client_exe,
            '--id', str(client_id),
            ]
    return subprocess.Popen(args, cwd=build_dir)

server_process = None
client_process = None
dealer_process = []

def handle_exit():
    global client_process, server_process
    print("terminating all processes")
    if client_process:
        subprocess.Popen.kill(client_process)
        client_process = None
    if server_process:
        subprocess.Popen.kill(server_process)
        server_process = None

    for i in range(len(dealer_process)):
        if dealer_process[i]:
            subprocess.Popen.kill(dealer_process[i])
            dealer_process[i] = None
    delete_all_keys(build_dir)

def main(https: str, db: str):
    global client_process, server_process
    create_all_keys(build_dir)
    create_and_fill_db(db)
    modify_config_per_test(https, db)
    try:
        print("Starting Mock Model Owner!")
        server_process = subprocess.Popen(["uvicorn", "server:model_owner", "--port", str(model_owner_port), "--reload",], cwd="scripts/", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        sleep(2)
        for id in dealer_ids:
            print("Starting Mock dealer_", id)
            dealer_process.append(subprocess.Popen(["uvicorn", "server:dealer", "--port", str(dealer_rest_ports[id]), "--reload",], cwd="scripts/", stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
        sleep(5)
        for _ in range(MAX_RETRIES):
            client_process = launch_client()
            client_process.wait()
            if client_process.returncode == 0:
                handle_exit()  # Terminate processes if client gets the desired response
                break
            sleep(0.5)
    except Exception as e:
        print(e)
        if server_process:
            subprocess.Popen.kill(server_process)
        handle_exit()
        sys.exit(1)
    finally:
        # Terminate all processes on program exit
        if server_process:
            subprocess.Popen.kill(server_process)
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