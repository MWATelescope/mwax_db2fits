/**
 * @file fitswriter.c
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the code that handles writing fits files
 *
 */
#include <assert.h>
#include <errno.h>
#include <fitsio.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "fitswriter.h"
#include "utils.h"
#include "fitswriter.h"
#include "multilog.h"

extern dada_db_s g_ctx;

/**
 * 
 *  @brief Creates a blank new fits file called 'filename' and populates it with data from the psrdada header.
 *  @param[out] fptr pointer to the pointer of the fitsfile created.
 *  @param[in] filename Full path and name of the fits file to create.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error.
 */
int create_fits(fitsfile **fptr, const char *filename)
{
  int status = 0;
      
  multilog(g_ctx.log, LOG_INFO, "create_fits(): Creating new fits file %s...\n", filename);

  // So CFITSIO overwrites the file, we should prefix the filename with !
  int len = strlen(filename)+2;
  char cfitsio_filename[len];
  sprintf(cfitsio_filename, "!%s", filename);

  // Create a new blank fits file
  if (fits_create_file(fptr, cfitsio_filename, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error creating fits file: %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }
    
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
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_simple, filename, status, error_text);
    return -1;
  }

  // BITPIX
  long bitpix = 8;
  char key_bitpix[FLEN_KEYWORD] = "BITPIX";

  if ( fits_write_key(*fptr, TLONG, key_bitpix, &bitpix, "", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_bitpix, filename, status, error_text);
    return -1;
  }

  // NAXIS
  long naxis = 0;
  char key_naxis[FLEN_KEYWORD] = "NAXIS";

  if ( fits_write_key(*fptr, TLONG, key_naxis, &naxis, "", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_naxis, filename, status, error_text);
    return -1;
  }
   
  // INTTIME
  char key_inttime[FLEN_KEYWORD] = "INTTIME";
  float int_time_sec = (float)g_ctx.obs_int_time_msec / 1000.0;
  if ( fits_write_key(*fptr, TFLOAT, key_inttime, &(int_time_sec), NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_inttime, filename, status, error_text);
    return -1;
  }

  // PROJID
  char key_projid[FLEN_KEYWORD] = "PROJID";

  if ( fits_write_key(*fptr, TSTRING, key_projid, g_ctx.obs_proj_id, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_projid, filename, status, error_text);
    return -1;
  }

  // OBSID
  char key_obsid[FLEN_KEYWORD] = "OBSID";

  if ( fits_write_key(*fptr, TLONG, key_obsid, &(g_ctx.obs_id), NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_obsid, filename, status, error_text);
    return -1;
  }

  return(EXIT_SUCCESS);
}

/**
 * 
 *  @brief Closes the fits file.
 *  @param[in,out] fptr Pointer to a pointer to the fitsfile structure.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int close_fits(fitsfile **fptr)
{
  multilog(g_ctx.log, LOG_DEBUG, "close_fits() called.\n");
  
  int status = 0;

  if (*fptr != NULL)
  {    
    if (fits_close_file(*fptr, &status))
    {
      char error_text[30]="";
      fits_get_errstatus(status, error_text);
      multilog(g_ctx.log, LOG_ERR, "Error closing fits file. Error: %d -- %s\n", status, error_text);
      return EXIT_FAILURE;
    }
    else
    {
      *fptr = NULL;
    }
  }
  else
  {
    multilog(g_ctx.log, LOG_WARNING, "Fits file is already closed.\n");
  }

  return(EXIT_SUCCESS);
}

/**
 * 
 *  @brief Opens a fits file for reading.
 *  @param[in,out] fptr Pointer to a pointer of the openned fits file.
 *  @param[in] filename Full path/name of the file to be openned.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
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

/**
 * 
 *  @brief Creates a new IMGHDU in an existing fits file.
 *  @param[in] fptr Pointer to the fits file we will write to.
 *  @param[in] unix_time The Unix time for this integration / timestep.
 *  @param[in] unix_millisecond_time Number of milliseconds since the last integer of unix_time.
 *  @param[in] marker The artificial counter we use to keep track of which integration/timestep this is within the observation (0 based).
 *  @param[in] baselines The number of baselines in the data (used to calculate number of elements).
 *  @param[in] fine_channels The number of fine channels (used to calculate number of elements).
 *  @param[in] polarisations The number of pols in each antenna-normally 2 (used to calculate number of elements).
 *  @param[in] int_time The integration time of the observation (milliseconds).
 *  @param[in] buffer The pointer to the data to write into the HDU.
 *  @param[in] bytes The number of bytes in the buffer to write.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int create_fits_imghdu(fitsfile *fptr, time_t unix_time, int unix_millisecond_time, int marker, int baselines, int fine_channels, 
                       int polarisations, float int_time_msec, char *buffer, uint64_t bytes)
{
  // Each imagehdu will be [baseline][freq][pols][real][imaginary]   
  int status = 0;
  int bitpix = FLOAT_IMG;  //complex(r,i)  = 2x4 bytes  == 8 bytes
  long naxis = 2; 
  uint64_t axis1 = baselines;
  uint64_t axis2 = (fine_channels+1) * polarisations * 2;   // we add 1 because of weights, we x2 as we store real and imaginary
  long naxes[2] = { axis1, axis2 };  // 10440 x 2056 elements

  multilog(g_ctx.log, LOG_DEBUG, "Creating new HDU in fits file with dimensions %lld x %lld...\n", (long long)axis1, (long long)axis2);

  // Create new IMGHDU    
  if (fits_create_img(fptr, bitpix, naxis, naxes, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error creating HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }  

  // TIME  - cotter uses this to align each channel
  char key_time[FLEN_KEYWORD] = "TIME";

  if (fits_write_key(fptr, TLONG, key_time, &unix_time, (char *) "Unix time (seconds)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing key %s into HDU. Error: %d -- %s\n", key_time, status, error_text);
    return EXIT_FAILURE;
  }
  
  // MILLITIME - provides millisecond component of TIME
  char key_millitim[FLEN_KEYWORD] = "MILLITIM";

  if (fits_update_key(fptr, TINT, key_millitim, &unix_millisecond_time, (char *)"Milliseconds since TIME",&status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing key %s into HDU. Error: %d -- %s\n", key_millitim, status, error_text);
    return EXIT_FAILURE;
  }

  // INTTIME
  char key_inttime[FLEN_KEYWORD] = "INTTIME";
  float int_time_sec = (float)int_time_msec / 1000.0;
  if (fits_write_key(fptr, TFLOAT, key_inttime, &int_time_sec, (char*)"Integration time (s)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", key_inttime, status, error_text);
    return EXIT_FAILURE;
  }

  // MARKER  
  char key_marker[FLEN_KEYWORD] = "MARKER";

  if (fits_write_key(fptr, TINT, key_marker, &marker, (char*)"Data offset marker (all channels should match)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(g_ctx.log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", key_marker, status, error_text);
    return EXIT_FAILURE;
  }

  /* Write the array */
  long nelements = bytes / (abs(bitpix) / 8);

  // Check that number of elements * bytes per element matches what we expect  
  u_int64_t expected_bytes = (axis1 * axis2 * (abs(bitpix) / 8));
  if (bytes != expected_bytes)
  {
    multilog(g_ctx.log, LOG_ERR, "HDU bytes (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", bytes, expected_bytes);
    return EXIT_FAILURE;
  }

  // Actually write the HDU data
  if (fits_write_img(fptr, TFLOAT, 1, nelements, buffer, &status))
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