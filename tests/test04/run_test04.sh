#!/usr/bin/env bash

echo "Test04- see README.md for more information"

echo "Removing old tmp, fits and data files"
rm -v *.tmp
rm -v *.fits
rm -v *.dat
rm -v mwax_db2fits.log

echo "Clearing ring buffers"
dada_db -k 2345 -d

echo "Creating ring buffers (4 buffers of 240 bytes)"
dada_db -k 2345 -n 4 -b 240

echo "Create subobservation 1"
./make_test04_data 1 test04_header_1.txt test04_data1.dat

echo "Create subobservation 2"
./make_test04_data 2 test04_header_2.txt test04_data2.dat

echo "Load into ring buffers"
dada_diskdb -s -k 2345 -f test04_data1.dat
dada_diskdb -s -k 2345 -f test04_data2.dat

echo "Load our quit command into ring buffer"
dada_diskdb -s -k 2345 -f ../quit_header.txt

echo "Launching mwax_db2fits"
../../bin/mwax_db2fits -k 2345 --destination-path=. -l 0 -n eth0 -i 224.0.2.2 -p 50001 |& tee mwax_db2fits.log
