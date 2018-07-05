/**
 * @file fitswriter.c
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the code that handles writing fits files
 *
 */
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dada.h"
#include "fitsio.h"
#include "fitswriter.h"
#include "utils.h"
#include "fitswriter.h"
#include "multilog.h"

extern dada_db_t g_ctx;

// Create a blank new fits file called 'filename'
// Populate it with data from the metafits file (fptr_metafits needs to point to an open valid fits file)
// Fits pointer is passed to the caller
int create_fits(fitsfile **fptr, const char *filename)
{
  int status = 0;
      
  multilog(g_ctx.log, LOG_INFO, "Creating new fits file %s...\n", filename);

  // So CFITSIO overwrites the file, we should prefix the filename with !
  int len = strlen(filename);
  char* cfitsio_filename = malloc(len*sizeof(char));
  sprintf(cfitsio_filename, "!%s", filename);

  // Create a new blank fits file
  if (fits_create_file(fptr, cfitsio_filename, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error creating fits file: %s. Error: %d -- %s\n", filename, status, error_text);
    return EXIT_FAILURE;
  }
  
  // free the malloc'ed filename
  free(cfitsio_filename);
  
  //
  // Add the core keywords
  //
  // SIMPLE
  int simple = TRUE;  
  char key_simple[FLEN_KEYWORD] = "SIMPLE";

  if ( fits_write_key(*fptr, TLOGICAL, key_simple, &simple, "", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_simple, filename, status, error_text);
    return EXIT_FAILURE;
  }

  // BITPIX
  long bitpix = 8;
  char key_bitpix[FLEN_KEYWORD] = "BITPIX";

  if ( fits_write_key(*fptr, TLONG, key_bitpix, &bitpix, "", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_bitpix, filename, status, error_text);
    return EXIT_FAILURE;
  }

  // NAXIS
  long naxis = 0;
  char key_naxis[FLEN_KEYWORD] = "NAXIS";

  if ( fits_write_key(*fptr, TLONG, key_naxis, &naxis, "", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_naxis, filename, status, error_text);
    return EXIT_FAILURE;
  }
   
  // INTTIME
  char key_inttime[FLEN_KEYWORD] = "INTTIME";

  if ( fits_write_key(*fptr, TFLOAT, key_inttime, &(g_ctx.obs_int_time), NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_inttime, filename, status, error_text);
    return EXIT_FAILURE;
  }

  // PROJID
  char key_projid[FLEN_KEYWORD] = "PROJID";

  if ( fits_write_key(*fptr, TSTRING, key_projid, g_ctx.obs_proj_id, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_projid, filename, status, error_text);
    return EXIT_FAILURE;
  }

  // OBSID
  char key_obsid[FLEN_KEYWORD] = "OBSID";

  if ( fits_write_key(*fptr, TLONG, key_obsid, &(g_ctx.obs_id), NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key: %s to file %s. Error: %d -- %s\n", key_obsid, filename, status, error_text);
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
      char error_text[30]="";
      fits_get_errstatus(status, error_text);
      multilog(g_ctx.log, LOG_ERR, "Error closing fits file. Error: %d -- %s\n", status, error_text);
      return EXIT_FAILURE;
    }
  }
  else
  {
    multilog(g_ctx.log, LOG_WARNING, "Fits file is already closed.\n");
  }

  return(EXIT_SUCCESS);
}

int open_fits(fitsfile **fptr, const char *filename)
{
  int status = 0;

  if (fits_open_file(fptr, filename, READONLY, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error openning fits file %s. Error: %d -- %s\n", filename, status, error_text);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int create_fits_imghdu(fitsfile *fptr, int baselines, int fine_channels, int polarisations, void *buffer, uint64_t bytes)
{
  // Each imagehdu will be [baseline][freq][pols]   
  int status = 0;
  int bitpix = -32;
  long naxis = 2; 
    
  //long naxes[2] = { baselines, fine_channels * polarisations };
  long naxes[2] = { 32, 8 };  // 256 elements
  //float array[8][32];

  fits_create_img(fptr, bitpix, naxis, naxes, &status);

  if (status)
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error creating HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }  
   
  /* Write the array */
  long nelements = bytes / (abs(bitpix) / 8);

  // TINT=32bit int
  fits_write_img(fptr, TFLOAT, 1, nelements, buffer, &status);
  
  if (status)
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing data into HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }
    
  return EXIT_SUCCESS;
}

/*int read_metafits(fitsfile *fptr_metafits, metafits_info *mptr)
{  
  int status = 0;
  
  // INTTIME
  float inttime = 0;  
  char key_inittime[FLEN_KEYWORD] = "INTTIME";

  multilog(g_ctx.log, LOG_INFO, "Reading %s from metafits\n", key_inittime);  
  if ( fits_read_key(fptr_metafits, TFLOAT, key_inittime, &inttime, NULL, &status) )
  {    
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_inittime, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->inttime = inttime;

  // PROJECT
  char project[FLEN_VALUE];  
  char key_project[FLEN_KEYWORD] = "PROJECT";

  multilog(g_ctx.log, LOG_INFO, "Reading PROJECT from metafits\n");
  if ( fits_read_key(fptr_metafits, TSTRING, key_project, project, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_project, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->project = strdup(project);

  // GPSTIME / OBSID
  long obsid = 0;  
  char key_gpstime[FLEN_KEYWORD] = "GPSTIME";

  multilog(g_ctx.log, LOG_INFO, "Reading GPSTIME from metafits\n");
  if ( fits_read_key(fptr_metafits, TLONG, key_gpstime, &obsid, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_gpstime, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->obsid = obsid;

  return (EXIT_SUCCESS);
}*/