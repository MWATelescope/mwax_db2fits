#
# test_reset
#
# Run this to remove and recreate ring buffers
#

# create ring buffer
# header elements - default 4096 bytes
# data elements - 
# 10440 x 256+1 x 4
# [baseline][freq][pols]
#
# complex(r,i)  = 2x4 bytes  == 8 bytes
# polarisations = x2^2 == x4 == 32 bytes
# baselines     = x10,440    == 334,080 bytes 
# for 5kHz      = x(1280/5)  == x256 + 1 == 85,858,560 bytes 
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
echo "removing old test files"
rm /tmp/*.fits

echo "...instantiating ring buffers..."
dada_db -d -k 0x5678
dada_db -b 85858560 -n 40 -k 0x5678

echo ring buffers created