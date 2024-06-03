#!/bin/bash
echo "This test runs MySQL and PostgreSQL tests and compares them in terms of creation, insertion and retrieval!"
python3 db_scripts/postgresql_test.py
python3 db_scripts/mysql_test.py
echo "Starting Super Dealer DB test!"
python3 db_scripts/super_dealer_test.py
