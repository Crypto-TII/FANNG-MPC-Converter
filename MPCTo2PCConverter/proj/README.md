# Multi-Party Computation (MPC) to Two-Party Computation (2PC) Conversion

## Overview
The actors involved in this system are: **Super Dealer**, **Dealers**, **Model Owner**, and **Client**. The system operates around continuous generation of additive MPC _triple shares_ by a group of dealers. These shares enable MPC multiplication. The MPC shares are converted into 2PC shares for ML inference, facilitated by a REST API among the four actors—**Client**, **Model Owner**, **Super Dealer** and **Dealers** along with gRPC support for efficient and fast transfer of shares between **Model Owner** and **Dealers** —using executables with matching names. The data is stored in individual MySQL databases with additional support for PostgreSQL, for each actor.

## REST API Interactions

### **model_inference**: [ Client &rarr; Model Owner ]
#### Request Parameters (sent by client):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| model_id | String  | Name of the model              |

#### Response (from model owner):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| batch_id | String  | Unique ID (UUID) for inference |


### **batch_id**: [ Model owner &rarr; Super Dealer ]
#### Request Parameters (sent by client):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| model_id | String  | Name of the model              |

#### Response (from model owner):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| batch_id | String  | Unique ID (UUID) for inference |

### **Seed**: [ Client &rarr; Dealer (sent to each dealer) ]
#### Request Parameters (sent by client):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| batch_id | String  | Unique ID (UUID) for inference |

#### Response (from dealer):

| Argument | Type    | Comment                        |
|:---------|:--------|:-------------------------------|
| shares_seed     | String  | Decimal representation of seed |
| mac_key_mask     | String  | Mask used for mac key |


## gRPC interactions
To enhance communication efficiency between the Model Owner and Dealer, especially when handling substantial data volumes, we've chosen to transition from REST to gRPC. This decision stems from exhaustive experimentation and comprehensive comparisons tailored to the converter's specific use case. Our findings demonstrate that in scenarios demanding larger data transfers, gRPC surpasses REST, showcasing superior speed and efficiency.

### **batch** : model-owner &rarr; dealer (sent to each dealer)
##### Request parameters (sent by model owner):

|argument | type | comment |
|:--------| :---- | :-------: |
| batch_id | str | unique id (uuid) for inference |


##### Response (from dealer):

|argument | type | comment |
|:--------| :---- | :-------: |
| masked_data | array | [[mac_key_share], [share,mac_share],[share,mac_share],...] |

The dealer sends masked mac_key_share(always the first value in the message) along with the masked share and mac_share.
## Flow of messages
1. Client starts by issuing a _model\_inference_ request
2. Model Owner after being prompted by the Client, issues a _batch_ request to the Super Dealer to send batch_id for the requested model.
3. The Super Dealer checks for the presence of batch_id and if found returns it to the Model Owner if not the process is terminated as no materials are available for the batch.
4. Model Owner now sends this batch_id, if found, to the dealers dynamically chosen by the clients (options to choose from 2 to 10 dealers) and gets the share, mac_share and mac_key_share.
5. After the Model Owner signals the Dealers for shares (in parallel) it sends back the batch_id to the Client.
6. After receiving the batch_id from the Model Owner, Client issues a _seed_ request to get the seeds from the Dealers in order to start the reconstruction process for the share, mac_share and mac_key_share.

## Testing of the new converter

Refer to [Testing Communication via mocking](scripts/README.md)

## Swagger API docs

We make use of Uvicorn and FastAPI to generate the Swagger API docs for the actors that initiate a REST server. To launch the servers run the below command inside the [scripts directory](../proj/scripts):

* For Model Owner's REST server:
    ```
    uvicorn server:model_owner --port 9999 --reload # change port as per availability
    ```
* For Super Dealer's REST server:
    ```
    uvicorn server:super_dealer --port 9998 --reload # change port as per availability
    ```
* For Dealer's REST server:
    ```
    uvicorn server:dealer --port 9997 --reload # change port as per availability
    ```

Now, to view the doc go to `localhost:{port_number}/docs` (Eg: model_owner's swagger docs: `localhost:9999/docs`) and you will be able to see the swagger docs for the above servers! You will be able to see something like this [image](doc/Model_Owner_REST_server.png) on the url.

### Business Process Model
[Business Process Mapping of the new Converter](./doc/BPM_Converter.pdf)  

## Database schema

For the purpose of this project, schemas are provided in MySQL as well as in PostgreSQL. The schemas can be found [here](../db_scripts/schema_files/). The database model draw.io file can be downloaded from [here](doc/Database_Model.drawio)
