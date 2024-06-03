# Information regarding db_scripts folder

This folder contains the database creation scripts for Client, Dealer and Model Owner in MySQL as well as PostgreSQL. The schema scripts are located in the schema_files folder. The configurations for all of the testing can be found in db_scripts/conf.py. 

### Running the database tests in a dockerized environment
To facilitate running tests within a dockerized environment, please adhere to the following instructions:

1. Begin by setting up the environment as detailed in the provided [README](../../MPCTo2PCConverter/README.md) file.

2. Once you are inside the dockerized container, run the below command:

    ```
    cd /home/MPCTo2PCConverter 
    ./db_scripts/db_tests.sh
    ```

The above command runs the below files: 

* **mysql_tests.py** which tests the time take for the creation of dealer's db and filling it with shares in your local mysql server.

* **postgresql_tests.py** which tests the time taken for the creation of dealer's db and filling it with shares in your local postgresql server.

* **super_dealer_tests.py** which tests the time taken for the creation of Super Dealer's db in MySQL and PostgreSQL.

### Note
While filling the database tables, make sure to fill the dependent tables like types_per_batch, types_per_model only AFTER the tables like batch and model are filled as the foreign key constraints might fail and you will not be able to add data. For the user's reference this order is followed exactly in mysql_tests.py and postgresql_tests.py

### Additional Functionalities

1. If a user wants to just create the db schema using the provided code, just use the run_sql function from utils.py
and provide the path for the schema script file, cursor and connection object from mysql.connector.

    MySQL:  
    ```
    from conf import run_sql
    import mysql.connector 

    db_connection = mysql.connector.connect(
        host="{your_host}",
        user="{your_user}",
        password="{your_password}"
    )

    cursor = db_connection.cursor()
    script_path = /path/to/script.sql

    run_sql(script_path, cursor, db_connection)
    ```
     PostgreSQL:  
    ```
    from conf import run_sql
    import mysql.connector 

    db_connection = psycopg2.connect(
    host={your_host},
    database={your database},
    user={your_user},
    password={your_password}
    )
    cursor = db_connection.cursor()
    script_path = /path/to/script.sql

    run_sql(script_path, cursor, db_connection)
    ```

2. **utils.py** contains two additional functions create_db and insert_shares that creates and inserts shares for Model Owner, Dealer, and Client and the comments in the function best describe the arguments to be passed.

3. The usage of the above mentioned functions are demonstrated in **mysql_tests.py** and **postgresql_tests.py**

4. If you just want to create the databases without filling them with dummy data in mysql and postgres you can run the following:

    ```
    ./db_scripts/db_creation.sh
    ```
5. Once the user is done executing the tests, it is good practice to drop the databases which can be done by executing the below command:
    ```
    python3 db_scripts/drop_db_all.py
    ```
6. A test file for how the Super Dealer's DB will be filled is also provided and to run the tests for MySQL and PostgreSQL
time comparisons you can run the below command:
    ```
    python3 db_scripts/super_dealer_tests.py
    ```
