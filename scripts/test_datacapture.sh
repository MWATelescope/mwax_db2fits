#
# test_datacapture
#
# Run this to test mwax_db2fits
#
echo Instantiating output ring buffer...
dada_db -b 338297856 -k 2345 -n 40 -l -p -c 0

echo Loading test dada file into ringbuffer
dada_diskdb -k 2345 -f /data/mwax-db2fits/test_128t_4khz_1sec.dada

# executing data capture
echo Starting data capture
../bin/mwax_db2fits -k 2345 --destination=/data/mwax-db2fits/. --metafits=../1096952256.metafits --health-ip=127.0.0.1 --health-port=7123

# clean up
echo Cleaning up ringbuffer
dada_db -d -k 2345
