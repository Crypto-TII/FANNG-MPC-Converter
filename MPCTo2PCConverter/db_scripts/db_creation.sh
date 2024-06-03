#!/bin/bash
echo "This script creates the Model Owner, Dealer, Client and Super Dealer tables in MySQL and PostgreSQL "
python3 db_scripts/create_db_mysql.py
python3 db_scripts/create_db_postgres.py
