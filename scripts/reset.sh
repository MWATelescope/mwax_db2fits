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
# baselines     = (NINPUTS_XGPU * (NINPUTS_XGPU+1))/2 = (512*(512+1))/2 == 131328 * 32 bytes == 4,202,496 bytes 
# for 1kHz      = (BANDWIDTH_HZ / 1000)  = x(1280000/1000)  == (x1280 + 1) x 4,202,496 bytes == 5,383,397,376 bytes 
# for 10kHz     = (BANDWIDTH_HZ / 10000) = x(1280000/10000) == (x128 + 1) x 4,202,496 bytes == 542,121,984 bytes
# So the first 512 pol*baselines are visibilities, the final 1 is weights
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
dada_db -b 338297856 -n 40 -k 0x5678  

echo ring buffers created
