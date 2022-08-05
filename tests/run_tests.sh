#!/usr/bin/env bash
# exit when any command fails
set -e

echo Checking for a working Python3 environment
which python3
python3 --version

echo About to try and installl packages, press Ctrl-C to cancel
pip3 install --upgrade pip
pip3 install -r requirements.txt

echo Building test01...
gcc test01/make_test01_data.c common.c -o test01/make_test01_data

echo Executing test01...
cd test01
./run_test01.sh
cd ..

echo Building test02...
gcc test02/make_test02_data.c common.c -o test02/make_test02_data

echo Executing test02...
cd test02
./run_test02.sh
cd ..

echo Building test03...
gcc test03/make_test03_data.c common.c -o test03/make_test03_data

echo Executing test03...
cd test03
./run_test03.sh
cd ..

echo Analysing Test Results
pytest test01.py
pytest test02.py
pytest test03.py