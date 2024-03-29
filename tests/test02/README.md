# Test 02: Normal observation with a >8 time gap between subobservations

This has occured before when:
a) An error in mwax_subfile_Distributor skips a subobs
b) An error in mwax_u2s which causes it to not create a .sub file with the correct header (last time it was due to the 100G link going down and coming back)

## Instructions

See [README.MD](../README.MD)

## Objectives

* Test that it is handled if the 2nd subobs of a 4 subobs obs is missing, and this next processes subobs3 which is offset by >8 seconds from subobs1
* mwax_db2fits should start the complete writing out the .fits.tmp file after the first subobs
* but when it hits the next (third) subobs, should notice the >8s gap and then skip the observation
* Until the next new observation/subobs comes around.
* So:
* Subobs 1: Obsid=1, offset=0 , duration=32: Create fits.tmp file
* Subobs 2: missing
* Subobs 3: Obsid=1, offset=16, duration=32: Skip
* Subobs 4: Obsid=1, offset=24, duration=32: Skip
* Subobs 5: Obsid=2, offset=0,  duration=8 : Start and finish new obs OK

## Input data

* Four PSRDADA headers for the 4 subobservations
* Four generated data files for the 4 subobservations
* 2 timesteps per subobs
* 2 tiles (3 baselines)
* 1 coarse channel (148, correlator channel 8)
* 2 fine channels per coarse
* Correlator mode: 640kHz, 4 sec

## Expected Outputs

* No file output for first subobs/obs
* A complete fits file for the second obsid, which has:
  * Primary HDU correctly populated
  * ImageHD (timestep 1, visibilities) 16x3  
  * ImageHD (timestep 1, weights) 4x3
  * ImageHD (timestep 2, visibilities) 16x3
  * ImageHD (timestep 2, weights) 4x3
