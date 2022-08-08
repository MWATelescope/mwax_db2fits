#!/usr/bin/env bash

echo "Test02- see README.md for more information"

echo "Removing old tmp, fits and data files"
rm -v *.tmp
rm -v *.fits
rm -v *.dat
rm -v mwax_db2fits.log

echo "Clearing ring buffers"
dada_db -k 2345 -d

echo "Creating ring buffers (8 buffers of 240 bytes)"
dada_db -k 2345 -n 8 -b 240

echo "Create obs 1, subobservation 1"
./make_test02_data test02_obs1_header_1.txt test02_obs1_data1.dat

echo "Skipping (on purpose) obs1, subobservation 2"

echo "Create obs1, subobservation 3"
./make_test02_data test02_obs1_header_3.txt test02_obs1_data3.dat

echo "Create obs1, subobservation 4"
./make_test02_data test02_obs1_header_4.txt test02_obs1_data4.dat

echo "Create obs2, subobservation 1"
./make_test02_data test02_obs2_header_1.txt test02_obs2_data1.dat

echo "Load into ring buffers"
dada_diskdb -s -k 2345 -f test02_obs1_data1.dat
dada_diskdb -s -k 2345 -f test02_obs1_data3.dat
dada_diskdb -s -k 2345 -f test02_obs1_data4.dat
dada_diskdb -s -k 2345 -f test02_obs2_data1.dat

echo "Load our quit command into ring buffer"
dada_diskdb -s -k 2345 -f ../quit_header.txt

echo "Launching mwax_db2fits"
../../bin/mwax_db2fits -k 2345 --destination-path=. -l 0 -n eth0 -i 224.0.2.2 -p 50001 |& tee mwax_db2fits.log
