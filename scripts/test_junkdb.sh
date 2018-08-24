#
# test_nicdb
#
# Run this to simulate data going to mwa_xc_data_capture
#

# create ring buffer
# header elements - default 4096 bytes
# data elements - 
# 10440 x 256+1 x 4
# [baseline][freq][pols]
#
# complex(r,i)  = 2x4 bytes  == 8 bytes
# polarisations = x2^2 == x4 == 32 bytes
# baselines     = x(128x(128+1))/2 = 8256 x 32 == 264,192 bytes 
# for 5kHz      = x(1280/5)  == x256 + 1 == 67,897,344 bytes 
# for 40kHz     = x(1280/40) == x32 + 1  ==  8,718,336 bytes 
# So the first 256 pol*baselines are visibilities, the final 1 is weights
#
# number of elements = 40  (40 timesteps of 200ms each)
#              
# shared memory key - 0x128
# lock shared memory in RAM
# page all blocks into RAM
# one reader for now
# persistent mode
# -c 0 <- use NUMA node
#echo populating with junk 16 seconds worth of 5 kHz 0.4 sec data
#dada_junkdb -b $[67897344*20] -g -m 0 -s 10000 -v -r 2048 -k 0x5678 test_obs1_1_header
#dada_junkdb -b $[67897344*20] -g -m 0 -s 10000 -v -r 2048 -k 0x5678 test_obs1_2_header
#echo Observation 1 sent

echo populating with junk 8 seconds worth of 40 kHz 2 sec data
dada_junkdb -b $[67897344*4] -g -m 0 -s 10000 -v -r 2048 -k 0x5678 test_obs2_1_header
echo Observation 2 sent
