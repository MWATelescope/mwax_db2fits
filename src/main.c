/*
 * main.c
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "main.h"
#include "args.h"
#include "fitsio.h"
#include "fitswriter.h"
#include "log.h"
#include "utils.h"

int LOG_LEVEL=XC_LOG_DEBUG;
globalArgs_t globalArgs;

int main(int argc, char* argv[])
{	  
	log_info("Starting up mwa_xc_datacapture...");
  
  if (process_args(argc, argv))
  {
    exit(EXIT_FAILURE);
  }
  
  // Do all the things
  fitsfile *fptr;
  fitsfile *fptr_metafits;
  metafits_info mptr;

  int status = 0;
    
  // Open metafits file
  if (open_fits(&fptr_metafits, globalArgs.metafits_path)) {
    log_error("Error openning metafits file.");
    exit(EXIT_FAILURE);
  }

  // Read the metafits into a structure
  if (read_metafits(fptr_metafits, &mptr)) {
    log_error("Error reading metafits file.");
    exit(EXIT_FAILURE);
  }

  // Create new fits file, based on metafits
  // Make a new filename- oooooooooo_YYYYMMDDhhmmss_gpuboxGG_FF.fits
  char filename[44] = "";
  char timestring[15];
  get_time_string_for_fits(timestring);
  int channel = 1;
  int file_number = 0;
  
  sprintf(filename, "%ld_%s_gpubox%02d_%02d.fits", mptr.obsid, timestring, channel, file_number);

  if (create_fits(&fptr, filename, &mptr)) {
    log_error("Error creating new fits file.");
    exit(EXIT_FAILURE);
  }

  // Close the metafits file
  if (close_fits(fptr_metafits)) {
    log_error("Error closing metafits file.");
    exit(EXIT_FAILURE);
  }

  // Close the fits file we created
  if (close_fits(fptr)) {
    log_error("Error closing new fits file.");
    exit(EXIT_FAILURE);
  }

  // Terminate cleanly
  log_info("Exiting...");
  exit(EXIT_SUCCESS);
}
