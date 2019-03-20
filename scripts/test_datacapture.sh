#
# test_datacapture
#
# Run this to test mwax_db2fits
#
echo loading test dada file into ringbuffer
dada_diskdb -k 0x5678 -f /home/gsleap/work/github/mwax-db2fits/mwax_250Hz_10kHz_200ms.dada

# executing data capture
../bin/mwax_db2fits -k 0x5678 --destination=. --metafits=../1096952256.metafits --health-ip=127.0.0.1 --health-port=7123
