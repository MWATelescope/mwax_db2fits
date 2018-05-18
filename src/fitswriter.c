/*
 * fitswriter.c
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fitsio.h"
#include "fitswriter.h"
#include "log.h"

// Create a blank new fits file called 'filename'
// Populate it with data from the metafits file (fptr_metafits needs to point to an open valid fits file)
// Fits pointer is passed to the caller
int create_fits(fitsfile **fptr, const char *out_filename, metafits_info *mptr)
{
  int status = 0;

  // Check that the metafits data is there!
  assert(mptr != NULL);
    
  printf("Creating new fits file %s...\n", out_filename);

  // Create a new blank fits file
  if (fits_create_file(fptr, out_filename, &status))
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }
  
  //
  // Add the core keywords
  //
  // SIMPLE
  int simple = TRUE;
  if ( fits_write_key(*fptr, TLOGICAL, "SIMPLE", &simple, "", &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }

  // BITPIX
  long bitpix = 8;
  if ( fits_write_key(*fptr, TLONG, "BITPIX", &bitpix, "", &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }

  // NAXIS
  long naxis = 0;
  if ( fits_write_key(*fptr, TLONG, "NAXIS", &naxis, "", &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }
   
  // INTTIME
  if ( fits_write_key(*fptr, TFLOAT, "INTTIME", &mptr->inttime, NULL, &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }

  // PROJID
  if ( fits_write_key(*fptr, TSTRING, "PROJID", mptr->project, NULL, &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }

  // OBSID
  if ( fits_write_key(*fptr, TLONG, "OBSID", &mptr->obsid, NULL, &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }

  return(EXIT_SUCCESS);
}

int close_fits(fitsfile *fptr)
{
  int status = 0;

  if (fptr != NULL)
  {
    if (fits_close_file(fptr, &status))
    {
      fits_report_error(stdout, status);
      return status;
    }
  }
  else
  {
    printf("Fits file is already closed.");
  }

  return(EXIT_SUCCESS);
}

int open_fits(fitsfile **fptr, const char* filename)
{
  int status = 0;

  if (fits_open_file(fptr, filename, READONLY, &status))
  {
    fits_report_error(stdout, status);
    return(status);
  }

  return EXIT_SUCCESS;
}

int read_metafits(fitsfile *fptr_metafits, metafits_info *mptr)
{  
  int status = 0;
  
  // INTTIME
  float inttime = 0;  
  log_debug("Reading INITTIME from metafits");  
  if ( fits_read_key(fptr_metafits, TFLOAT, "INTTIME", &inttime, NULL, &status) )
  {    
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }
  mptr->inttime = inttime;

  // PROJECT
  char project[FLEN_VALUE];  
  log_debug("Reading PROJECT from metafits");
  if ( fits_read_key(fptr_metafits, TSTRING, "PROJECT", project, NULL, &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }
  mptr->project = strdup(project);

  // GPSTIME / OBSID
  long obsid = 0;  
  log_debug("Reading GPSTIME from metafits");
  if ( fits_read_key(fptr_metafits, TLONG, "GPSTIME", &obsid, NULL, &status) )
  {
    fits_report_error(stdout, status);
    return EXIT_FAILURE;
  }
  mptr->obsid = obsid;

  return (EXIT_SUCCESS);
}