import numpy as np
from conf import dealer_ids, num_rows, secret, materials_upper_bound, materials_lower_bound, \
    test_mode, seed

def random_int_generator(rng, lower_bound, upper_bound):
    while True:
        yield rng.integers(lower_bound, upper_bound)

def generate_dealer_table(filename, num_rows):
    generator = np.random.default_rng(seed)
    random_int = random_int_generator(generator, materials_lower_bound, materials_upper_bound)
    with open(filename, 'w') as file:
        for row in range(num_rows):
            share = next(random_int)
            mac_share = next(random_int)
            file.write(f"{row+1}\t{share}\t{mac_share}\n")

def generate_last_dealer_table(filename, dealer_files, num_rows):    
    sum_share = np.full(num_rows, secret, dtype=int) 
    sum_mac_share = np.full(num_rows, secret, dtype=int) 
        
    for file in dealer_files:
        with open(file, 'r') as f:
            lines = f.readlines()
            for row in range(num_rows):
                _, share, mac_share = lines[row].split('\t')
                sum_share[row] -= int(share)
                sum_mac_share[row] -= int(mac_share)               
    
    with open(filename, 'w') as file:
        for row in range(num_rows):
            file.write(f"{row+1}\t{sum_share[row]}\t{sum_mac_share[row]}\n")

def generate_dummy_materials():
    dealer_files = []
    for dealer_id in dealer_ids[:-1]: # Not for the last dealer
        filename = f"../unittest/{test_mode}_test/dealer_{dealer_id}_shares.txt"
        generate_dealer_table(filename, num_rows)  
        dealer_files.append(filename)

    last_dealer_filename = f"../unittest/{test_mode}_test/dealer_{dealer_ids[-1]}_shares.txt"
    generate_last_dealer_table(last_dealer_filename, dealer_files, num_rows)
