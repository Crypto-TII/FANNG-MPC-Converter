# Testing Communication via mocking!
Testing of Model Owner, Super Dealer and Client service is explained via mocking these services individually and with an end to end test involving all three services. 
### Testing mode
We offer three testing modes `0p_test` (trusted dealer setup), `3p test` which is the 3 party test and `5p test` which is the 5 party test. In this context, party means dealers. To set these, you need to go inside the [conf.py](conf.py) file and change the test_mode variable to either `0p`, `3p` or `5p`. The default mode is set to `5p` testing
```
test_mode = "5p" # Other option here is "5p"
```
### Individual mocking of services
Be inside the `proj` directory of the repository to run the below tests.

#### Super Dealer Test (MO (mock) -> SD (Real))
We launch our real super dealer service and make a batch request to the service that acts as our mock model owner. 
```
python3 scripts/sd_test.py
```
#### Client Test (Client (Real) -> MO (mock))
We launch our real client that makes request to a mock model owner service.
```
python3 scripts/client_test.py
```
#### gRPC Shares transfer Test (MO(Real) -> Dealers (Real))
Model owners makes requests for shares to our dynamically chosen dealers.
```
python3 scripts/mo_dealer_test.py
```
#### End to End Testing (Client (Real) -> MO (Real) -> SD (Real) -> Dealers (Real))
In this comprehensive test, the Client initiates a request for a batch_id, sending the model_id to the Model Owner. The Model Owner then queries the Super Dealer for the batch_id and responds to the Client with the received batch_id and in parallel starts to query dealers for shares and to prepare seeds for the clients.

```
./scripts/end2end.sh 1 scripts/log.txt  [Usage: ./scripts/end2end.sh <num_iterations> <path_to_log_file>]
```
This is a bash script that kills all processes before starting any new configuration out of the four configurations available (http, mysql), (http, postgres), (https, mysql) and (https, postgres). The integer `1` corresponds to the number of iterations for the end2end_testing that the user wants to run and `scripts/log.txt` is the file path where the log will be saved.

Once the tests are successfully executed, the log.txt will have an output like the following
```
------------------------------------------------------------------------------
===SUCCESSFULLY FINISHED ALL 4 ITERATIONS=======================
------------------------------------------------------------------------------
```
When the end2end test fails you will have an assertion failure of one of the three tests which are `2pc_share_test`, `2pc_mac_key_test` and `shares_transfer` test.

#### Generating testing materials
The above tests implicitly call the `generate_materials.py` file which generates the testing materials or the dummy shares that are being used to fill the tables of the actors. To execute this file separately or to create dummy shares run the below line:
```
python3 scripts/generate_materials.py
```
#### Filling databases
The above tests implicitly use functionalities from `fill_db.py` file which fills all of the tables with the initial state of values expected by the Converter. To run the file separately use the following command:
```
python3 scripts/fill_db.py 0 # {0: MySQL, 1: PostgreSQL}
```

#### Filling databases with taaslib dealers
If the user wants to use taaslib dealers to generate the cryptographic materials then the following changes need to be made in the [conf.py](conf.py) file :
```
test_mode = "3p"
taas = True
```
This requires the dealer tables to be created by taaslib(follow the instructions on the [taaslib repo](https://bitbucket.org/tiicrypto/tii-taaslib/src/master/)) and then does the conversion from 3 Party to 2 Party for the shares.
This mode will only create the tables using the [fill_db.py](fill_db.py) for Model Owner, Super Dealer and Client.

#### Configurations for testing
The testing scripts use configurations taken from the [conf.py](conf.py) file. In this file, users can customize various parameters such as the model_id, testing database table values, secret, P value and number of shares(num_rows) and others used in testing.

It's important to note that the [conf.py](conf.py) file primarily derives parameters like the actor's host, port, certs and keys information along with the database connection parameters from the `config.yaml` file located within the testing directory of the testing mode you are using, so it could be either [5p_test/config.yaml](../../unittest/5p_test/config.yaml) or [3p_test/config.yaml](../../unittest/3p_test/config.yaml). Therefore, any necessary adjustments should be made in the `config.yaml` file of the desired testing mode to ensure that the changes are reflected in the end-to-end testing configurations.