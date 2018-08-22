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
# polarisations NPOL         == x2^2 == x4 == 32 bytes
# baselines     = (NINPUTS_XGPU * (NINPUTS_XGPU+2))/8 = (256*(256+2))/8 == 8256 * 32 bytes == 264,192 bytes 
# for 5kHz      = (BANDWIDTH_HZ / 5000) = x(1280000/5000) == (x256 + 1) x 264,192 bytes == 67,897,344 bytes 
# So the first 256 pol*baselines are visibilities, the final 1 is weights
#
# number of elements = 40  (support a max of 40 timesteps of 200ms each i.e. the biggest subobs we can take)
#              
# shared memory key - 0x128
# lock shared memory in RAM
# page all blocks into RAM
# one reader for now
# persistent mode
# -c 0 <- use NUMA node
echo "removing old test files"
rm *.fits

echo "...instantiating ring buffers..."
dada_db -d -k 0x5678
dada_db -b 67897344 -n 40 -k 0x5678 -p -l

echo ring buffers created