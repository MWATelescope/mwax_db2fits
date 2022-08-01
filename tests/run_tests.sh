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

echo Analysing Test Results
pytest -v test01.py