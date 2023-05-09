# Test 03: Normal observation which is split into 2 batches (000 and 001)

## Instructions

```bash
mwax_db2fits$ cd tests
mwax_db2fits/tests$ build_tests.sh
mwax_db2fits/tests$ cd test03
mwax_db2fits/tests/test03$ ./run_test03.sh
```

## Objectives

* Test that a normal observation (24 sec) is split correctly into 16 seconds and 8 seconds.

## Input data

* Three PSRDADA headers for the 3 subobservations
* Three generated data files for the 3 subobservations
* 4 timesteps (2 per subobs)
* 2 tiles (3 baselines)
* 1 coarse channel (148, correlator channel 8)
* 2 fine channels per coarse
* Correlator mode: 640kHz, 4 sec

## Expected Outputs

* A first fits file, ending in _000.fits which has:
  * Primary HDU correctly populated
  * ImageHD (timestep 1, visibilities) 16x3  
  * ImageHD (timestep 1, weights) 4x3
  * ImageHD (timestep 2, visibilities) 16x3
  * ImageHD (timestep 2, weights) 4x3
  * ImageHD (timestep 3, visibilities) 16x3
  * ImageHD (timestep 3, weights) 4x3
  * ImageHD (timestep 4, visibilities) 16x3
  * ImageHD (timestep 4, weights) 4x3
* Second fits file, ending in _001.fits which has:
  * Primary HDU correctly populated
  * ImageHD (timestep 5, visibilities) 16x3  
  * ImageHD (timestep 5, weights) 4x3
  * ImageHD (timestep 6, visibilities) 16x3
  * ImageHD (timestep 6, weights) 4x3
  