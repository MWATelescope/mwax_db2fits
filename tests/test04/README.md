# Test 04: Two observations, checking if health info includes weights

## Instructions

See [README.MD](../README.MD)

## Objectives

* Test that the correct weights are extracted for health packets

## Input data

* Two PSRDADA headers for the 2 subobservations
* Two generated data files for the 2 subobservations
* 4 timesteps (2 per subobs)
* 2 tiles (3 baselines)
* 1 coarse channel (148, correlator channel 8)
* 2 fine channels per coarse
* Correlator mode: 640kHz, 4 sec

## Expected Outputs

* A single fits file, which has:
  * Primary HDU correctly populated
  * ImageHD (timestep 1, visibilities) 16x3  
  * ImageHD (timestep 1, weights) 4x3
  * ImageHD (timestep 2, visibilities) 16x3
  * ImageHD (timestep 2, weights) 4x3
  * ImageHD (timestep 3, visibilities) 16x3
  * ImageHD (timestep 3, weights) 4x3
  * ImageHD (timestep 4, visibilities) 16x3
  * ImageHD (timestep 4, weights) 4x3

* mwax_db2fits.log
  * 4 lines for the fits file (one per weights HDU) showing the weights values
  