# create ssl keys and certificates for REST servers
from conf import super_dealer_host, model_owner_host, grpc_hosts, build_dir, dealer_ids,\
dealer_rest_hosts, num_dealers
import subprocess
from glob import glob
from os import remove
import sys

def create_keys(prefix, host, out_dir):
    subprocess.run(['openssl', 'ecparam', '-genkey', '-name', 'prime256v1',
                    '-noout', '-out', f'{prefix}-private-key.pem'],
                    cwd=out_dir, capture_output=True)
    subprocess.run(['openssl', 'ec', '-in', f'{prefix}-private-key.pem',
                    '-pubout', '-out', f'{prefix}-public-key.pem'],
                    cwd=out_dir, capture_output=True)
    subprocess.run(['openssl', 'req', '-new', '-x509', '-sha256', '-key',
                    f'{prefix}-private-key.pem', '-subj', f'/CN={host}',
                    '-out', f'{prefix}-certificate.pem'],
                    cwd=out_dir, capture_output=True)

def delete_all_keys(out_dir):
    print("deleting old keys")
    for f in glob(f'{out_dir}/model_owner*.pem'):
        remove(f)
    for f in glob(f'{out_dir}/dealer_*.pem'):
        remove(f)
    for f in glob(f'{out_dir}/grpc_dealer_*.pem'):
        remove(f)
    for f in glob(f'{out_dir}/super_dealer*.pem'):
        remove(f)

def create_all_keys(out_dir):
    delete_all_keys(out_dir)
    print('creating dealers keys and certificates')
    for id in range(num_dealers):
        create_keys(f'dealer_{id}', dealer_rest_hosts[id], out_dir)
        create_keys(f'grpc_dealer_{id}', grpc_hosts[id], out_dir)
    print("creating super dealers keys and certificates")
    print("creating keys for super dealer")
    create_keys('super_dealer', super_dealer_host, out_dir)
    print("creating model owner keys and certificates")
    create_keys('model_owner', model_owner_host, out_dir)


if __name__ == '__main__':
    create_all_keys(build_dir)
