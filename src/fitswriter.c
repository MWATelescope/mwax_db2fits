/*
 * fitswriter.c
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#include <stdio.h>
#include "fitsio.h"
#include "fitswriter.h"

// Create a blank new fits file called 'filename'
// Fits pointer is passed to the caller
int create_fits(fitsfile **fptr, const char *filename)
{
  int status = 0;

  printf("Creating new fits file %s...\n", filename);

  fits_create_file(fptr, filename, &status);

  if (status)
    fits_report_error(stderr, status);

  return(status);
}

int close_fits(fitsfile *fptr)
{
  int status = 0;

  if (fptr != NULL)
  {
    printf("Closing fits file.\n");

    fits_close_file(fptr, &status);

    if (status)
      fits_report_error(stderr, status);
  }

  return(status);
}

int open_fits(fitsfile **fptr, const char* filename)
{
  int status = 0;

  fits_open_file(fptr, filename, READONLY, &status);

  if (status)
    fits_report_error(stderr, status);
  return(status);
}
