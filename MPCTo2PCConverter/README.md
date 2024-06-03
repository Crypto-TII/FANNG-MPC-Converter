# MPC to 2PC Converter
The project securely converts MPC additive shares to 2PC additive shares.  
Conversion is implemented by sending masked values of all MPC shares to one of the two parties, and sending the masks to the other party.  
By summing their inputs, the two parties get 2PC shares of the original MPC values.

## Directory Structure
* **proj** - main project directory with source files
* **proj/scripts** - python scripts for end-2-end testing of the project along with mock testing of actors.
* **unitest** - MPC data files used for testing. We have a folder structure that contains 0p testing, 3p testing and 5p testing files. We have a separate dummy material file `P0-sample_matrix_triple.txt` inside the [0p_test](../MPCTo2PCConverter/unittest/0p_test/) folder that is used in database testing files.


## Build instructions
### Dockerized environment
* Clone the Converter: 
  ```
  git clone git@github.com:Crypto-TII/FANNG-MPC-Converter.git
  ```
* In the same directory where you cloned the repo copy the docker files from inside the repo.

    ```
    cp ./FANNG-MPC-Converter/docker-files/development-environment.dockerfile ./
    cp ./FANNG-MPC-Converter/docker-files/docker-compose.yaml ./
    ```
* In the same directory as your repo, make the database folders: 
    ```
    mkdir postgresql mysql
    ```
* In the end,  your working directory would have 5 things: `FANNG-MPC-Converter`, `mysql`, `postgresql`, `development-environment.dockerfile` and `docker-compose.yaml`

* Now in the docker-compose.yaml file you need to add the path from root for the mysql, postgres and FANNG-MPC-Converter repo. It is instructed inside the docker-compose on where to add the path.
* Now build and run the docker: 
  ```
  docker-compose build --no-cache   # This is only needed once
  docker-compose up
  ```
* Now go inside the docker container:
  ```
  docker ps
  ```
* Copy the container id of the development-environment container
  ```
  docker exec -it CONTAINER_ID /bin/bash
  cd /home/MPCTo2PCConverter/proj
  ./build.sh
  ```
## Build instruction with TaasLib
* Clone the Converter as above!

* Clone the Taaslib repo (This will be available in the future!)

* Copy the combined `docker-compose-taas-converter.yaml` file
  ```
  cp ./FANNG-MPC-Converter/docker-files/docker-compose-taas-converter.yaml ./
  cp ./FANNG-MPC-Converter/docker-files/development-environment.dockerfile ./
  ```
  Add path as instructed in the above file!
* Run the docker-compose file
  ```
  docker-compose -f docker-compose-taas-converter.yaml up
  ```
* Set taas = True in the two configuration files here: [file1](db_scripts/conf.py) and [file2](proj/scripts/conf.py)
* Now inside the MPCTo2PCConverter folder run the following:
  ```
  python3 db_scripts/create_db_mysql.py
  ```
* Also in this file, please remove the material type 
* Execution of the tests will remain the same as listed below and in this [readme](proj/scripts/README.md)
## Functional Testing
* First step, copy one of the config files from the unittest section and place it in the config folder, as shown below:
```
cp unittest/3p_test/config.yaml proj/config/
```
* Four unittest executables are created in the build directory: `tests`, `tests_mysql`, `tests_postgresql` and `grpc_test`.
  ```
  cd /home/MPCTo2PCConverter/proj/build
  ./tests
  ./tests_mysql
  ./tests_postgresql
  ./grpc_test
  ```
* Running these executables returns an error code of `0` for success, and an error code of `1` for failure.
* One of the tests (the HTTPS communication) depends on SSL keys which are created during the build. If the files (3 files of the form: `test*.pem`) are missing, rerun the build command
* Note, that a MySQL server needs to run for the `tests_mysql` unitests.
* Note, that a PostgreSQL server needs to run for the `tests_postgresql` unittests.
* To run the end to end testing of the new Converter service, follow this [README](proj/scripts/README.md). [**Note**: MySQL and a PostgreSQL server needs to be running for running the above scripts.] 

#### Configurations for Testing Converter functionality

For testing we provide three modes:

1. 0p testing: Trusted dealer setup with one dealer. The configuration parameters for this mode can be adjusted in this [config.yaml](../MPCTo2PCConverter/unittest/0p_test/config.yaml) file.

2. 3p testing: 3 dealer setup and the configuration parameters for this mode can be adjusted in is this [config.yaml](../MPCTo2PCConverter/unittest/3p_test/config.yaml) file.

3. 5p testing: 5 dealer setup and the configuration parameters for this mode can be adjustted is this [config.yaml](../MPCTo2PCConverter/unittest/5p_test/config.yaml) file.

For detailed instructions, go to this [README](../MPCTo2PCConverter/proj/scripts/README.md). 

For added reference, the configuration for model owner's database files can be changed as below inside the [5p_test/config.yaml](../MPCTo2PCConverter/unittest/5p_test/config.yaml) file if your testing mode is `5p`. 
  ```
  mo_mysql_host: "localhost"
  mo_mysql_port: 3306
  mo_mysql_user: "root"
  mo_mysql_password: ""
  mo_mysql_database: "model_owner"
  mo_postgres_host: "localhost"
  mo_postgres_port: 5432
  mo_postgres_user: "postgres"
  mo_postgres_password: ""
  mo_postgres_database: "model_owner"
  ```
You have the added option to change database configurations for all actors: `model_owner`, `super_dealer`,  `client`,   `dealer_0 to dealer_9` along with their rest or grpc server configurations and other parameters in the same file.

## Individually starting the actors
To individually start our actors, we need to follow a specific order so that the functionalities can work in the way they are intended. 

1. Firstly, we create the keys and certificates that will be used by our actors to launch their servers!
```
cd /home/MPCTo2PCConverter/proj/
python3 scripts/create_keys.py
```
2. Now we build the project:
```
mkdir build
./build.sh
```
2. Next, the user has to move to the build directory inside the `proj` folder:
```
cd build
```
3. Start Super Dealer
```
./super_dealer --id 0
```
4. Start Dealer.
```
./dealer --id 1
```
5. Start Model Owner
```
./model_owner --id 0
```
6. Finally, to start the Client!
```
./client --id 0
```
## Client as an API
Client has two functions that can be separately invoked as long as the three actors `model_owner`, `super_dealer` and `dealer` are active.

This endpoint initializes the client with the provided parameters.
```
    Client client(Parameters); # Initialize Client!
    Description: Initializes the client with the provided configuration parameters.
    Parameters:
        client_db_host (string): The host of the client's database.
        client_db_user (string): The username for accessing the client's database.
        client_db_password (string): The password for accessing the client's database.
        client_database (string): The name of the client's database.
        client_db_port (int): The port of the client's database.
        https (bool): Indicates whether HTTPS is used.
        db_system (int): The type of database system MySQL: 0 & PostgreSQL: 1.
        mo_host (string): The host of the model owner.
        mo_port (int): The port of the model owner.
        mo_cert (string): The certificate of the model owner.
        dealer_rest_hosts (vector of string): The hosts of dealer REST services.
        dealer_rest_certs (vector of string): The certificates of dealer REST services.
        dealer_rest_ports (vector of int): The ports of dealer REST services.
```

**Function 1**: *get_batch_id*

This endpoint retrieves the batch ID for a given model ID.
```
    Description: Retrieves the batch ID for a given model ID.
    Parameters:
        model_id (int): The ID of the model for which the batch ID is requested.
    Response:
        batch_id (string): The batch ID associated with the provided model ID.
        total_shares_per_batch (int): Total number of shares for the batch.
```
**Function 2**: *get_shares_seed_and_mac_mask*

This endpoint retrieves the shares seed and MAC mask for a given batch ID and dealer indices.
```
    Description: Retrieves the shares seed and MAC mask for a given batch ID and dealer indices.
    Parameters:
        batch_id (string): The batch ID for which the shares seed and MAC key mask are requested.
        dealer_indices (vector of int): The indices of dealers from which to retrieve the shares seed and MAC mask.
    Response:
        shares_seed (vector of string): The shares seed associated with the provided batch ID and dealer indices.
        mac_key_mask (vector of string): The MAC key mask associated with the provided batch ID and dealer indices.
```

## Adding more actors
To add more actors, the user needs to add configurations for the specific actors be it `dealers`, `model_owners`, `clients` or `super_dealers` in the main [config.yaml](../MPCTo2PCConverter/proj/config/config.yaml) file. For instance if the user wants to work with more dealers, they just need to change the `num_dealers` parameter by the additional number of dealers being added and inside the dealers configuration add the configuration for the new dealer with its id information.
```
dealers:
  - id: 10 # We have 10 dealers already starting from 0 to 9.. so this is how the user would add an extra dealer's configurations
    host: # Add information
    port: ......
    mysql_host: ......
    mysql_port: ......
    mysql_user: ......
    mysql_db: ......
    mysql_pwd: ......
    postgres_host: ......
    postgres_port: ......
    postgres_user: ......
    postgres_db: ......
    postgres_pwd: ......
    rest_host: ......
    rest_port: ......
    rest_cert: ......
    rest_privkey: ......
    grpc_cert: ......
    grpc_privkey: ......
    https: ......
    db_engine: ......
    model_id: ......
    num_dealers: ......
    max_share_size: ......
    dealer_ids_str: ......
```
## DB Testing
For database testing in MySQL and PostgreSQL in dockerized environment, refer to this [README](db_scripts/README.md).
## Generating API Swagger docs
The proj/scripts folder contains a file `server.py` which implements the skeleton of the API (returning dummy results) as a FastAPI http server.
See [README](proj/scripts/README.md) in the script folder for details.
