#!/usr/bin/env bash
# exit when any command fails
set -e

pushd tests

echo Checking for a working Python3 environment
which python3
python3 --version

echo About to try and installl packages, press Ctrl-C to cancel
pip3 install --upgrade pip
pip3 install -r requirements.txt

for i in {01..04}
do
    echo Building test${i}...
    gcc test${i}/make_test${i}_data.c common.c -o test${i}/make_test${i}_data

    echo Executing test${i}...
    pushd test${i}
    ./run_test${i}.sh
    popd
done

echo Analysing Test Results
for i in {01..04}
do
    pytest test${i}.py
done

popd