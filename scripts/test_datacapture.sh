#
# test_datacapture
#
# Run this to test mwax_db2fits
#
echo Instantiating output ring buffer...
dada_db -b 338297856 -k 2345 -n 40 

echo Loading test dada file into ringbuffer
dada_diskdb -k 2345 -f new128_4kHz_1s_250Hzres.dada 

# executing data capture
echo Starting data capture
../bin/mwax_db2fits -k 2345 --destination-path=. --metafits-path=../1096952256.metafits --health-ip=127.0.0.1 --health-port=7123 --stats-path=../

# clean up
echo Cleaning up ringbuffer
dada_db -d -k 2345
