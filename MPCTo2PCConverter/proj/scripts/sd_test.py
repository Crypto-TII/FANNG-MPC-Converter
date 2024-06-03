import subprocess
import time
import sys
import os
import ssl
import http.client
from conf import super_dealer_exe, super_dealer_host, super_dealer_port,\
    model_id, sd_id, build_dir, model_id, db_engine, protocol, taas
from utils import modify_config_per_test
from create_keys import create_all_keys, delete_all_keys
import requests
from fill_db import create_and_fill_db

def launch_super_dealer():
    print("launching super dealer")
    return subprocess.Popen([super_dealer_exe,
                            '--id', str(sd_id),
                            ], cwd=build_dir)

super_dealer_process = None

def terminate_super_dealer_processes():
    global super_dealer_process

    if super_dealer_process:
        print("terminating super dealer process")
        super_dealer_process.terminate()
        super_dealer_process = None
    # delete certificate and key files
    delete_all_keys(build_dir)


def main(https: str, db: str):
    global super_dealer_process
    create_all_keys(build_dir)
    create_and_fill_db(db)
    modify_config_per_test(https, db)
    try:
        super_dealer_process = launch_super_dealer()
        time.sleep(2)
        if https == '1':
            context = ssl.create_default_context()
            context.load_verify_locations(cafile=os.path.join(build_dir, "super_dealer-certificate.pem"))
            conn = http.client.HTTPSConnection(super_dealer_host, super_dealer_port, context=context)
            conn.request("GET", f"/batch_id?model_id={model_id}")
            response = conn.getresponse()
        else:
            url_super_dealer = f"http://{super_dealer_host}:{super_dealer_port}/batch_id?model_id={model_id}"
            response = requests.get(url_super_dealer)
        print("Response from Super Dealer: ", response.text if https == '0' else response.read().decode('utf-8'))
        terminate_super_dealer_processes()
    except Exception as e:
        print(e)
        terminate_super_dealer_processes()
        sys.exit(1)
    finally:
        terminate_super_dealer_processes()

if __name__ == '__main__':
    for https in protocol.__members__.values():
        for db in db_engine.__members__.values():
            if taas == True and db == db_engine.postgres:
                continue
            print(f"Running Test with HTTPS: {https}, DB: {db}")
            main(https.value, db.value)
    